"""
CPR AI Vision System - Stage 1: YOLO Person Detection + ByteTrack Multi-Person Tracking
Features:
- Automatic CUDA GPU detection with fallback to CPU / Apple MPS.
- Ultralytics YOLO (YOLO11n default) strictly filtered to class 0 ('person').
- ByteTrack integration with persistent anonymous Track ID assignment.
- Technical HUD overlay (FPS, Device, Person Count, Tracked IDs).
"""

import cv2
import time
import sys
import torch
from ultralytics import YOLO

class CPRPersonTracker:
    def __init__(self, model_name: str = "yolo11n.pt", camera_index: int = 0, conf_threshold: float = 0.45):
        self.conf_threshold = conf_threshold
        self.camera_index = camera_index
        
        # 1. Device Resolution (CUDA -> MPS -> CPU)
        self.device = self._resolve_compute_device()
        print(f"[CPR-Tracker] Target Compute Device: {self.device}")

        # 2. Model Initialization
        print(f"[CPR-Tracker] Loading YOLO detector: {model_name}...")
        self.model = YOLO(model_name)
        # Move model to target device
        self.model.to(self.device)
        print("[CPR-Tracker] Model loaded successfully.")

        # 3. Camera Setup
        backend = cv2.CAP_DSHOW if sys.platform.startswith("win") else cv2.CAP_ANY
        self.cap = cv2.VideoCapture(self.camera_index, backend)
        
        # Request 1280x720 (falls back to native camera max if unsupported)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
        
        if not self.cap.isOpened():
            raise RuntimeError(f"Unable to access camera on index {self.camera_index}")
        
        self.frame_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.frame_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        print(f"[CPR-Tracker] Camera initialized: {self.frame_width}x{self.frame_height}")

        # Performance tracking
        self.fps_history = []
        self.prev_time = time.perf_counter()

    def _resolve_compute_device(self) -> str:
        """Autodetects NVIDIA CUDA, Apple Silicon MPS, or CPU."""
        if torch.cuda.is_available():
            gpu_name = torch.cuda.get_device_name(0)
            print(f"[CPR-Tracker] GPU Acceleration: ENABLED ({gpu_name})")
            return "cuda:0"
        elif hasattr(torch.backends, "mps") and torch.backends.mps.is_available():
            print("[CPR-Tracker] Apple Silicon GPU: ENABLED (MPS)")
            return "mps"
        else:
            print("[CPR-Tracker] WARNING: GPU not available. Running on CPU.")
            return "cpu"

    def run(self):
        print("\n" + "=" * 65)
        print("  CPR AI Crowd Pressure Radar — Stage 1 Live Tracking")
        print("  Controls: Press 'q' to quit | 'c' to clear frame")
        print("=" * 65 + "\n")

        while True:
            t0 = time.perf_counter()
            ret, frame = self.cap.read()
            if not ret:
                print("[ERROR] Camera frame drop detected. Attempting to recover...")
                time.sleep(0.1)
                continue

            # Run YOLO + ByteTrack
            # classes=[0] filters COCO class 0 ('person') directly in tensors
            # persist=True preserves IDs across frames in ByteTrack
            results = self.model.track(
                source=frame,
                persist=True,
                tracker="bytetrack.yaml",
                classes=[0],
                conf=self.conf_threshold,
                device=self.device,
                verbose=False
            )

            current_people_count = 0
            tracked_ids = []

            # Parse bounding boxes and ByteTrack IDs
            if results and len(results) > 0 and results[0].boxes is not None:
                boxes = results[0].boxes
                for box in boxes:
                    # Coordinates in integer pixels
                    x1, y1, x2, y2 = map(int, box.xyxy[0].cpu().numpy())
                    conf = float(box.conf[0].cpu().numpy())
                    
                    # Track ID (None if detection not yet confirmed by ByteTrack)
                    track_id = int(box.id[0].cpu().numpy()) if box.id is not None else None
                    
                    current_people_count += 1
                    if track_id is not None:
                        tracked_ids.append(track_id)

                    # Bounding Box Color Coding:
                    # High confidence: Cyan/Teal (#00E5FF), Lower: Light Blue
                    box_color = (255, 229, 0) if track_id is not None else (120, 120, 120)

                    # Draw Bounding Box (Corner Bracket style for clean technical look)
                    cv2.rectangle(frame, (x1, y1), (x2, y2), box_color, 2)

                    # Draw Track Badge
                    badge_text = f"ID #{track_id} ({conf*100:.0f}%)" if track_id is not None else f"Person ({conf*100:.0f}%)"
                    (w_text, h_text), _ = cv2.getTextSize(badge_text, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
                    
                    # Badge background
                    cv2.rectangle(frame, (x1, y1 - 20), (x1 + w_text + 8, y1), box_color, -1)
                    cv2.putText(frame, badge_text, (x1 + 4, y1 - 5),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 0, 0), 1, cv2.LINE_AA)

            # Compute actual inference + rendering FPS
            t1 = time.perf_counter()
            frame_time = t1 - t0
            current_fps = 1.0 / frame_time if frame_time > 0 else 0
            self.fps_history.append(current_fps)
            if len(self.fps_history) > 30:
                self.fps_history.pop(0)
            avg_fps = sum(self.fps_history) / len(self.fps_history)

            # Draw Technical Monitoring HUD
            self._draw_monitoring_hud(frame, current_people_count, tracked_ids, avg_fps, frame_time)

            cv2.imshow("CPR - Crowd Pressure Radar (Vision Engine)", frame)
            
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                print("[CPR-Tracker] User requested shutdown.")
                break

        self.cleanup()

    def _draw_monitoring_hud(self, frame, count: int, ids: list, fps: float, latency: float):
        """Draws an enterprise cybersecurity/IoT technical monitoring HUD."""
        h, w = frame.shape[:2]

        # Top Control Banner (Semi-transparent black)
        overlay = frame.copy()
        cv2.rectangle(overlay, (0, 0), (w, 48), (15, 23, 42), -1)
        # Bottom Telemetry Bar
        cv2.rectangle(overlay, (0, h - 32), (w, h), (15, 23, 42), -1)
        cv2.addWeighted(overlay, 0.75, frame, 0.25, 0, frame)

        # Title & System Badge
        cv2.putText(frame, "CPR VISION SYSTEM", (16, 28),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 255), 2, cv2.LINE_AA)
        cv2.putText(frame, "RADAR v1.0", (220, 28),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, (148, 163, 184), 1, cv2.LINE_AA)

        # Compute Device Indicator
        dev_color = (0, 255, 0) if "cuda" in self.device else (0, 165, 255)
        dev_label = f"BACKEND: {self.device.upper()}"
        cv2.putText(frame, dev_label, (w - 240, 28),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, dev_color, 1, cv2.LINE_AA)

        # FPS & Inference Latency
        fps_label = f"FPS: {fps:.1f} ({latency*1000:.1f}ms)"
        cv2.putText(frame, fps_label, (w - 420, 28),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)

        # Bottom Bar: Person Count & Active Track IDs
        count_color = (0, 255, 0) if count < 5 else ((0, 165, 255) if count < 10 else (0, 0, 255))
        cv2.putText(frame, f"VISIBLE PERSONS: {count}", (16, h - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, count_color, 2, cv2.LINE_AA)

        id_str = ", ".join(map(str, ids[:8])) + ("..." if len(ids) > 8 else "")
        cv2.putText(frame, f"ACTIVE TRACK IDS: [{id_str}]", (240, h - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, (203, 213, 225), 1, cv2.LINE_AA)

    def cleanup(self):
        self.cap.release()
        cv2.destroyAllWindows()
        print("[CPR-Tracker] Camera released. Resources cleared.")

if __name__ == "__main__":
    tracker = CPRPersonTracker(model_name="yolo11n.pt", camera_index=0, conf_threshold=0.45)
    tracker.run()
