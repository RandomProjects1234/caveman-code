"""Drawing for the terminal build of CMC.

OGABOOGA CODER has its own Draw tab; this little window is for when someone
runs a program with `cmc run` outside the IDE.
"""


class NullDrawSurface:
    """A pen that draws nothing (used in tests and quiet modes)."""

    def clear(self):
        pass

    def size(self, w, h):
        pass

    def color(self, c):
        pass

    def dot(self, x, y, r):
        pass

    def circle(self, x, y, r):
        pass

    def line(self, x1, y1, x2, y2):
        pass

    def box(self, x, y, w, h):
        pass

    def blob(self, x, y, w, h):
        pass

    def write(self, text, x, y):
        pass

    def hold_open(self):
        pass


class TkDrawSurface:
    """A white picture window drawn with tkinter."""

    def __init__(self, title="CMC picture"):
        self.title = title
        self.root = None
        self.canvas = None
        self.color = "black"
        self.width = 400
        self.height = 400

    def _ensure(self):
        if self.root is not None:
            return
        import tkinter as tk

        self.root = tk.Tk()
        self.root.title(self.title)
        self.canvas = tk.Canvas(self.root, width=self.width, height=self.height, bg="white")
        self.canvas.pack()

    def clear(self):
        self._ensure()
        self.canvas.delete("all")

    def size(self, w, h):
        self.width = int(w)
        self.height = int(h)
        self._ensure()
        self.canvas.config(width=self.width, height=self.height)

    def color(self, c):
        self.color = c

    def dot(self, x, y, r):
        self._ensure()
        self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=self.color, outline=self.color)

    def circle(self, x, y, r):
        self._ensure()
        self.canvas.create_oval(x - r, y - r, x + r, y + r, outline=self.color, width=2)

    def line(self, x1, y1, x2, y2):
        self._ensure()
        self.canvas.create_line(x1, y1, x2, y2, fill=self.color, width=2)

    def box(self, x, y, w, h):
        self._ensure()
        self.canvas.create_rectangle(x, y, x + w, y + h, outline=self.color, width=2)

    def blob(self, x, y, w, h):
        self._ensure()
        self.canvas.create_rectangle(x, y, x + w, y + h, fill=self.color, outline=self.color)

    def write(self, text, x, y):
        self._ensure()
        self.canvas.create_text(x, y, text=str(text), fill=self.color, anchor="nw")

    def used(self):
        return self.root is not None

    def hold_open(self):
        """Keep the picture up until the person closes it."""
        if self.root is None:
            return
        print("Open picture window! Close it when you are done looking.")
        self.root.mainloop()