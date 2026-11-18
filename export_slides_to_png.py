import os
import win32com.client

pptx_path = os.path.abspath(r"C:\Users\subha\Downloads\crowd-pressure-radar\Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx")
output_dir = os.path.abspath(r"C:\Users\subha\Downloads\crowd-pressure-radar\pitch_slides_png")
os.makedirs(output_dir, exist_ok=True)

powerpoint = win32com.client.Dispatch("PowerPoint.Application")
# Keep window minimized or background
try:
    deck = powerpoint.Presentations.Open(pptx_path, WithWindow=False)
    for i, slide in enumerate(deck.Slides):
        out_file = os.path.join(output_dir, f"slide_{i+1}.png")
        slide.Export(out_file, "PNG", 1920, 1080)
        print(f"Exported: {out_file}")
    deck.Close()
finally:
    powerpoint.Quit()

print("All slides exported successfully.")
