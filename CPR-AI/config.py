"""
CPR AI Vision System - Global Production Configuration
"""
from dataclasses import dataclass, field
from typing import Tuple, List
import os

@dataclass
class VisionConfig:
    # -------------------------------------------------------------
    # 1. Camera & Video Source Settings
    # -------------------------------------------------------------
    # source: can be an integer index (0, 1) or IP stream URL (e.g. "http://192.168.1.50:8080/video")
    # or path to recorded test video file (e.g. "test_crowd.mp4")
    camera_source: str = os.getenv("CPR_CAMERA_SOURCE", "0")
    frame_width: int = 640
    frame_height: int = 480
    target_fps: int = 30
    auto_reconnect_delay: float = 2.0
    
    # Low-Light & Image Preprocessing
    enable_clahe: bool = True           # Adaptive histogram equalization for dark rooms/cheap sensors
    clahe_clip_limit: float = 2.2
    gamma_correction: float = 1.15       # > 1.0 boosts shadows without blowing out highlights

    # -------------------------------------------------------------
    # 2. YOLO & ByteTrack Detector Settings
    # -------------------------------------------------------------
    model_name: str = os.getenv("CPR_YOLO_MODEL", "yolo11n.pt")
    confidence_threshold: float = 0.40
    iou_threshold: float = 0.45         # NMS IoU threshold for overlapping crowds
    target_class_id: int = 0            # 0 = person in COCO
    tracker_config: str = "bytetrack.yaml"

    # -------------------------------------------------------------
    # 3. Virtual Counting Line Geometry (Relative ratios: 0.0 - 1.0)
    # -------------------------------------------------------------
    line_start_ratio: Tuple[float, float] = (0.05, 0.55)
    line_end_ratio: Tuple[float, float] = (0.95, 0.55)
    line_debounce_seconds: float = 1.5

    # -------------------------------------------------------------
    # 4. Movement, Dwell Time & Stationary Analytics
    # -------------------------------------------------------------
    speed_smoothing_window: int = 6
    stationary_speed_threshold: float = 25.0   # Pixels/sec
    stationary_dwell_seconds: float = 3.0     # Time at low speed to trigger alert
    max_trajectory_history: int = 25

    # -------------------------------------------------------------
    # 5. Zone / ROI Congestion & Crowd Pressure Estimator
    # -------------------------------------------------------------
    # Default monitoring polygon (entire center corridor or customizable)
    zone_polygon_ratios: List[Tuple[float, float]] = field(default_factory=lambda: [
        (0.08, 0.15),
        (0.92, 0.15),
        (0.92, 0.88),
        (0.08, 0.88)
    ])
    
    green_occupancy_max: int = 3
    amber_occupancy_max: int = 6
    # > 6 triggers RED congestion alert

    # -------------------------------------------------------------
    # 6. Backend Integration & Telemetry Pipeline
    # -------------------------------------------------------------
    backend_url: str = os.getenv("CPR_BACKEND_URL", "http://localhost:5000/api/telemetry")
    auth_token: str = os.getenv("CPR_AUTH_TOKEN", "cpr-exhibition-secret-2026")
    device_id: str = "cpr-vision-gate-01"
    zone_id: str = "zone-corridor-a"
    telemetry_rate_hz: float = 2.0      # Broadcast telemetry twice per second
    firmware_version: str = "cpr-ai-v2.4-cuda"

    # -------------------------------------------------------------
    # 7. Visual Palette (BGR for OpenCV)
    # -------------------------------------------------------------
    COLOR_CYAN: Tuple[int, int, int] = (255, 229, 0)
    COLOR_GREEN: Tuple[int, int, int] = (50, 205, 50)
    COLOR_AMBER: Tuple[int, int, int] = (0, 165, 255)
    COLOR_RED: Tuple[int, int, int] = (30, 30, 240)
    COLOR_LINE: Tuple[int, int, int] = (255, 191, 0)
    COLOR_DARK_SLATE: Tuple[int, int, int] = (15, 23, 42)

cfg = VisionConfig()
