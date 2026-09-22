"""The drag-and-drop block workshop for OGABOOGA CODER.

Left side: a palette of blocks.  Middle: a canvas where blocks stack and
nest.  Blocks live in plain lists, so the same shape can always be turned
back into CMC text (and text can be turned back into blocks).
"""

import re
import tkinter as tk
from tkinter import ttk

from . import blocks as B
from . import theme
from .boot import cmc

MIN_W = 200
GAP = 10
HEADER = 32
FIELD_H = 24
INDENT = 24
BODY_PAD = 10
BOTTOM_PAD = 10
ELSE_BAR = 24
DRAG_SLOP = 6
NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def _inside(rect, x, y):
    rx, ry, rw, rh = rect
    return rx <= x <= rx + rw and ry <= y <= ry + rh


class BlockCanvas(ttk.Frame):
    def __init__(self, parent, on_change=None, on_status=None):
        super().__init__(parent)
        self.on_change = on_change
        self.on_status = on_status

        self.blocks = []
        self.history = []
        self.layout = []
        self.entries_by_uid = {}
        self.selected = None
        self.dragging = None
        self.ghost = None
        self.preview = None
        self._field_editor = None
        self._press_pos = None

        self._build_palette()
        self._build_canvas()
        self.redraw()

    # ------------------------------------------------------------ build

    def _build_palette(self):
        holder = tk.Frame(self, bg=theme.PANEL, width=220)
        holder.pack(side="left", fill="y")
        holder.pack_propagate(False)

        tk.Label(
            holder,
            text="BLOCKS",
            bg=theme.PANEL,
            fg=theme.ACCENT,
            font=theme.FONT_UI_BOLD,
            pady=6,
        ).pack(fill="x")

        canvas = tk.Canvas(holder, bg=theme.PANEL, highlightthickness=0, width=214)
        scroll = ttk.Scrollbar(holder, command=canvas.yview)
        canvas.configure(yscrollcommand=scroll.set)
        scroll.pack(side="right", fill="y")
        canvas.pack(side="left", fill="both", expand=True)

        inner = tk.Frame(canvas, bg=theme.PANEL)
        canvas.create_window((0, 0), window=inner, anchor="nw")

        def on_inner(_event=None):
            canvas.configure(scrollregion=canvas.bbox("all"))

        inner.bind("<Configure>", on_inner)

        grouped = {}
        for kind, spec in B.SPECS.items():
            grouped.setdefault(spec["category"], []).append(kind)

        for category in B.CATEGORY_ORDER:
            kinds = grouped.get(category, [])
            if not kinds:
                continue
            tk.Label(
                inner,
                text=category,
                bg=theme.PANEL,
                fg=theme.CATEGORY_COLORS.get(category, theme.DIM),
                font=theme.FONT_UI_BOLD,
                anchor="w",
                padx=8,
                pady=4,
            ).pack(fill="x")
            for kind in kinds:
                spec = B.SPECS[kind]
                label = tk.Label(
                    inner,
                    text="  " + spec["title"],
                    bg=theme.block_color(kind),
                    fg="#241a12",
                    font=theme.FONT_UI_BOLD,
                    anchor="w",
                    padx=6,
                    pady=4,
                )
                label.pack(fill="x", padx=8, pady=2)
                label.bind("<ButtonPress-1>", lambda e, k=kind: self._palette_press(e, k))
                label.bind("<B1-Motion>", self._palette_motion)
                label.bind("<ButtonRelease-1>", self._palette_release)

    def _build_canvas(self):
        holder = tk.Frame(self, bg=theme.BG)
        holder.pack(side="left", fill="both", expand=True)

        self.canvas = tk.Canvas(holder, bg=theme.BG, highlightthickness=0)
        hscroll = ttk.Scrollbar(holder, orient="horizontal", command=self.canvas.xview)
        vscroll = ttk.Scrollbar(holder, orient="vertical", command=self.canvas.yview)
        self.canvas.configure(xscrollcommand=hscroll.set, yscrollcommand=vscroll.set)
        hscroll.pack(side="bottom", fill="x")
        vscroll.pack(side="right", fill="y")
        self.canvas.pack(side="left", fill="both", expand=True)

        self.canvas.bind("<ButtonPress-1>", self._canvas_press)
        self.canvas.bind("<B1-Motion>", self._canvas_motion)
        self.canvas.bind("<ButtonRelease-1>", self._canvas_release)
        self.canvas.bind("<Button-3>", self._context_menu)
        self.canvas.bind("<Delete>", lambda e: self.delete_selected())
        self.canvas.bind("<Control-z>", lambda e: self.undo())
        self.canvas.configure(takefocus=True)

    # --------------------------------------------------------- public api

    def get_blocks(self):
        return self.blocks

    def set_blocks(self, new_blocks, remember=True):
        if remember:
            self._snapshot()
        self.blocks = new_blocks
        self.selected = None
        self.redraw()
        self._notify()

    def source(self):
        return B.blocks_to_source(self.blocks)

    def clear_all(self):
        if not self.blocks:
            return
        self._snapshot()
        self.blocks = []
        self.selected = None
        self.redraw()
        self._notify()

    def undo(self):
        if not self.history:
            self._status("Nothing to undo.")
            return
        self.blocks = self.history.pop()
        self.selected = None
        self.redraw()
        self._notify()
        self._status("Undone!")

    def delete_selected(self):
        if self.selected is None:
            return
        self._snapshot()
        location = self._find_location(self.selected)
        if location is None:
            return
        parent, which, index = location
        target = self._target_list(parent, which)
        del target[index]
        self.selected = None
        self.redraw()
        self._notify()

    def add_block(self, kind):
        block = B.new_block(kind)
        self._snapshot()
        self.blocks.append(block)
        self.selected = block
        self.redraw()
        self._notify()
        self._status("Added " + block.title() + " block.")

    # ------------------------------------------------------------ helpers

    def _snapshot(self):
        self.history.append(B.clone_all(self.blocks))
        if len(self.history) > 80:
            self.history.pop(0)

    def _notify(self):
        if self.on_change is not None:
            self.on_change()

    def _status(self, message):
        if self.on_status is not None:
            self.on_status(message)

    def _target_list(self, parent, which):
        if parent is None:
            return self.blocks
        return parent.body if which == "body" else parent.else_body

    def _find_location(self, block, blocks=None, parent=None, which="body"):
        if blocks is None:
            blocks = self.blocks
        for index, candidate in enumerate(blocks):
            if candidate is block:
                return parent, which, index
            found = self._find_location(block, candidate.body, candidate, "body")
            if found:
                return found
            found = self._find_location(block, candidate.else_body, candidate, "else")
            if found:
                return found
        return None

    # ------------------------------------------------------------- layout

    def _layout_blocks(self, blocks, x, y):
        entries = []
        for block in blocks:
            entries.append(self._layout_block(block, x, y))
            y = entries[-1]["outer"][1] + entries[-1]["outer"][3] + GAP
        return entries

    def _layout_block(self, block, x, y):
        spec = B.SPECS[block.kind]
        entry = {
            "block": block,
            "fields": {},
            "body_children": [],
            "else_children": [],
            "body_rect": None,
            "else_rect": None,
        }
        self.entries_by_uid[id(block)] = entry
        self.layout.append(entry)  # parents come before children on purpose

        title = block.title()
        title_w = max(52, len(title) * 7 + 18)
        cursor = x + 8 + title_w + 4
        for key, label, _ftype, fw in spec["fields"]:
            if label:
                cursor += len(label) * 7 + 4
            entry["fields"][key] = (cursor, y + (HEADER - FIELD_H) // 2, fw, FIELD_H)
            cursor += fw + 8
        header_right = cursor + 6
        width = max(MIN_W, header_right - x)

        bottom = y + HEADER
        container = spec.get("container")

        if container:
            body_x = x + INDENT
            inner_y = y + HEADER + BODY_PAD
            child_entries = self._layout_blocks(block.body, body_x, inner_y)
            entry["body_children"] = [(child["block"], child) for child in child_entries]
            bottom = inner_y
            for child in child_entries:
                bottom = max(bottom, child["outer"][1] + child["outer"][3])
            if child_entries:
                child_right = max(c["outer"][0] + c["outer"][2] for c in child_entries)
                body_w = max(140, child_right - (body_x - 8))
            else:
                body_w = 160
            entry["body_rect"] = (body_x - 8, y + HEADER, body_w, max(28, bottom - (y + HEADER) + 6))

            if block.kind == "if":
                else_y = bottom + ELSE_BAR + 6
                else_entries = self._layout_blocks(block.else_body, body_x, else_y)
                entry["else_children"] = [(child["block"], child) for child in else_entries]
                else_bottom = else_y
                for child in else_entries:
                    else_bottom = max(else_bottom, child["outer"][1] + child["outer"][3])
                if else_entries:
                    child_right = max(c["outer"][0] + c["outer"][2] for c in else_entries)
                    else_w = max(140, child_right - (body_x - 8))
                else:
                    else_w = 160
                entry["else_rect"] = (
                    body_x - 8,
                    bottom + 2,
                    else_w,
                    max(24, else_bottom - (bottom + 2)),
                )
                bottom = else_bottom
                width = max(width, body_w + INDENT + 24, else_w + INDENT + 24)
            else:
                width = max(width, body_w + INDENT + 24)

        height = bottom + BOTTOM_PAD - y
        entry["outer"] = (x, y, width, height)
        return entry

    def _entry_for(self, block):
        return self.entries_by_uid.get(id(block))

    def _rebuild_layout(self):
        self.layout = []
        self.entries_by_uid = {}
        self._layout_blocks(self.blocks, 18, 18)

    # ------------------------------------------------------------ drawing

    def redraw(self):
        self.canvas.delete("all")
        self._field_editor = None
        self._rebuild_layout()

        if not self.blocks:
            self.canvas.create_text(
                40,
                40,
                anchor="nw",
                text=(
                    "This is your block workshop!\n\n"
                    "1. Click a block on the left to add it here.\n"
                    "2. Or drag it where you want it.\n"
                    "3. Drag boxes / numbers / text into the little slots.\n"
                    "4. Press RUN to run your blocks.\n\n"
                    "Right-click a block to delete or copy it."
                ),
                fill=theme.DIM,
                font=theme.FONT_UI,
            )

        for entry in self.layout:
            self._draw_container(entry)
        for entry in self.layout:
            self._draw_header(entry)
            self._draw_fields(entry)

        if self.selected is not None:
            entry = self._entry_for(self.selected)
            if entry:
                x, y, w, h = entry["outer"]
                self.canvas.create_rectangle(
                    x - 3, y - 3, x + w + 3, y + h + 3,
                    outline=theme.WHITE, width=2, dash=(4, 3),
                )

        if self.dragging and self.dragging.get("pointer"):
            self._draw_ghost(*self.dragging["pointer"])

        self._update_scroll()

    def _draw_container(self, entry):
        block = entry["block"]
        if not B.SPECS[block.kind].get("container"):
            return
        x, y, w, h = entry["outer"]
        self._round_rect(x - 2, y - 2, x + w + 6, y + h + 4, 12,
                         fill="#2b2016", outline=theme.PANEL3, width=2)
        if entry["body_rect"]:
            bx, by, bw, bh = entry["body_rect"]
            if not entry["body_children"]:
                self.canvas.create_text(
                    bx + 12, by + 14, anchor="w",
                    text="drop blocks here", fill="#6b5947", font=theme.FONT_SMALL,
                )
        if block.kind == "if":
            if entry["else_rect"]:
                ex, ey, ew, eh = entry["else_rect"]
                self.canvas.create_text(
                    x + 14, ey + 2, anchor="nw", text="wonga", fill=theme.PURPLE,
                    font=(theme.FONT_CODE[0], 10, "bold"),
                )
                if not entry["else_children"]:
                    self.canvas.create_text(
                        ex + 12, ey + 12, anchor="w",
                        text="drop blocks here for 'else'", fill="#6b5947", font=theme.FONT_SMALL,
                    )

    def _draw_header(self, entry):
        block = entry["block"]
        x, y, w, _h = entry["outer"]
        color = block.color()
        title = block.title()
        self._round_rect(x, y, x + w, y + HEADER, 9, fill=color, outline="#1b120c", width=1)
        self.canvas.create_text(
            x + 12, y + HEADER // 2 + 1, anchor="w", text=title,
            fill="#241a12", font=(theme.FONT_UI_BOLD[0], 10, "bold"),
        )

    def _draw_fields(self, entry):
        block = entry["block"]
        spec = B.SPECS[block.kind]
        color = block.color()
        for key, label, _ftype, _w in spec["fields"]:
            fx, fy, fw, fh = entry["fields"][key]
            if label:
                self.canvas.create_text(
                    fx - 5, fy + fh // 2, anchor="e", text=label,
                    fill="#241a12", font=theme.FONT_SMALL,
                )
            self.canvas.create_rectangle(
                fx, fy, fx + fw, fy + fh, fill="#241a12", outline=color, width=1,
            )
            text = self._clip(block.field(key), fw)
            self.canvas.create_text(
                fx + 5, fy + fh // 2 + 1, anchor="w", text=text,
                fill=theme.TEXT, font=theme.FONT_CODE_SMALL,
            )

    @staticmethod
    def _clip(text, width):
        limit = max(3, int((width - 10) / 6.4))
        text = text.replace("\n", " ")
        if len(text) > limit:
            return text[: limit - 1] + "..."
        return text if text else "..."

    def _round_rect(self, x1, y1, x2, y2, r, **kwargs):
        points = [
            x1 + r, y1, x2 - r, y1, x2, y1, x2, y1 + r, x2, y2 - r, x2, y2,
            x2 - r, y2, x1 + r, y2, x1, y2, x1, y2 - r, x1, y1 + r, x1, y1,
        ]
        return self.canvas.create_polygon(points, smooth=True, **kwargs)

    def _draw_ghost(self, x, y):
        if self.ghost:
            self.canvas.delete(self.ghost)
        self.ghost = self.canvas.create_rectangle(
            x - 60, y - 14, x + 60, y + 14,
            outline=theme.WHITE, dash=(5, 3), width=2,
        )

    def _draw_preview(self, target, x, y):
        if self.preview:
            self.canvas.delete(self.preview)
            self.preview = None
        if target is None:
            return
        if target[0] == "field":
            _kind, block, key = target
            entry = self._entry_for(block)
            if entry:
                fx, fy, fw, fh = entry["fields"][key]
                self.preview = self.canvas.create_rectangle(
                    fx - 2, fy - 2, fx + fw + 2, fy + fh + 2,
                    outline=theme.WHITE, width=2,
                )
            return
        if target[0] == "trash":
            return
        _kind, parent, which, index = target
        if parent is None:
            if not self.blocks:
                y_line = 24
            elif index >= len(self.blocks):
                entry = self._entry_for(self.blocks[-1])
                _, py, _pw, ph = entry["outer"]
                y_line = py + ph + GAP // 2
            else:
                entry = self._entry_for(self.blocks[index])
                y_line = entry["outer"][1] - GAP // 2
            x1, x2 = 20, 420
        else:
            entry = self._entry_for(parent)
            children = entry["body_children"] if which == "body" else entry["else_children"]
            if not children:
                if which == "body":
                    bx, by, bw, _bh = entry["body_rect"]
                else:
                    bx, by, bw, _bh = entry["else_rect"]
                y_line = by + 14
                x1, x2 = bx + 6, bx + bw + 6
            elif index >= len(children):
                child = children[-1][1]
                y_line = child["outer"][1] + child["outer"][3] + GAP // 2
                x1 = child["outer"][0]
                x2 = x1 + child["outer"][2]
            else:
                child = children[index][1]
                y_line = child["outer"][1] - GAP // 2
                x1 = child["outer"][0]
                x2 = x1 + child["outer"][2]
        self.preview = self.canvas.create_line(x1, y_line, x2, y_line, fill=theme.WHITE, width=3)

    def _update_scroll(self):
        max_x = 500
        max_y = 400
        for entry in self.layout:
            x, y, w, h = entry["outer"]
            max_x = max(max_x, x + w + 60)
            max_y = max(max_y, y + h + 60)
        self.canvas.configure(scrollregion=(0, 0, max_x, max_y))

    # ------------------------------------------------------------ events

    def _canvas_point(self, event):
        x = self.canvas.canvasx(event.x_root - self.canvas.winfo_rootx())
        y = self.canvas.canvasy(event.y_root - self.canvas.winfo_rooty())
        return x, y

    def _drop_target(self, x, y):
        for entry in reversed(self.layout):
            for key, rect in entry["fields"].items():
                if _inside(rect, x, y):
                    return ("field", entry["block"], key)

        for entry in reversed(self.layout):
            if entry["else_rect"] and _inside(entry["else_rect"], x, y):
                index = len(entry["else_children"])
                for i, (_child, child_entry) in enumerate(entry["else_children"]):
                    rect = child_entry["outer"]
                    if y < rect[1] + rect[3] / 2:
                        index = i
                        break
                return ("insert", entry["block"], "else", index)
            if entry["body_rect"] and _inside(entry["body_rect"], x, y):
                index = len(entry["body_children"])
                for i, (_child, child_entry) in enumerate(entry["body_children"]):
                    rect = child_entry["outer"]
                    if y < rect[1] + rect[3] / 2:
                        index = i
                        break
                return ("insert", entry["block"], "body", index)

        index = len(self.blocks)
        for i, block in enumerate(self.blocks):
            entry = self._entry_for(block)
            rect = entry["outer"]
            if y < rect[1] + rect[3] / 2:
                index = i
                break
        return ("insert", None, "body", index)

    def _canvas_press(self, event):
        self.canvas.focus_set()
        self._commit_field_editor()
        x, y = self._canvas_point(event)
        self._press_pos = (x, y)

        for entry in reversed(self.layout):
            if _inside(entry["outer"], x, y):
                for key, rect in entry["fields"].items():
                    if _inside(rect, x, y):
                        self._edit_field(entry["block"], key, rect)
                        return
                self.selected = entry["block"]
                self.dragging = {"block": entry["block"], "moved": False, "pointer": (x, y)}
                self.redraw()
                return

        self.selected = None
        self.redraw()

    def _canvas_motion(self, event):
        x, y = self._canvas_point(event)
        if self.dragging is None:
            return
        if not self.dragging["moved"] and self._press_pos:
            dx = abs(x - self._press_pos[0])
            dy = abs(y - self._press_pos[1])
            if dx < DRAG_SLOP and dy < DRAG_SLOP:
                return
            self.dragging["moved"] = True
        self.dragging["pointer"] = (x, y)
        target = self._drop_target(x, y)
        if target[0] == "field" and not target[1].is_value():
            target = None
        self._draw_preview(target if (target and target[0] != "field") else None, x, y)
        if target and target[0] == "field":
            self._draw_preview(target, x, y)
        self._draw_ghost(x, y)

    def _canvas_release(self, event):
        if self.dragging is None:
            return
        drag = self.dragging
        self.dragging = None
        if self.preview:
            self.canvas.delete(self.preview)
            self.preview = None
        x, y = self._canvas_point(event)

        if not drag["moved"]:
            self.redraw()
            return

        # dropped on the palette or far off the canvas -> remove / cancel
        pointer_widget = self.canvas.winfo_containing(event.x_root, event.y_root)
        if pointer_widget is not self.canvas and pointer_widget is not None:
            self._snapshot()
            location = self._find_location(drag["block"])
            if location:
                parent, which, index = location
                del self._target_list(parent, which)[index]
            self.selected = None
            self.redraw()
            self._notify()
            self._status("Deleted block.")
            return

        target = self._drop_target(x, y)
        self._snapshot()
        location = self._find_location(drag["block"])
        if location:
            parent, which, index = location
            del self._target_list(parent, which)[index]
        if target[0] == "field" and drag["block"].is_value():
            _kind, block, key = target
            block.fields[key] = B.value_expression(drag["block"])
            self.selected = block
        else:
            _kind, parent, which, index = target
            self._target_list(parent, which).insert(index, drag["block"])
            self.selected = drag["block"]
        self.redraw()
        self._notify()

    def _palette_press(self, event, kind):
        block = B.new_block(kind)
        self.dragging = {"block": block, "from_palette": True, "moved": False,
                         "pointer": (0, 0), "kind": kind}
        self._press_pos = (event.x_root, event.y_root)

    def _palette_motion(self, event):
        if self.dragging is None:
            return
        dx = abs(event.x_root - self._press_pos[0])
        dy = abs(event.y_root - self._press_pos[1])
        if not self.dragging["moved"] and dx < DRAG_SLOP and dy < DRAG_SLOP:
            return
        self.dragging["moved"] = True
        x = self.canvas.canvasx(event.x_root - self.canvas.winfo_rootx())
        y = self.canvas.canvasy(event.y_root - self.canvas.winfo_rooty())
        self.dragging["pointer"] = (x, y)
        if event.x_root >= self.canvas.winfo_rootx():
            self._draw_ghost(x, y)
            target = self._drop_target(x, y)
            if target[0] == "field" and not self.dragging["block"].is_value():
                target = None
            self._draw_preview(target, x, y)

    def _palette_release(self, event):
        if self.dragging is None:
            return
        drag = self.dragging
        self.dragging = None
        if self.preview:
            self.canvas.delete(self.preview)
            self.preview = None

        if not drag["moved"]:
            self.add_block(drag["kind"])
            return

        x = self.canvas.canvasx(event.x_root - self.canvas.winfo_rootx())
        y = self.canvas.canvasy(event.y_root - self.canvas.winfo_rooty())
        pointer_widget = self.canvas.winfo_containing(event.x_root, event.y_root)
        if pointer_widget is not self.canvas:
            self.redraw()
            self._status("Drag cancelled.")
            return

        target = self._drop_target(x, y)
        if target[0] == "field" and drag["block"].is_value():
            _kind, block, key = target
            self._snapshot()
            block.fields[key] = B.value_expression(drag["block"])
            self.selected = block
        elif target[0] == "field":
            self.redraw()
            self._status("Only values (number, text, box, random) fit in a slot.")
            return
        else:
            _kind, parent, which, index = target
            self._snapshot()
            self._target_list(parent, which).insert(index, drag["block"])
            self.selected = drag["block"]
        self.redraw()
        self._notify()

    def _context_menu(self, event):
        self._commit_field_editor()
        x, y = self._canvas_point(event)
        hit = None
        for entry in reversed(self.layout):
            if _inside(entry["outer"], x, y):
                hit = entry["block"]
                break
        if hit is None:
            return
        self.selected = hit
        self.redraw()

        menu = tk.Menu(self, tearoff=0, bg=theme.PANEL2, fg=theme.TEXT,
                       activebackground=theme.PANEL3, activeforeground=theme.WHITE)
        menu.add_command(label="Delete", command=self.delete_selected)
        menu.add_command(label="Copy", command=lambda: self._duplicate(hit))
        menu.add_command(label="Move up", command=lambda: self._move(hit, -1))
        menu.add_command(label="Move down", command=lambda: self._move(hit, 1))
        try:
            menu.tk_popup(event.x_root, event.y_root)
        finally:
            menu.grab_release()

    def _duplicate(self, block):
        location = self._find_location(block)
        if location is None:
            return
        parent, which, index = location
        self._snapshot()
        self._target_list(parent, which).insert(index + 1, B.clone(block))
        self.redraw()
        self._notify()

    def _move(self, block, direction):
        location = self._find_location(block)
        if location is None:
            return
        parent, which, index = location
        target = self._target_list(parent, which)
        new_index = index + direction
        if new_index < 0 or new_index >= len(target):
            return
        self._snapshot()
        target[index], target[new_index] = target[new_index], target[index]
        self.redraw()
        self._notify()

    # ------------------------------------------------------ field editing

    def _edit_field(self, block, key, rect):
        self._commit_field_editor()
        x, y, w, h = rect
        editor = tk.Entry(
            self.canvas,
            bg="#120d08",
            fg=theme.TEXT,
            insertbackground=theme.WHITE,
            font=theme.FONT_CODE_SMALL,
            relief="flat",
            highlightthickness=1,
            highlightbackground=theme.WHITE,
        )
        editor.insert(0, block.field(key))
        editor.select_range(0, "end")
        editor.place(x=x, y=y, width=w, height=h)
        editor.focus_set()
        self._field_editor = (editor, block, key)

        def commit(_event=None):
            self._commit_field_editor()

        def cancel(_event=None):
            self._cancel_field_editor()

        editor.bind("<Return>", commit)
        editor.bind("<FocusOut>", commit)
        editor.bind("<Escape>", cancel)

    def _commit_field_editor(self):
        if self._field_editor is None:
            return
        editor, block, key = self._field_editor
        self._field_editor = None
        text = editor.get()
        editor.destroy()

        if self._valid_field(block, key, text):
            if block.field(key) != text:
                self._snapshot()
                block.fields[key] = text
                self.redraw()
                self._notify()
            else:
                self.redraw()
        else:
            self._status("That does not fit in the slot. Try again!")
            self.redraw()

    def _cancel_field_editor(self):
        if self._field_editor is None:
            return
        editor, _block, _key = self._field_editor
        self._field_editor = None
        editor.destroy()
        self.redraw()

    def _valid_field(self, block, key, text):
        spec = B.SPECS[block.kind]
        ftype = None
        for field_key, _label, field_type, _w in spec["fields"]:
            if field_key == key:
                ftype = field_type
                break
        text = text.strip()
        if ftype == "name":
            return bool(NAME_RE.match(text)) and text.lower() not in B.FORBIDDEN
        if ftype == "params":
            if not text:
                return True
            return all(NAME_RE.match(p.strip()) for p in text.split(",") if p.strip())
        if ftype == "expr":
            if not text and ((block.kind == "ork" and key == "expr") or (block.kind == "ask" and key == "prompt")):
                return True
            try:
                cmc.parse_expression_source(text)
                return True
            except cmc.CmcError:
                return False
        if ftype == "args":
            if not text:
                return True
            try:
                cmc.parse_source("f(" + text + ")")
                return True
            except cmc.CmcError:
                return False
        if ftype == "number":
            try:
                float(text)
                return True
            except ValueError:
                return False
        if ftype == "text":
            return True
        return True