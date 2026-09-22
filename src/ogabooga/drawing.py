"""The Draw tab: a white picture the running program can scribble on.

Drawing calls come from the program's worker thread, so they are pushed into
a queue and painted by the main thread.
"""

import tkinter as tk
from tkinter import ttk

from . import theme


class DrawingPanel(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.queue = []
        self.used = False
        self.color = "black"

        bar = tk.Frame(self, bg=theme.PANEL)
        bar.pack(fill="x")
        tk.Label(
            bar,
            text="DRAW",
            bg=theme.PANEL,
            fg=theme.ACCENT,
            font=theme.FONT_UI_BOLD,
            padx=10,
            pady=4,
        ).pack(side="left")
        tk.Button(
            bar,
            text="Wipe picture",
            command=self.wipe,
            bg=theme.PANEL2,
            fg=theme.TEXT,
            activebackground=theme.PANEL3,
            activeforeground=theme.WHITE,
            relief="flat",
            font=theme.FONT_SMALL,
            padx=8,
        ).pack(side="right", padx=6, pady=2)

        holder = tk.Frame(self, bg=theme.PANEL)
        holder.pack(fill="both", expand=True)
        self.canvas = tk.Canvas(
            holder,
            width=500,
            height=400,
            bg="white",
            highlightthickness=2,
            highlightbackground=theme.PANEL3,
        )
        self.canvas.pack(padx=16, pady=16)

        self.after(40, self._pump)

    # --------------------------------------------------- main-thread paint

    def _pump(self):
        while self.queue:
            name, args = self.queue.pop(0)
            try:
                self._apply(name, args)
            except Exception:
                pass
        self.after(40, self._pump)

    def _apply(self, name, args):
        if name == "clear":
            self.canvas.delete("all")
        elif name == "size":
            self.canvas.configure(width=int(args[0]), height=int(args[1]))
        elif name == "color":
            self.color = args[0]
        elif name == "dot":
            x, y, r = args
            self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=self.color, outline=self.color)
        elif name == "circle":
            x, y, r = args
            self.canvas.create_oval(x - r, y - r, x + r, y + r, outline=self.color, width=2)
        elif name == "line":
            x1, y1, x2, y2 = args
            self.canvas.create_line(x1, y1, x2, y2, fill=self.color, width=2)
        elif name == "box":
            x, y, w, h = args
            self.canvas.create_rectangle(x, y, x + w, y + h, outline=self.color, width=2)
        elif name == "blob":
            x, y, w, h = args
            self.canvas.create_rectangle(x, y, x + w, y + h, fill=self.color, outline=self.color)
        elif name == "write":
            text, x, y = args
            self.canvas.create_text(x, y, text=str(text), fill=self.color, anchor="nw")

    # ----------------------------------------------- worker-thread surface

    def _enqueue(self, name, *args):
        self.used = True
        self.queue.append((name, args))

    def clear(self):
        self._enqueue("clear")

    def size(self, w, h):
        self._enqueue("size", w, h)

    def color(self, c):
        self._enqueue("color", c)

    def dot(self, x, y, r):
        self._enqueue("dot", x, y, r)

    def circle(self, x, y, r):
        self._enqueue("circle", x, y, r)

    def line(self, x1, y1, x2, y2):
        self._enqueue("line", x1, y1, x2, y2)

    def box(self, x, y, w, h):
        self._enqueue("box", x, y, w, h)

    def blob(self, x, y, w, h):
        self._enqueue("blob", x, y, w, h)

    def write(self, text, x, y):
        self._enqueue("write", text, x, y)

    def hold_open(self):
        pass  # the Draw tab never closes

    # --------------------------------------------------------------- extra

    def wipe(self):
        self.canvas.delete("all")
        self.color = "black"