"""
CPR AI Vision System - Virtual Counting Line & Bidirectional Crossing Engine
Implements vector geometric ray-intersection to calculate Entry and Exit events with hysteresis debounce.
"""
import time
from typing import Tuple, Dict, Optional, Set
import numpy as np

def ccw(A: Tuple[int, int], B: Tuple[int, int], C: Tuple[int, int]) -> bool:
    """Tests if points A, B, C are in counter-clockwise order."""
    return (C[1] - A[1]) * (B[0] - A[0]) > (B[1] - A[1]) * (C[0] - A[0])

def segments_intersect(A: Tuple[int, int], B: Tuple[int, int], 
                        C: Tuple[int, int], D: Tuple[int, int]) -> bool:
    """Returns True if line segment AB intersects line segment CD."""
    return (ccw(A, C, D) != ccw(B, C, D)) and (ccw(A, B, C) != ccw(A, B, D))

class VirtualLineCounter:
    def __init__(self, start_pt: Tuple[int, int], end_pt: Tuple[int, int], debounce_seconds: float = 1.5):
        self.p1 = start_pt
        self.p2 = end_pt
        self.debounce_seconds = debounce_seconds

        self.entry_count: int = 0
        self.exit_count: int = 0

        # Normal vector perpendicular to the line for direction detection
        dx = self.p2[0] - self.p1[0]
        dy = self.p2[1] - self.p1[1]
        length = np.hypot(dx, dy) + 1e-6
        # Unit normal pointing "downward / entry"
        self.normal = (-dy / length, dx / length)

        # Track ID cooldown dictionary: track_id -> timestamp of last counted crossing
        self.last_crossed: Dict[int, float] = {}
        
        # Recent crossing events for visual flash effect (timestamp, 'ENTRY' | 'EXIT')
        self.recent_events = []

    def update_geometry(self, start_pt: Tuple[int, int], end_pt: Tuple[int, int]):
        self.p1 = start_pt
        self.p2 = end_pt
        dx = self.p2[0] - self.p1[0]
        dy = self.p2[1] - self.p1[1]
        length = np.hypot(dx, dy) + 1e-6
        self.normal = (-dy / length, dx / length)

    def check_crossing(self, track_id: int, prev_centroid: Tuple[int, int], curr_centroid: Tuple[int, int], current_time: float) -> Optional[str]:
        """
        Evaluates whether movement from prev_centroid to curr_centroid crossed the line.
        Returns: 'ENTRY', 'EXIT', or None
        """
        # 1. Check debounce
        if track_id in self.last_crossed:
            if (current_time - self.last_crossed[track_id]) < self.debounce_seconds:
                return None

        # 2. Check if trajectory segment intersects counting line
        if not segments_intersect(self.p1, self.p2, prev_centroid, curr_centroid):
            return None

        # 3. Calculate direction vector of motion
        motion_x = curr_centroid[0] - prev_centroid[0]
        motion_y = curr_centroid[1] - prev_centroid[1]
        
        # Dot product with line normal determines crossing direction
        dot_prod = (motion_x * self.normal[0]) + (motion_y * self.normal[1])

        event_type = None
        if dot_prod > 0:
            self.entry_count += 1
            event_type = "ENTRY"
        else:
            self.exit_count += 1
            event_type = "EXIT"

        self.last_crossed[track_id] = current_time
        self.recent_events.append((current_time, event_type))
        return event_type

    def get_recent_event_status(self, current_time: float) -> Optional[str]:
        """Returns the most recent event if occurred within 0.8 seconds (for UI flashing)."""
        self.recent_events = [e for e in self.recent_events if (current_time - e[0]) < 0.8]
        if self.recent_events:
            return self.recent_events[-1][1]
        return None
