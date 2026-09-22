"""Coloring CMC words inside the text editor."""

import re

from .boot import cmc
from . import theme

KEYWORDS = sorted(
    set(w["name"] for w in cmc.words.SYNTAX_WORDS if w["name"] != "plop")
    | set(a for w in cmc.words.SYNTAX_WORDS for a in w.get("aliases", []))
    | {"and", "or", "not", "in"},
    key=len,
    reverse=True,
)

BUILTINS = sorted(
    set(w["name"] for w in cmc.words.BUILTIN_WORDS if w["name"] != "plop_call") | {"snorf", "plop"},
    key=len,
    reverse=True,
)

CONSTANTS = ["gronk", "nork", "plop", "pi", "true", "false", "nothing"]

_PATTERNS = [
    ("number", re.compile(r"\b\d+(?:\.\d+)?\b")),
    ("string", re.compile(r'"(?:[^"\\\n]|\\.)*"|\'(?:[^\'\\\n]|\\.)*\'')),
    ("keyword", re.compile(r"\b(?:" + "|".join(re.escape(w) for w in KEYWORDS) + r")\b")),
    ("builtin", re.compile(r"\b(?:" + "|".join(re.escape(w) for w in BUILTINS) + r")\b")),
    ("constant", re.compile(r"\b(?:" + "|".join(CONSTANTS) + r")\b")),
    ("operator", re.compile(r"==|!=|<=|>=|[+\-*/%<>=]|[\[\]()]")),
]

COMMENT_RE = re.compile(r"\bugg\b.*$")

TAG_COLORS = {
    "number": theme.PURPLE,
    "string": theme.GREEN,
    "keyword": theme.ACCENT,
    "builtin": theme.BLUE,
    "constant": theme.PURPLE,
    "operator": theme.DIM,
    "comment": "#8a7a68",
}


def configure(widget):
    widget.tag_configure("comment", foreground=TAG_COLORS["comment"], overstrike=False)
    widget.tag_configure("keyword", foreground=TAG_COLORS["keyword"], font=(theme.FONT_CODE[0], theme.FONT_CODE[1], "bold"))
    widget.tag_configure("builtin", foreground=TAG_COLORS["builtin"])
    widget.tag_configure("constant", foreground=TAG_COLORS["constant"])
    widget.tag_configure("string", foreground=TAG_COLORS["string"])
    widget.tag_configure("number", foreground=TAG_COLORS["number"])
    widget.tag_configure("operator", foreground=TAG_COLORS["operator"])


def highlight(widget):
    """Re-color the whole text widget.  Cheap enough for kid-sized programs."""
    for tag in TAG_COLORS:
        widget.tag_remove(tag, "1.0", "end")
    text = widget.get("1.0", "end-1c")
    if not text.strip():
        return
    for tag, pattern in _PATTERNS:
        for match in pattern.finditer(text):
            start = widget.index("1.0+" + str(match.start()) + "c")
            end = widget.index("1.0+" + str(match.end()) + "c")
            widget.tag_add(tag, start, end)
    for match in COMMENT_RE.finditer(text):
        start = widget.index("1.0+" + str(match.start()) + "c")
        end = widget.index("1.0+" + str(match.end()) + "c")
        widget.tag_add("comment", start, end)