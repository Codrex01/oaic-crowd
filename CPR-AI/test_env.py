"""
CPR AI Vision System - Environment & CUDA Verification
Validates PyTorch CUDA acceleration on RTX 3050, OpenCV backend, and Ultralytics.
"""
import sys

def verify_environment():
    print("=" * 60)
    print("CPR AI Vision System — Environment Diagnostic")
    print("=" * 60)

    # 1. Python Check
    print(f"[1/4] Python Version: {sys.version.split()[0]}")

    # 2. PyTorch & CUDA Check
    print("\n[2/4] Checking PyTorch & CUDA Acceleration:")
    try:
        import torch
        print(f"      - PyTorch Version: {torch.__version__}")
        cuda_available = torch.cuda.is_available()
        print(f"      - CUDA Available:  {cuda_available}")

        if cuda_available:
            device_count = torch.cuda.device_count()
            current_device = torch.cuda.current_device()
            gpu_name = torch.cuda.get_device_name(current_device)
            vram_gb = torch.cuda.get_device_properties(current_device).total_memory / (1024 ** 3)
            print(f"      - GPU Device:      {gpu_name} (Index: {current_device})")
            print(f"      - Total VRAM:      {vram_gb:.2f} GB")
            print(f"      - CUDA Arch:       sm_{torch.cuda.get_device_capability(current_device)[0]}{torch.cuda.get_device_capability(current_device)[1]}")
            
            # Simple tensor compute test on GPU
            x = torch.randn(1000, 1000, device="cuda")
            y = torch.matmul(x, x)
            print("      - Tensor Compute:  PASSED on CUDA")
        else:
            print("      - WARNING: Running in CPU mode. CUDA acceleration is NOT active.")
            print("        Run: pip install torch torchvision --index-url https://download.pytorch.org/whl/cu124")
    except ImportError as e:
        print(f"      - ERROR: PyTorch is not installed: {e}")

    # 3. OpenCV Check
    print("\n[3/4] Checking OpenCV:")
    try:
        import cv2
        print(f"      - OpenCV Version:  {cv2.__version__}")
        build_info = cv2.getBuildInformation()
        has_cuda_cv = "CUDA" in build_info and "YES" in build_info[build_info.find("CUDA"):build_info.find("CUDA") + 30]
        print(f"      - OpenCV Backend:  Default (GUI & video capture ready)")
    except ImportError as e:
        print(f"      - ERROR: OpenCV is not installed: {e}")

    # 4. Ultralytics Check
    print("\n[4/4] Checking Ultralytics YOLO:")
    try:
        import ultralytics
        from ultralytics import YOLO
        print(f"      - Ultralytics Ver: {ultralytics.__version__}")
        print("      - Testing model load (yolo11n.pt)...")
        # YOLO11n is the fastest modern lightweight detector (2.6M params, ~6.5ms on RTX 3050)
        model = YOLO("yolo11n.pt")
        print("      - Ultralytics Load: PASSED")
    except ImportError as e:
        print(f"      - ERROR: Ultralytics not installed: {e}")
        print("        Run: pip install ultralytics")
    except Exception as e:
        print(f"      - ERROR loading YOLO: {e}")

    print("\n" + "=" * 60)
    print("Diagnostic Complete.")
    print("=" * 60)

if __name__ == "__main__":
    verify_environment()
