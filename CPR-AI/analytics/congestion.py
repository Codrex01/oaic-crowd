"""
CPR AI Vision System - Crowd Congestion & Operational Pressure Estimator
Calculates:
- Crowd Density Index (Occupancy vs Target Threshold)
- Stop-and-Go Index (Ratio of stationary individuals to moving flow)
- Average Velocity of tracked crowd flow
- Operational Status (GREEN, AMBER, RED)
"""
from typing import Dict, List, Tuple
import numpy as np
from vision.movement import PersonTrackState

class CongestionEstimator:
    def __init__(self, green_max: int = 3, amber_max: int = 6):
        self.green_max = green_max
        self.amber_max = amber_max

    def evaluate(self, active_tracks: Dict[int, PersonTrackState], zone_ids: List[int]) -> Dict:
        """
        Evaluates crowd state for the specified zone members.
        """
        zone_tracks = [active_tracks[tid] for tid in zone_ids if tid in active_tracks]
        occupancy = len(zone_tracks)

        if occupancy == 0:
            return {
                "state": "GREEN",
                "score": 0.0,
                "density": 0.0,
                "avg_speed": 0.0,
                "stop_go_index": 0.0,
                "stationary_count": 0,
                "occupancy": 0,
                "alerts": []
            }

        # 1. Speeds and Stationary analysis
        speeds = [t.speed_px_per_sec for t in zone_tracks]
        avg_speed = float(np.mean(speeds)) if speeds else 0.0
        
        stationary_count = sum(1 for t in zone_tracks if t.is_stationary)
        stop_go_index = float(stationary_count / occupancy) if occupancy > 0 else 0.0

        # 2. Normalized Density relative to Amber Max
        density_norm = min(1.0, occupancy / max(1, self.amber_max))

        # 3. Composite Crowd Pressure Score (0.0 to 1.0)
        # Weighting: 50% Occupancy load + 30% Stop-and-Go turbulence + 20% Flow deceleration
        normal_speed = 60.0  # reference pixel velocity for brisk pedestrian walking
        speed_deceleration = max(0.0, 1.0 - min(1.0, avg_speed / normal_speed))
        
        score = (0.50 * density_norm) + (0.30 * stop_go_index) + (0.20 * speed_deceleration)
        score = round(float(np.clip(score, 0.0, 1.0)), 2)

        # 4. Operational State Machine
        alerts = []
        if occupancy > self.amber_max or score >= 0.75 or (stop_go_index >= 0.6 and occupancy >= 4):
            state = "RED"
            alerts.append("CRITICAL_CONGESTION_LEVEL")
            if stop_go_index >= 0.6:
                alerts.append("BOTTLENECK_STATIONARY_CLUSTER")
        elif occupancy > self.green_max or score >= 0.42:
            state = "AMBER"
            alerts.append("ELEVATED_DENSITY_WARNING")
        else:
            state = "GREEN"

        if stationary_count >= 2:
            alerts.append("MULTIPLE_STATIONARY_PERSONS")

        return {
            "state": state,
            "score": score,
            "density": round(density_norm, 2),
            "avg_speed": round(avg_speed, 1),
            "stop_go_index": round(stop_go_index, 2),
            "stationary_count": stationary_count,
            "occupancy": occupancy,
            "alerts": alerts
        }
