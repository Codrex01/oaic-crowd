"""
CPR AI Vision System - Master Application (CPR-AI/main.py)
Crowd Pressure Radar — Complete Hybrid AI + IoT Vision Module

Features:
- Multi-source adaptive video acquisition (USB webcam, Mobile Phone Wi-Fi stream, or video).
- CLAHE low-light enhancement for dim exhibition halls / cheap webcam sensors.
- YOLO11n person detection + ByteTrack persistent multi-person tracking on NVIDIA CUDA.
- Geometric bidirectional entry/exit counting line with hysteresis debounce.
- Trajectory velocity, dwell-time analysis, and stationary bottleneck detection.
- Zone-based crowd congestion analysis (Density, Stop-and-Go index, GREEN/AMBER/RED state).
- Asynchronous non-blocking telemetry transmitter communicating with Express backend.

Controls:
  [e] Toggle Adaptive Low-Light Enhancement (ON/OFF)
  [w] Move Counting Line UP
  [s] Move Counting Line DOWN
  [r] Reset In/Out Counters
  [q] Graceful Shutdown
"""

import cv2
import time
import sys
import torch
from ultralytics import YOLO

from config import cfg
from vision.camera import AdaptiveCamera
from vision.movement import MovementTracker
from vision.line_counter import VirtualLineCounter
from vision.zones import ZoneManager
from analytics.congestion import CongestionEstimator
from integration.backend import TelemetryClient

class CPRVisionMaster:
    def __init__(self):
        print("=" * 65)
        print("  CPR AI VISION SYSTEM (Crowd Pressure Radar) — v2.4")
        print("=" * 65)

        # 1. Hardware device assignment
        self.device = "cuda:0" if torch.cuda.is_available() else "cpu"
        print(f"[CPR-Master] Compute Backend: {self.device}")
        if self.device == "cuda:0":
            print(f"[CPR-Master] Device Name:     {torch.cuda.get_device_name(0)}")

        # 2. Detector Model
        print(f"[CPR-Master] Loading YOLO model: {cfg.model_name}...")
        self.model = YOLO(cfg.model_name)
        self.model.to(self.device)
        print("[CPR-Master] YOLO model initialized.")

        # 3. Adaptive Camera Engine
        print(f"[CPR-Master] Initializing Video Source: {cfg.camera_source}...")
        self.camera = AdaptiveCamera(
            source=cfg.camera_source,
            req_width=cfg.frame_width,
            req_height=cfg.frame_height,
            enable_clahe=cfg.enable_clahe,
            clahe_clip=cfg.clahe_clip_limit,
            gamma=cfg.gamma_correction
        )
        self.w = self.camera.actual_width or cfg.frame_width
        self.h = self.camera.actual_height or cfg.frame_height
        print(f"[CPR-Master] Stream Active: {self.w}x{self.h} | Low-Light Filter: {'ON' if self.camera.enable_clahe else 'OFF'}")

        # 4. Line Counter Geometry
        self.line_y_ratio = cfg.line_start_ratio[1]
        self._calculate_line_points()
        self.line_counter = VirtualLineCounter(
            self.line_p1, self.line_p2, 
            debounce_seconds=cfg.line_debounce_seconds
        )

        # 5. Zone Manager & Congestion Analytics
        self.zone = ZoneManager(cfg.zone_id, cfg.zone_polygon_ratios, self.w, self.h)
        self.movement = MovementTracker(
            stationary_speed_thresh=cfg.stationary_speed_threshold,
            stationary_dwell_thresh=cfg.stationary_dwell_seconds
        )
        self.congestion_evaluator = CongestionEstimator(
            green_max=cfg.green_occupancy_max,
            amber_max=cfg.amber_max
        )

        # 6. Background Telemetry Client
        print(f"[CPR-Master] Connecting to Backend Telemetry: {cfg.backend_url}...")
        self.telemetry = TelemetryClient(
            backend_url=cfg.backend_url,
            auth_token=cfg.auth_token,
            send_rate_hz=cfg.telemetry_rate_hz
        )

        # FPS Buffer
        self.fps_buffer = []

    def _calculate_line_points(self):
        p1 = (int(self.w * cfg.line_start_ratio[0]), int(self.h * self.line_y_ratio))
        p2 = (int(self.w * cfg.line_end_ratio[0]), int(self.h * self.line_y_ratio))
        self.line_p1 = p1
        self.line_p2 = p2

    def run(self):
        print("\n[CPR-Master] Vision pipeline started successfully.")
        print("  Press 'e' to toggle low-light boost | 'w'/'s' to adjust line | 'r' to reset | 'q' to quit.\n")

        while True:
            t0 = time.perf_counter()
            ret, frame = self.camera.read_frame()

            if not ret or frame is None:
                # Camera reconnecting or waiting for frame
                time.sleep(0.05)
                continue

            current_time = time.time()

            # 1. Run YOLO + ByteTrack Inference
            results = self.model.track(
                source=frame,
                persist=True,
                tracker=cfg.tracker_config,
                classes=[cfg.target_class_id],
                conf=cfg.confidence_threshold,
                iou=cfg.iou_threshold,
                device=self.device,
                verbose=False
            )

            current_centroids = {}
            raw_boxes = []

            # 2. Parse Detections & Trajectories
            if results and len(results) > 0 and results[0].boxes is not None:
                for box in results[0].boxes:
                    x1, y1, x2, y2 = map(int, box.xyxy[0].cpu().numpy())
                    conf = float(box.conf[0].cpu().numpy())
                    track_id = int(box.id[0].cpu().numpy()) if box.id is not None else None

                    cx = (x1 + x2) // 2
                    cy = (y1 + y2) // 2
                    raw_boxes.append((x1, y1, x2, y2, conf, track_id, (cx, cy)))

                    if track_id is not None:
                        current_centroids[track_id] = (cx, cy)

            # 3. Geometric Crossing Evaluation
            for tid, (cx, cy) in current_centroids.items():
                existing_track = self.movement.get_track(tid)
                if existing_track and len(existing_track.positions) > 0:
                    prev_pt = (existing_track.positions[-1][0], existing_track.positions[-1][1])
                    self.line_counter.check_crossing(tid, prev_pt, (cx, cy), current_time)

            # Update movement positions and dwell timers
            self.movement.update_tracks(current_centroids, current_time)

            # 4. Zone Containment & Congestion Analytics
            zone_active_ids = self.zone.filter_occupancy(current_centroids)
            congestion_data = self.congestion_evaluator.evaluate(self.movement.tracks, zone_active_ids)

            # 5. Non-Blocking Telemetry Broadcast
            telemetry_payload = {
                "deviceId": cfg.device_id,
                "zoneId": cfg.zone_id,
                "timestamp": int(current_time),
                "source": "computer_vision",
                "state": congestion_data["state"],
                "score": round(congestion_data["score"] * 100.0, 1),
                "density": congestion_data["density"],
                "avgSpeed": congestion_data["avg_speed"],
                "stopGoIndex": congestion_data["stop_go_index"],
                "targetCount": congestion_data["occupancy"],
                "entryCount": self.line_counter.entry_count,
                "exitCount": self.line_counter.exit_count,
                "activeAlerts": congestion_data["alerts"],
                "firmwareVersion": cfg.firmware_version
            }
            self.telemetry.send_telemetry_async(telemetry_payload)

            # 6. Render Technical Visualization Overlay
            t1 = time.perf_counter()
            frame_time = t1 - t0
            fps = 1.0 / frame_time if frame_time > 0 else 0
            self.fps_buffer.append(fps)
            if len(self.fps_buffer) > 20:
                self.fps_buffer.pop(0)
            avg_fps = sum(self.fps_buffer) / len(self.fps_buffer)

            self._render_scene(frame, raw_boxes, zone_active_ids, congestion_data, avg_fps, frame_time * 1000, current_time)

            cv2.imshow("CPR - Crowd Pressure Radar (Master Vision Engine)", frame)

            # 7. Keyboard Events
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('e'):
                state = self.camera.toggle_enhancement()
                print(f"[CPR-Master] Adaptive Low-Light Enhancement: {'ON' if state else 'OFF'}")
            elif key == ord('r'):
                self.line_counter.entry_count = 0
                self.line_counter.exit_count = 0
                print("[CPR-Master] In/Out Counters reset.")
            elif key == ord('w'):
                self.line_y_ratio = max(0.15, self.line_y_ratio - 0.05)
                self._calculate_line_points()
                self.line_counter.update_geometry(self.line_p1, self.line_p2)
            elif key == ord('s'):
                self.line_y_ratio = min(0.85, self.line_y_ratio + 0.05)
                self._calculate_line_points()
                self.line_counter.update_geometry(self.line_p1, self.line_p2)

        self.cleanup()

    def _render_scene(self, frame, raw_boxes, zone_ids, congestion_data, fps, latency_ms, current_time):
        h, w = frame.shape[:2]

        # 1. Draw ROI Zone Boundary
        zone_color = (
            cfg.COLOR_GREEN if congestion_data["state"] == "GREEN" else
            (cfg.COLOR_AMBER if congestion_data["state"] == "AMBER" else cfg.COLOR_RED)
        )
        self.zone.draw(frame, color=zone_color, alpha=0.10)

        # 2. Draw Counting Line
        recent_event = self.line_counter.get_recent_event_status(current_time)
        line_color = (0, 255, 0) if recent_event == "ENTRY" else ((0, 165, 255) if recent_event == "EXIT" else cfg.COLOR_LINE)
        line_thick = 4 if recent_event else 2
        cv2.line(frame, self.line_p1, self.line_p2, line_color, line_thick, cv2.LINE_AA)
        cv2.putText(frame, "IN [v]", (self.line_p1[0] + 6, self.line_p1[1] + 16), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (50, 205, 50), 1, cv2.LINE_AA)
        cv2.putText(frame, "OUT [^]", (self.line_p2[0] - 55, self.line_p2[1] - 6), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 165, 255), 1, cv2.LINE_AA)

        # 3. Draw Tracked Persons, Motion Trails, and Badges
        for x1, y1, x2, y2, conf, track_id, (cx, cy) in raw_boxes:
            track = self.movement.get_track(track_id) if track_id else None

            # Bounding box color based on stationary state
            box_color = cfg.COLOR_RED if (track and track.is_stationary) else cfg.COLOR_CYAN

            # Draw Box
            cv2.rectangle(frame, (x1, y1), (x2, y2), box_color, 2)

            # Draw Motion Trail
            if track and len(track.positions) > 1:
                pts = list(track.positions)
                for i in range(1, len(pts)):
                    cv2.line(frame, (pts[i-1][0], pts[i-1][1]), (pts[i][0], pts[i][1]), (0, 255, 255), 1, cv2.LINE_AA)

            # Badge Label
            if track_id:
                dwell = f"{track.total_dwell_seconds:.0f}s" if track else "0s"
                badge = f"ID #{track_id} [{dwell}]"
            else:
                badge = f"Person ({conf*100:.0f}%)"

            (tw, th), _ = cv2.getTextSize(badge, cv2.FONT_HERSHEY_SIMPLEX, 0.40, 1)
            cv2.rectangle(frame, (x1, y1 - 18), (x1 + tw + 6, y1), box_color, -1)
            cv2.putText(frame, badge, (x1 + 3, y1 - 4), cv2.FONT_HERSHEY_SIMPLEX, 0.38, (0, 0, 0), 1, cv2.LINE_AA)

        # 4. Top Technical Header HUD
        overlay = frame.copy()
        cv2.rectangle(overlay, (0, 0), (w, 36), cfg.COLOR_DARK_SLATE, -1)
        cv2.rectangle(overlay, (0, h - 34), (w, h), cfg.COLOR_DARK_SLATE, -1)
        cv2.addWeighted(overlay, 0.78, frame, 0.22, 0, frame)

        # Header Details
        cv2.putText(frame, "CPR RADAR v2.4", (10, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.52, (0, 255, 255), 2, cv2.LINE_AA)
        
        backend_status = "NET: ONLINE" if self.telemetry.is_connected else "NET: LOCAL"
        status_color = (50, 205, 50) if self.telemetry.is_connected else (148, 163, 184)
        cv2.putText(frame, backend_status, (150, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.42, status_color, 1, cv2.LINE_AA)

        enh_label = "LOW-LIGHT: ON" if self.camera.enable_clahe else "LOW-LIGHT: OFF"
        enh_color = (0, 255, 255) if self.camera.enable_clahe else (148, 163, 184)
        cv2.putText(frame, enh_label, (260, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.42, enh_color, 1, cv2.LINE_AA)

        perf_label = f"{fps:.1f} FPS ({latency_ms:.0f}ms)"
        cv2.putText(frame, perf_label, (w - 180, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.42, (255, 255, 255), 1, cv2.LINE_AA)

        # 5. Bottom Telemetry Bar
        cv2.putText(frame, f"IN: {self.line_counter.entry_count}", (10, h - 11), cv2.FONT_HERSHEY_SIMPLEX, 0.46, (50, 205, 50), 2, cv2.LINE_AA)
        cv2.putText(frame, f"OUT: {self.line_counter.exit_count}", (85, h - 11), cv2.FONT_HERSHEY_SIMPLEX, 0.46, (0, 165, 255), 2, cv2.LINE_AA)
        cv2.putText(frame, f"ZONE OCCUPANCY: {congestion_data['occupancy']}", (170, h - 11), cv2.FONT_HERSHEY_SIMPLEX, 0.46, (255, 255, 255), 1, cv2.LINE_AA)

        # Congestion Badge
        c_state = congestion_data["state"]
        c_color = cfg.COLOR_GREEN if c_state == "GREEN" else (cfg.COLOR_AMBER if c_state == "AMBER" else cfg.COLOR_RED)
        cv2.putText(frame, f"STATE: {c_state}", (w - 120, h - 11), cv2.FONT_HERSHEY_SIMPLEX, 0.48, c_color, 2, cv2.LINE_AA)

    def cleanup(self):
        self.camera.release()
        self.telemetry.stop()
        cv2.destroyAllWindows()
        print("[CPR-Master] All resources released.")

if __name__ == "__main__":
    master = CPRVisionMaster()
    master.run()
