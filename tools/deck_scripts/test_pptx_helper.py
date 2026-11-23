import os
import pptx
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN

pptx_path = r"/Users/subhadip/subhansu'/oaic-crowd/docs/pitch/Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
prs = Presentation(pptx_path)

# Theme Colors
COLOR_DARK = RGBColor(0x24, 0x2D, 0x39)
COLOR_SLATE = RGBColor(0x63, 0x70, 0x80)
COLOR_GREEN = RGBColor(0x57, 0xAE, 0x30)
COLOR_RED = RGBColor(0xE6, 0x14, 0x37)
COLOR_TEAL = RGBColor(0x1E, 0xAF, 0xA6)
COLOR_GOLD = RGBColor(0xF6, 0xB8, 0x34)

def set_text(shape, text, font_size=12, bold=False, color=COLOR_DARK, align=PP_ALIGN.LEFT):
    tf = shape.text_frame
    tf.word_wrap = True
    # Clear existing paragraphs
    p = tf.paragraphs[0]
    p.text = text
    p.alignment = align
    p.font.name = 'Aptos'
    p.font.size = Pt(font_size)
    p.font.bold = bold
    p.font.color.rgb = color
    # Remove any extra paragraphs
    for p_extra in tf.paragraphs[1:]:
        p_extra.text = ""

def set_bullet_points(shape, items, font_size=11, bold_prefix_len=0, color=COLOR_DARK):
    tf = shape.text_frame
    tf.word_wrap = True
    for i, item in enumerate(items):
        if i == 0:
            p = tf.paragraphs[0]
        else:
            p = tf.add_paragraph()
        p.text = item
        p.alignment = PP_ALIGN.LEFT
        p.font.name = 'Aptos'
        p.font.size = Pt(font_size)
        p.font.color.rgb = color
        p.font.bold = False

print("Helper functions initialized successfully.")
