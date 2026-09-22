"""The output panel where programs talk back."""

import tkinter as tk
from tkinter import ttk

from . import theme


class OutputPane(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)

        header = tk.Frame(self, bg=theme.PANEL)
        header.pack(fill="x")
        tk.Label(
            header,
            text="OUTPUT",
            bg=theme.PANEL,
            fg=theme.ACCENT,
            font=theme.FONT_UI_BOLD,
            padx=10,
            pady=4,
        ).pack(side="left")
        self.status_label = tk.Label(
            header,
            text="ready",
            bg=theme.PANEL,
            fg=theme.DIM,
            font=theme.FONT_SMALL,
            padx=10,
        )
        self.status_label.pack(side="right")
        tk.Button(
            header,
            text="Clear",
            command=self.clear,
            bg=theme.PANEL2,
            fg=theme.TEXT,
            activebackground=theme.PANEL3,
            activeforeground=theme.WHITE,
            relief="flat",
            font=theme.FONT_SMALL,
            padx=8,
        ).pack(side="right", padx=6, pady=2)

        body = tk.Frame(self, bg=theme.BG)
        body.pack(fill="both", expand=True)
        self.text = tk.Text(
            body,
            height=9,
            wrap="word",
            border=0,
            highlightthickness=0,
            bg=theme.BG,
            fg=theme.TEXT,
            font=theme.FONT_CODE_SMALL,
            padx=10,
            pady=6,
            state="disabled",
        )
        self.text.pack(side="left", fill="both", expand=True)
        scroll = ttk.Scrollbar(body, command=self.text.yview)
        scroll.pack(side="right", fill="y")
        self.text.configure(yscrollcommand=scroll.set)

        self.text.tag_configure("out", foreground=theme.TEXT)
        self.text.tag_configure("err", foreground=theme.RED)
        self.text.tag_configure("info", foreground=theme.DIM)
        self.text.tag_configure("ok", foreground=theme.GREEN)
        self.text.tag_configure("ooga", foreground=theme.RED, font=theme.FONT_CODE_SMALL)

    def write(self, message, tag="out"):
        self.text.configure(state="normal")
        if self.text.index("end-1c") != "1.0":
            self.text.insert("end", "\n")
        self.text.insert("end", message, tag)
        self.text.see("end")
        self.text.configure(state="disabled")

    def status(self, message, color=None):
        self.status_label.configure(text=message, fg=color or theme.DIM)

    def clear(self):
        self.text.configure(state="normal")
        self.text.delete("1.0", "end")
        self.text.configure(state="disabled")
        self.status("ready")