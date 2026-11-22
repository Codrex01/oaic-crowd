"""
CPR AI Vision System - Stage 2: Virtual Counting Line & Motion Trajectory Tracking
Integrates:
- YOLO11n person detection + ByteTrack multi-person tracking on CUDA.
- Virtual boundary line with vector crossing detection (Entry vs Exit).
- Movement velocity, trajectory trails, dwell time, and stationary detection.
- Polished technical HUD with live telemetry badges.

Interactive Controls:
- 'q': Quit application
- 'r': Reset In/Out counters to zero
- 'w' / 's': Move the counting line Up / Down dynamically
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
from monitoring.overlay import HUDOverlay

class CPRStage2Runner:
    def __init__(self):
        # 1. Hardware device
        self.device = "cuda:0" if torch.cuda.is_available() else "cpu"
        print(f"[CPR-Stage2] Compute Backend: {self.device}")

        # 2. Detector Model
        print(f"[CPR-Stage2] Loading YOLO model: {cfg.model_name}...")
        self.model = YOLO(cfg.model_name)
        self.model.to(self.device)

        # 3. Camera Acquisition (Adaptive with CLAHE low-light enhancement)
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
        print(f"[CPR-Stage2] Camera Active: {self.w}x{self.h} | Low-Light Filter: {'ON' if self.camera.enable_clahe else 'OFF'}")

        # 4. Line Geometry
        self.line_y_ratio = cfg.line_start_ratio[1]
        self._calculate_line_points()

        # 5. Movement Engine & Virtual Counter
        self.movement = MovementTracker(
            stationary_speed_thresh=cfg.stationary_speed_threshold,
            stationary_dwell_thresh=cfg.stationary_dwell_seconds
        )
        self.counter = VirtualLineCounter(
            self.line_p1, self.line_p2, 
            debounce_seconds=cfg.line_debounce_seconds
        )
        self.hud = HUDOverlay(cfg)

        # FPS calculation
        self.fps_buffer = []

    def _calculate_line_points(self):
        """Calculates pixel coordinates for the horizontal virtual line."""
        p1_x = int(self.w * cfg.line_start_ratio[0])
        p1_y = int(self.h * self.line_y_ratio)
        p2_x = int(self.w * cfg.line_end_ratio[0])
        p2_y = int(self.h * self.line_y_ratio)
        self.line_p1 = (p1_x, p1_y)
        self.line_p2 = (p2_x, p2_y)

    def run(self):
        print("\n" + "=" * 65)
        print("  CPR AI Crowd Pressure Radar — Stage 2: Entry/Exit Line Counter")
        print("  Controls:")
        print("    [w] Move Counting Line UP      [s] Move Counting Line DOWN")
        print("    [r] Reset Counters to 0        [q] Quit System")
        print("=" * 65 + "\n")

        while True:
            t0 = time.perf_counter()
            ret, frame = self.camera.read_frame()
            if not ret or frame is None:
                print("[WARN] Dropped frame from camera sensor.")
                time.sleep(0.05)
                continue

            current_time = time.time()

            # 1. Run YOLO + ByteTrack
            results = self.model.track(
                source=frame,
                persist=True,
                tracker=cfg.tracker_config,
                classes=[cfg.target_class_id],
                conf=cfg.confidence_threshold,
                device=self.device,
                verbose=False
            )

            current_centroids = {}
            raw_boxes = []

            # 2. Extract Detections & Centroids
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

            # 3. Update Motion State & Evaluate Line Crossing
            for tid, (cx, cy) in current_centroids.items():
                existing_track = self.movement.get_track(tid)
                if existing_track and len(existing_track.positions) > 0:
                    prev_pt = (existing_track.positions[-1][0], existing_track.positions[-1][1])
                    curr_pt = (cx, cy)
                    
                    # Test geometric line crossing
                    self.counter.check_crossing(tid, prev_pt, curr_pt, current_time)

            # Update movement positions
            self.movement.update_tracks(current_centroids, current_time)

            # 4. Draw Detection Badges
            for x1, y1, x2, y2, conf, track_id, (cx, cy) in raw_boxes:
                track = self.movement.get_track(track_id) if track_id else None
                box_color = cfg.COLOR_CYAN if track_id else (120, 120, 120)

                if track and track.is_stationary:
                    box_color = cfg.COLOR_RED

                # Draw bounding box
                cv2.rectangle(frame, (x1, y1), (x2, y2), box_color, 2)

                # Compose badge text: ID, Conf, Dwell time
                if track_id:
                    dwell = f"{track.total_dwell_seconds:.0f}s" if track else "0s"
                    speed = f"{track.speed_px_per_sec:.0f}px/s" if track else ""
                    badge_text = f"ID #{track_id} | {dwell}"
                else:
                    badge_text = f"Person ({conf*100:.0f}%)"

                (tw, th), _ = cv2.getTextSize(badge_text, cv2.FONT_HERSHEY_SIMPLEX, 0.42, 1)
                cv2.rectangle(frame, (x1, y1 - 18), (x1 + tw + 6, y1), box_color, -1)
                cv2.putText(frame, badge_text, (x1 + 3, y1 - 4),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 0, 0), 1, cv2.LINE_AA)

            # 5. Performance Latency & HUD Drawing
            t1 = time.perf_counter()
            frame_time = t1 - t0
            fps = 1.0 / frame_time if frame_time > 0 else 0
            self.fps_buffer.append(fps)
            if len(self.fps_buffer) > 20:
                self.fps_buffer.pop(0)
            avg_fps = sum(self.fps_buffer) / len(self.fps_buffer)

            recent_event = self.counter.get_recent_event_status(current_time)

            self.hud.draw(
                frame=frame,
                active_tracks=self.movement.tracks,
                line_start=self.line_p1,
                line_end=self.line_p2,
                entry_count=self.counter.entry_count,
                exit_count=self.counter.exit_count,
                fps=avg_fps,
                latency_ms=frame_time * 1000,
                device_str=self.device,
                recent_event=recent_event
            )

            cv2.imshow("CPR - Crowd Pressure Radar (Line Counter Engine)", frame)

            # 6. Interactive Keyboard Handlers
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('r'):
                self.counter.entry_count = 0
                self.counter.exit_count = 0
                print("[CPR-Stage2] In/Out Counters reset to zero.")
            elif key == ord('w'):
                self.line_y_ratio = max(0.15, self.line_y_ratio - 0.05)
                self._calculate_line_points()
                self.counter.update_geometry(self.line_p1, self.line_p2)
                print(f"[CPR-Stage2] Counting Line moved UP to {self.line_y_ratio*100:.0f}%")
            elif key == ord('s'):
                self.line_y_ratio = min(0.85, self.line_y_ratio + 0.05)
                self._calculate_line_points()
                self.counter.update_geometry(self.line_p1, self.line_p2)
                print(f"[CPR-Stage2] Counting Line moved DOWN to {self.line_y_ratio*100:.0f}%")

        self.cleanup()

    def cleanup(self):
        self.camera.release()
        cv2.destroyAllWindows()
        print("[CPR-Stage2] Resources released successfully.")

if __name__ == "__main__":
    runner = CPRStage2Runner()
    runner.run()
