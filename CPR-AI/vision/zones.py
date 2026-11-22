"""
CPR AI Vision System - Configurable ROI Zones & Spatial Containment
Defines virtual geometric zones (e.g. Corridor, Gate Threshold, Waiting Area) and tracks localized occupancy.
"""
import cv2
import numpy as np
from typing import List, Tuple, Dict

class ZoneManager:
    def __init__(self, zone_id: str, polygon_ratios: List[Tuple[float, float]], frame_w: int, frame_h: int):
        self.zone_id = zone_id
        self.polygon_ratios = polygon_ratios
        self.frame_w = frame_w
        self.frame_h = frame_h
        self.polygon_pts = self._compute_pixel_points()

    def _compute_pixel_points(self) -> np.ndarray:
        pts = []
        for rx, ry in self.polygon_ratios:
            pts.append([int(rx * self.frame_w), int(ry * self.frame_h)])
        return np.array(pts, dtype=np.int32)

    def update_resolution(self, frame_w: int, frame_h: int):
        self.frame_w = frame_w
        self.frame_h = frame_h
        self.polygon_pts = self._compute_pixel_points()

    def contains_point(self, point: Tuple[int, int]) -> bool:
        """Returns True if point (cx, cy) is inside the polygon."""
        dist = cv2.pointPolygonTest(self.polygon_pts, (float(point[0]), float(point[1])), False)
        return dist >= 0

    def filter_occupancy(self, centroids: Dict[int, Tuple[int, int]]) -> List[int]:
        """Returns list of track IDs located inside this zone."""
        inside_ids = []
        for tid, pt in centroids.items():
            if self.contains_point(pt):
                inside_ids.append(tid)
        return inside_ids

    def draw(self, frame: np.ndarray, color: Tuple[int, int, int] = (255, 191, 0), alpha: float = 0.12):
        """Draws shaded polygon boundary with clear technical perimeter."""
        overlay = frame.copy()
        cv2.fillPoly(overlay, [self.polygon_pts], color)
        cv2.addWeighted(overlay, alpha, frame, 1.0 - alpha, 0, frame)
        cv2.polylines(frame, [self.polygon_pts], True, color, 1, cv2.LINE_AA)

        # Zone Label
        top_left = self.polygon_pts[0]
        cv2.putText(frame, f"ZONE: {self.zone_id.upper()}", (top_left[0] + 6, top_left[1] + 16),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.42, color, 1, cv2.LINE_AA)
