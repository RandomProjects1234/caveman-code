"""The OGABOOGA CODER look: warm cave colors and big friendly fonts."""

BG = "#241a12"
PANEL = "#31241a"
PANEL2 = "#3d2d1f"
PANEL3 = "#4a3728"
TEXT = "#f7ead9"
DIM = "#b39b80"
ACCENT = "#ffb340"
GREEN = "#4cc38a"
RED = "#ff6b6b"
BLUE = "#64b5f6"
PURPLE = "#c792ea"
YELLOW = "#ffd166"
WHITE = "#ffffff"

FONT_UI = ("Segoe UI", 11)
FONT_UI_BOLD = ("Segoe UI", 11, "bold")
FONT_UI_BIG = ("Segoe UI", 14, "bold")
FONT_TITLE = ("Segoe UI", 20, "bold")
FONT_SMALL = ("Segoe UI", 9)
FONT_CODE = ("Consolas", 12)
FONT_CODE_SMALL = ("Consolas", 10)

# colors for the drag-and-drop blocks
BLOCK_COLORS = {
    "say": "#ffb340",
    "ask": "#ffd166",
    "grunk": "#64b5f6",
    "set": "#4aa3df",
    "if": "#c792ea",
    "repeat": "#4cc38a",
    "while": "#35b07a",
    "foreach": "#2f9e6e",
    "clump": "#f38ba8",
    "ork": "#e06c8c",
    "call": "#f2a2b8",
    "set_index": "#5aa9d6",
    "pile_make": "#8fd694",
    "pile_plop": "#6fc47a",
    "pile_yoink": "#54ae66",
    "value_number": "#d0c4e8",
    "value_text": "#d0c4e8",
    "value_var": "#d0c4e8",
    "value_random": "#d0c4e8",
    "skrib": "#f9c74f",
}

DEFAULT_BLOCK_COLOR = "#b39b80"

CATEGORY_COLORS = {
    "Talking": "#ffb340",
    "Boxes": "#64b5f6",
    "Choices": "#c792ea",
    "Loops": "#4cc38a",
    "Clumps": "#f38ba8",
    "Values": "#d0c4e8",
    "Piles": "#8fd694",
    "Drawing": "#f9c74f",
}


def block_color(kind):
    if kind.startswith("skrib"):
        return BLOCK_COLORS["skrib"]
    return BLOCK_COLORS.get(kind, DEFAULT_BLOCK_COLOR)