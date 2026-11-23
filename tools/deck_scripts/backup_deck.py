import os
import shutil
import pptx
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor

original_pptx = r"C:\Users\subha\Downloads\crowd-pressure-radar\Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
backup_pptx = r"C:\Users\subha\Downloads\crowd-pressure-radar\Odisha_AI_Hackathon_Idea_Pitchdeck_Template_ORIGINAL_BACKUP.pptx"

if not os.path.exists(backup_pptx):
    shutil.copyfile(original_pptx, backup_pptx)
    print(f"Created backup: {backup_pptx}")
else:
    print(f"Backup already exists: {backup_pptx}")
