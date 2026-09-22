"""A code editor with line numbers, tabs, and CMC colors."""

import tkinter as tk
from tkinter import ttk

from . import highlight
from . import theme

OPENERS = ("binga", "booga", "zug", "zoop", "clump", "if", "repeat", "while", "for", "function", "fn", "def")


class CodeEditor(ttk.Frame):
    def __init__(self, parent, on_change=None):
        super().__init__(parent)
        self.on_change = on_change

        self.gutter = tk.Text(
            self,
            width=4,
            padx=6,
            takefocus=0,
            border=0,
            highlightthickness=0,
            bg=theme.PANEL,
            fg=theme.DIM,
            font=theme.FONT_CODE,
            state="disabled",
        )
        self.gutter.pack(side="left", fill="y")

        self.text = tk.Text(
            self,
            wrap="none",
            undo=True,
            border=0,
            highlightthickness=1,
            highlightbackground=theme.PANEL3,
            highlightcolor=theme.ACCENT,
            insertbackground=theme.ACCENT,
            selectbackground=theme.PANEL3,
            bg=theme.BG,
            fg=theme.TEXT,
            font=theme.FONT_CODE,
            padx=8,
            pady=6,
        )
        self.text.pack(side="left", fill="both", expand=True)

        scroll = ttk.Scrollbar(self, command=self.text.yview)
        scroll.pack(side="right", fill="y")
        self.text.configure(yscrollcommand=self._on_scroll)

        highlight.configure(self.text)

        self.text.bind("<<Modified>>", self._on_modified)
        self.text.bind("<Tab>", self._on_tab)
        self.text.bind("<Return>", self._on_return)
        self.text.bind("<KeyRelease>", self._on_key)
        self.text.bind("<ButtonRelease-1>", self._on_key)

    # ------------------------------------------------------------ events

    def _on_scroll(self, first, last):
        self.gutter.yview_moveto(first)
        for child in self.winfo_children():
            if isinstance(child, ttk.Scrollbar):
                child.set(first, last)

    def _on_modified(self, _event=None):
        self.text.edit_modified(False)
        self._refresh()
        if self.on_change is not None:
            self.on_change()

    def _on_key(self, _event=None):
        self._refresh()

    def _on_tab(self, _event=None):
        self.text.insert("insert", "    ")
        return "break"

    def _on_return(self, _event=None):
        line = self.text.get("insert linestart", "insert")
        indent = line[: len(line) - len(line.lstrip(" "))]
        word = line.strip().split(" ")[0].lower() if line.strip() else ""
        if word in OPENERS:
            indent += "    "
        self.text.insert("insert", "\n" + indent)
        return "break"

    # ------------------------------------------------------------ drawing

    def _refresh(self):
        self._update_gutter()
        highlight.highlight(self.text)

    def _update_gutter(self):
        last_line = int(self.text.index("end-1c").split(".")[0])
        width = max(3, len(str(last_line)))
        if int(self.gutter.cget("width")) != width:
            self.gutter.configure(width=width)
        numbers = "\n".join(str(n).rjust(width) for n in range(1, last_line + 1))
        self.gutter.configure(state="normal")
        self.gutter.delete("1.0", "end")
        self.gutter.insert("1.0", numbers)
        self.gutter.configure(state="disabled")
        self.gutter.yview_moveto(self.text.yview()[0])

    # ------------------------------------------------------------- api

    def get_text(self):
        return self.text.get("1.0", "end-1c")

    def set_text(self, source):
        self.text.delete("1.0", "end")
        if source:
            self.text.insert("1.0", source)
        self.text.edit_reset()
        self.text.edit_modified(True)
        self._refresh()

    def insert_text(self, source):
        self.text.insert("insert", source)
        self.text.focus_set()
        self._refresh()

    def clear(self):
        self.set_text("")

    def position_text(self):
        line, col = self.text.index("insert").split(".")
        return "Line " + line + ", Col " + str(int(col) + 1)

    def goto_line(self, number):
        self.text.mark_set("insert", str(number) + ".0")
        self.text.see("insert")
        self.text.focus_set()

    def focus_editor(self):
        self.text.focus_set()