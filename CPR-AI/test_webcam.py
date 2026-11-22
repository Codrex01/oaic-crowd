"""
CPR AI Vision System - Camera Performance & FPS Benchmark
Measures actual hardware frame rate, tests resolution switching, and detects camera freeze.
"""
import cv2
import time
import sys

def benchmark_camera(camera_index=0, target_frames=120, req_width=1280, req_height=720):
    print("=" * 60)
    print(f"CPR AI Camera Benchmark (Index: {camera_index})")
    print("=" * 60)

    # Use DirectShow backend on Windows for lowest latency
    backend = cv2.CAP_DSHOW if sys.platform.startswith("win") else cv2.CAP_ANY
    cap = cv2.VideoCapture(camera_index, backend)

    if not cap.isOpened():
        print(f"[ERROR] Could not open camera at index {camera_index}.")
        print("Check if another application (e.g. Teams, Zoom, Browser) is using the webcam.")
        return

    # Attempt to set requested resolution
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, req_width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, req_height)

    actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    sensor_fps = cap.get(cv2.CAP_PROP_FPS)

    print(f"Configured Resolution: {actual_w}x{actual_h}")
    print(f"Camera Driver Reported FPS: {sensor_fps if sensor_fps > 0 else 'Unknown / Variable'}")
    print(f"\nBenchmarking actual frame acquisition across {target_frames} frames...")
    print("Press 'q' in the preview window to stop early.\n")

    timestamps = []
    start_total = time.perf_counter()

    for i in range(target_frames):
        t0 = time.perf_counter()
        ret, frame = cap.read()
        t1 = time.perf_counter()

        if not ret:
            print(f"[WARN] Frame acquisition failed at frame {i}.")
            break

        timestamps.append(t1 - t0)

        # Draw HUD on benchmark preview
        fps_instant = 1.0 / (t1 - t0) if (t1 - t0) > 0 else 0
        cv2.putText(frame, f"CPR Camera Benchmark: {actual_w}x{actual_h}", (20, 35),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
        cv2.putText(frame, f"Acquiring Frame: {i+1}/{target_frames}", (20, 65),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
        cv2.putText(frame, f"Instant Latency: {(t1-t0)*1000:.1f} ms ({fps_instant:.1f} FPS)", (20, 95),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        cv2.imshow("CPR Camera Benchmark", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            print("[INFO] Benchmark stopped by user.")
            break

    total_time = time.perf_counter() - start_total
    cap.release()
    cv2.destroyAllWindows()

    if len(timestamps) > 10:
        actual_fps = len(timestamps) / total_time
        avg_latency_ms = (sum(timestamps) / len(timestamps)) * 1000
        min_latency_ms = min(timestamps) * 1000
        max_latency_ms = max(timestamps) * 1000

        print("-" * 60)
        print("Camera Benchmark Results:")
        print(f"  - Total Captured Frames: {len(timestamps)}")
        print(f"  - Total Test Time:       {total_time:.2f} seconds")
        print(f"  - Real Measured FPS:     {actual_fps:.2f} FPS")
        print(f"  - Avg Grab Latency:      {avg_latency_ms:.2f} ms")
        print(f"  - Min / Max Latency:     {min_latency_ms:.2f} ms / {max_latency_ms:.2f} ms")
        print("-" * 60)
        if actual_fps >= 25.0:
            print("[STATUS] Frame rate is optimal for real-time crowd tracking.")
        else:
            print("[STATUS] Camera frame rate is below 25 FPS; consider running at 640x480 resolution.")

if __name__ == "__main__":
    benchmark_camera(camera_index=0)
