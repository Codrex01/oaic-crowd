"""
CPR AI Vision System - Technical HUD & Information Overlay
Renders high-contrast, scalable HUD banners, directional line graphics, motion trails, and alert badges.
"""
import cv2
import numpy as np
from typing import Dict, Tuple, List, Optional
from vision.movement import PersonTrackState

class HUDOverlay:
    def __init__(self, config):
        self.cfg = config

    def draw(self, frame: np.ndarray, 
             active_tracks: Dict[int, PersonTrackState],
             line_start: Tuple[int, int],
             line_end: Tuple[int, int],
             entry_count: int,
             exit_count: int,
             fps: float,
             latency_ms: float,
             device_str: str,
             recent_event: Optional[str] = None):
        
        h, w = frame.shape[:2]

        # 1. Draw Virtual Counting Line & Direction Guide
        self._draw_counting_line(frame, line_start, line_end, recent_event)

        # 2. Draw Person Motion Trails & Bounding Boxes
        self._draw_tracks(frame, active_tracks)

        # 3. Draw Top Information Banner
        self._draw_top_banner(frame, fps, latency_ms, device_str, w)

        # 4. Draw Bottom Telemetry Bar
        self._draw_bottom_bar(frame, len(active_tracks), entry_count, exit_count, h, w)

    def _draw_counting_line(self, frame, p1, p2, recent_event):
        """Draws the virtual sensor line with directional arrows and event flash."""
        line_color = self.cfg.COLOR_LINE
        thickness = 2

        if recent_event == "ENTRY":
            line_color = (0, 255, 0)  # Bright Green flash
            thickness = 4
        elif recent_event == "EXIT":
            line_color = (0, 165, 255) # Bright Orange flash
            thickness = 4

        # Draw main counting threshold
        cv2.line(frame, p1, p2, line_color, thickness, cv2.LINE_AA)
        
        # Center marker & Arrow
        mid_x = (p1[0] + p2[0]) // 2
        mid_y = (p1[1] + p2[1]) // 2
        cv2.circle(frame, (mid_x, mid_y), 5, line_color, -1)
        
        # Direction Indicator Badges
        cv2.putText(frame, "ENTRY [v]", (p1[0] + 8, p1[1] + 18),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.4, (50, 205, 50), 1, cv2.LINE_AA)
        cv2.putText(frame, "EXIT [^]", (p2[0] - 65, p2[1] - 8),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 165, 255), 1, cv2.LINE_AA)

    def _draw_tracks(self, frame, active_tracks: Dict[int, PersonTrackState]):
        """Renders bounding box, motion tail vector, and stationary alerts."""
        for tid, track in active_tracks.items():
            pts = list(track.positions)
            if not pts:
                continue

            # Draw Motion Trail (historical breadcrumb trail)
            if len(pts) > 1:
                for i in range(1, len(pts)):
                    pt_a = (pts[i-1][0], pts[i-1][1])
                    pt_b = (pts[i][0], pts[i][1])
                    alpha = i / len(pts)
                    trail_color = (int(255 * alpha), int(229 * alpha), 0)
                    cv2.line(frame, pt_a, pt_b, trail_color, 1, cv2.LINE_AA)

            # Current centroid
            curr = pts[-1]
            cx, cy = curr[0], curr[1]

            # Stationary indicator badge
            if track.is_stationary:
                cv2.circle(frame, (cx, cy), 14, (0, 0, 255), 2)
                cv2.putText(frame, "STATIONARY", (cx - 35, cy - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.38, (0, 0, 255), 1, cv2.LINE_AA)

    def _draw_top_banner(self, frame, fps, latency_ms, device_str, w):
        """Top technical HUD bar."""
        overlay = frame.copy()
        cv2.rectangle(overlay, (0, 0), (w, 38), self.cfg.COLOR_DARK_SLATE, -1)
        cv2.addWeighted(overlay, 0.75, frame, 0.25, 0, frame)

        # Title
        cv2.putText(frame, "CPR RADAR - STAGE 2", (12, 24),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 255), 2, cv2.LINE_AA)

        # Performance Stats
        fps_text = f"FPS: {fps:.1f} ({latency_ms:.1f}ms)"
        cv2.putText(frame, fps_text, (w // 2 - 50, 24),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1, cv2.LINE_AA)

        # Hardware Backend
        dev_color = (50, 205, 50) if "cuda" in device_str else (0, 165, 255)
        cv2.putText(frame, device_str.upper(), (w - 110, 24),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, dev_color, 1, cv2.LINE_AA)

    def _draw_bottom_bar(self, frame, live_count, entries, exits, h, w):
        """Bottom operational telemetry bar."""
        overlay = frame.copy()
        cv2.rectangle(overlay, (0, h - 34), (w, h), self.cfg.COLOR_DARK_SLATE, -1)
        cv2.addWeighted(overlay, 0.75, frame, 0.25, 0, frame)

        # Visible Crowd Count
        count_color = self.cfg.COLOR_GREEN if live_count <= 4 else (self.cfg.COLOR_AMBER if live_count <= 8 else self.cfg.COLOR_RED)
        cv2.putText(frame, f"OCCUPANCY: {live_count}", (12, h - 11),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.48, count_color, 2, cv2.LINE_AA)

        # Entry Count
        cv2.putText(frame, f"TOTAL IN: {entries}", (w // 2 - 60, h - 11),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.48, (50, 205, 50), 2, cv2.LINE_AA)

        # Exit Count
        cv2.putText(frame, f"TOTAL OUT: {exits}", (w - 140, h - 11),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.48, (0, 165, 255), 2, cv2.LINE_AA)
