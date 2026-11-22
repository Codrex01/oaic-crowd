"""
CPR AI Vision System - Multi-Source Camera Stream Engine
Features:
- Seamless support for USB webcams, IP/Mobile phone streams (DroidCam, Iriun, IP Webcam HTTP/RTSP), and video files.
- Adaptive low-light enhancement: CLAHE (Contrast Limited Adaptive Histogram Equalization) and gamma boost.
- Automatic disconnection detection and background thread reconnection.
"""
import cv2
import time
import sys
import numpy as np
from typing import Optional, Tuple

class AdaptiveCamera:
    def __init__(self, source: str = "0", req_width: int = 640, req_height: int = 480,
                 enable_clahe: bool = True, clahe_clip: float = 2.2, gamma: float = 1.15):
        self.source_str = source
        self.req_width = req_width
        self.req_height = req_height
        
        # Enhancement switches
        self.enable_clahe = enable_clahe
        self.clahe_clip = clahe_clip
        self.gamma = gamma
        self.clahe = cv2.createCLAHE(clipLimit=self.clahe_clip, tileGridSize=(8, 8)) if enable_clahe else None

        # Build gamma lookup table once
        inv_gamma = 1.0 / self.gamma
        self.gamma_lut = np.array([((i / 255.0) ** inv_gamma) * 255 for i in range(256)]).astype("uint8")

        self.cap: Optional[cv2.VideoCapture] = None
        self.is_connected: bool = False
        self.actual_width: int = 0
        self.actual_height: int = 0
        self.is_video_file: bool = False

        self._connect()

    def _connect(self) -> bool:
        """Opens capture device using appropriate OS backend."""
        # Check if source is integer (USB webcam index) or URL/file string
        try:
            device_index = int(self.source_str)
            backend = cv2.CAP_DSHOW if sys.platform.startswith("win") else cv2.CAP_ANY
            self.cap = cv2.VideoCapture(device_index, backend)
            self.is_video_file = False
        except ValueError:
            # Source is an RTSP/HTTP URL or file path
            self.cap = cv2.VideoCapture(self.source_str)
            self.is_video_file = self.source_str.lower().endswith(('.mp4', '.avi', '.mkv', '.mov'))

        if not self.cap.isOpened():
            self.is_connected = False
            return False

        # Set resolution for webcams
        if not self.is_video_file:
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.req_width)
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.req_height)

        self.actual_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.actual_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.is_connected = True
        return True

    def read_frame(self) -> Tuple[bool, Optional[np.ndarray]]:
        """Reads a frame with error handling and optional adaptive low-light enhancement."""
        if not self.is_connected or self.cap is None:
            # Attempt to reconnect
            if self._connect():
                print("[CPR-Camera] Reconnection established successfully.")
            else:
                return False, None

        ret, frame = self.cap.read()

        # Handle video file loop for continuous exhibition demonstrations
        if not ret and self.is_video_file:
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
            ret, frame = self.cap.read()

        if not ret or frame is None:
            self.is_connected = False
            return False, None

        # Apply low-light preprocessing if enabled
        if self.enable_clahe:
            frame = self._enhance_frame(frame)

        return True, frame

    def _enhance_frame(self, frame: np.ndarray) -> np.ndarray:
        """Enhances contrast and dark shadow visibility in low-light environments."""
        # 1. CLAHE in LAB color space
        lab = cv2.cvtColor(frame, cv2.COLOR_BGR2LAB)
        l, a, b = cv2.split(lab)
        l_clahe = self.clahe.apply(l)
        enhanced_lab = cv2.merge((l_clahe, a, b))
        bgr = cv2.cvtColor(enhanced_lab, cv2.COLOR_LAB2BGR)

        # 2. Gamma lookup table
        if abs(self.gamma - 1.0) > 0.05:
            bgr = cv2.LUT(bgr, self.gamma_lut)

        return bgr

    def toggle_enhancement(self) -> bool:
        """Toggles low-light filter on/off dynamically."""
        self.enable_clahe = not self.enable_clahe
        if self.enable_clahe and self.clahe is None:
            self.clahe = cv2.createCLAHE(clipLimit=self.clahe_clip, tileGridSize=(8, 8))
        return self.enable_clahe

    def release(self):
        if self.cap is not None:
            self.cap.release()
            self.is_connected = False
