"""Popup windows: examples, the caveman dictionary, and about."""

import os
import tkinter as tk
from tkinter import ttk

from . import theme
from .boot import cmc


class ExamplesDialog(tk.Toplevel):
    def __init__(self, parent, on_pick):
        super().__init__(parent)
        self.title("Example caves")
        self.configure(bg=theme.PANEL)
        self.geometry("900x560")
        self.transient(parent)
        self.on_pick = on_pick

        paths = cmc.paths.example_files()
        self.paths = paths

        header = tk.Label(
            self, text="Pick an example to see what CMC can do",
            bg=theme.PANEL, fg=theme.ACCENT, font=theme.FONT_UI_BIG, pady=8,
        )
        header.pack(fill="x")

        body = tk.PanedWindow(self, orient="horizontal", bg=theme.PANEL, sashwidth=6)
        body.pack(fill="both", expand=True, padx=10, pady=6)

        left = tk.Frame(body, bg=theme.PANEL)
        listbox = tk.Listbox(
            left, bg=theme.BG, fg=theme.TEXT, selectbackground=theme.PANEL3,
            selectforeground=theme.WHITE, font=theme.FONT_UI, activestyle="none",
            highlightthickness=0, border=0,
        )
        scroll = ttk.Scrollbar(left, command=listbox.yview)
        listbox.configure(yscrollcommand=scroll.set)
        scroll.pack(side="right", fill="y")
        listbox.pack(side="left", fill="both", expand=True)
        left.configure(width=280)
        body.add(left, minsize=240)

        right = tk.Frame(body, bg=theme.BG)
        self.preview = tk.Text(
            right, bg=theme.BG, fg=theme.TEXT, font=theme.FONT_CODE_SMALL,
            wrap="none", border=0, highlightthickness=0, padx=10, pady=8,
        )
        pscroll = ttk.Scrollbar(right, command=self.preview.yview)
        self.preview.configure(yscrollcommand=pscroll.set)
        pscroll.pack(side="right", fill="y")
        self.preview.pack(side="left", fill="both", expand=True)
        body.add(right)

        buttons = tk.Frame(self, bg=theme.PANEL)
        buttons.pack(fill="x", pady=8)

        def button(text, command, color=theme.PANEL2):
            tk.Button(
                buttons, text=text, command=command, bg=color, fg=theme.TEXT,
                activebackground=theme.PANEL3, activeforeground=theme.WHITE,
                relief="flat", font=theme.FONT_UI_BOLD, padx=14, pady=6,
            ).pack(side="left", padx=8)

        button("Open in Text tab", lambda: self._pick("text"), theme.PANEL2)
        button("Open as Blocks", lambda: self._pick("blocks"), theme.ACCENT)
        button("Close", self.destroy, theme.PANEL2)

        for path in paths:
            listbox.insert("end", "  " + os.path.basename(path))
        if paths:
            listbox.selection_set(0)
            self._show(0)
        listbox.bind("<<ListboxSelect>>", lambda e: self._show(listbox.curselection()))
        listbox.bind("<Double-Button-1>", lambda e: self._pick("text"))

        self.listbox = listbox
        self.grab_set()

    def _show(self, selection):
        if not selection:
            return
        path = self.paths[selection[0]]
        with open(path, "r", encoding="utf-8") as handle:
            source = handle.read()
        self.preview.delete("1.0", "end")
        self.preview.insert("1.0", source)

    def _pick(self, mode):
        selection = self.listbox.curselection()
        if not selection:
            return
        path = self.paths[selection[0]]
        with open(path, "r", encoding="utf-8") as handle:
            source = handle.read()
        self.on_pick(path, source, mode)
        self.destroy()


class DictionaryDialog(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.title("Caveman dictionary")
        self.configure(bg=theme.PANEL)
        self.geometry("880x600")
        self.transient(parent)

        tk.Label(
            self, text="Caveman dictionary -- every CMC word",
            bg=theme.PANEL, fg=theme.ACCENT, font=theme.FONT_UI_BIG, pady=8,
        ).pack(fill="x")

        search_row = tk.Frame(self, bg=theme.PANEL)
        search_row.pack(fill="x", padx=10)
        tk.Label(search_row, text="Search:", bg=theme.PANEL, fg=theme.TEXT,
                 font=theme.FONT_UI).pack(side="left")
        self.search = tk.Entry(
            search_row, bg=theme.BG, fg=theme.TEXT, insertbackground=theme.WHITE,
            font=theme.FONT_UI, relief="flat", highlightthickness=1,
            highlightbackground=theme.PANEL3,
        )
        self.search.pack(side="left", fill="x", expand=True, padx=8, pady=6)
        self.search.bind("<KeyRelease>", lambda e: self._fill())

        body = tk.PanedWindow(self, orient="horizontal", bg=theme.PANEL, sashwidth=6)
        body.pack(fill="both", expand=True, padx=10, pady=6)

        left = tk.Frame(body, bg=theme.PANEL)
        self.listbox = tk.Listbox(
            left, bg=theme.BG, fg=theme.TEXT, selectbackground=theme.PANEL3,
            selectforeground=theme.WHITE, font=theme.FONT_UI, activestyle="none",
            highlightthickness=0, border=0, width=26,
        )
        scroll = ttk.Scrollbar(left, command=self.listbox.yview)
        self.listbox.configure(yscrollcommand=scroll.set)
        scroll.pack(side="right", fill="y")
        self.listbox.pack(side="left", fill="both", expand=True)
        body.add(left, minsize=200)

        right = tk.Frame(body, bg=theme.BG)
        self.detail = tk.Text(
            right, bg=theme.BG, fg=theme.TEXT, font=theme.FONT_UI, wrap="word",
            border=0, highlightthickness=0, padx=12, pady=10,
        )
        dscroll = ttk.Scrollbar(right, command=self.detail.yview)
        self.detail.configure(yscrollcommand=dscroll.set)
        dscroll.pack(side="right", fill="y")
        self.detail.pack(side="left", fill="both", expand=True)
        body.add(right)

        self.detail.tag_configure("title", foreground=theme.ACCENT, font=theme.FONT_TITLE)
        self.detail.tag_configure("sub", foreground=theme.DIM, font=theme.FONT_SMALL)
        self.detail.tag_configure("head", foreground=theme.GREEN, font=theme.FONT_UI_BOLD)
        self.detail.tag_configure("code", foreground=theme.YELLOW, font=theme.FONT_CODE_SMALL)
        self.detail.tag_configure("body", foreground=theme.TEXT, font=theme.FONT_UI)

        self.words = sorted(cmc.words.all_words(), key=lambda w: (w.get("category", ""), w["name"]))
        self._fill()
        self.listbox.bind("<<ListboxSelect>>", lambda e: self._show())
        if self.listbox.size():
            self.listbox.selection_set(0)
            self._show()
        self.grab_set()

    def _visible(self):
        needle = self.search.get().strip().lower()
        if not needle:
            return self.words
        found = []
        for word in self.words:
            haystack = " ".join(
                [word["name"], word.get("display", ""), word.get("summary", "")]
                + list(word.get("aliases", []))
            ).lower()
            if needle in haystack:
                found.append(word)
        return found

    def _fill(self):
        self.listbox.delete(0, "end")
        self.visible = self._visible()
        for word in self.visible:
            name = word.get("display", word["name"])
            alias = (" (" + word["aliases"][0] + ")") if word.get("aliases") else ""
            self.listbox.insert("end", "  " + name + alias)
        if self.visible:
            self.listbox.selection_clear(0, "end")
            self.listbox.selection_set(0)
            self._show()

    def _show(self):
        selection = self.listbox.curselection()
        if not selection:
            return
        word = self.visible[selection[0]]
        self.detail.configure(state="normal")
        self.detail.delete("1.0", "end")
        name = word.get("display", word["name"])
        self.detail.insert("end", name, "title")
        if word.get("aliases"):
            self.detail.insert("end", "\naka: " + ", ".join(word["aliases"]), "sub")
        self.detail.insert("end", "\n" + word.get("category", "") + "\n\n", "sub")
        self.detail.insert("end", word.get("summary", "") + "\n\n", "body")
        self.detail.insert("end", "HOW TO WRITE IT\n", "head")
        self.detail.insert("end", "    " + word.get("syntax", "") + "\n", "code")
        for extra in word.get("extra_syntax", []):
            self.detail.insert("end", "    " + extra + "\n", "code")
        self.detail.insert("end", "\n" + self._clean(word.get("doc", "")) + "\n", "body")
        self.detail.insert("end", "\nEXAMPLE\n", "head")
        self.detail.insert("end", word.get("example", "") + "\n", "code")
        if word.get("tips"):
            self.detail.insert("end", "\nTIPS\n", "head")
            for tip in word["tips"]:
                self.detail.insert("end", "  - " + self._clean(tip) + "\n", "body")
        if word.get("related"):
            self.detail.insert("end", "\nSEE ALSO: " + ", ".join(word["related"]), "sub")
        self.detail.configure(state="disabled")

    @staticmethod
    def _clean(text):
        return text.replace("`", "")


class AboutDialog(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.title("About OGABOOGA CODER")
        self.configure(bg=theme.PANEL)
        self.geometry("560x420")
        self.transient(parent)

        text = tk.Text(
            self, bg=theme.BG, fg=theme.TEXT, font=theme.FONT_UI, wrap="word",
            border=0, highlightthickness=0, padx=16, pady=14,
        )
        text.pack(fill="both", expand=True)
        text.tag_configure("title", foreground=theme.ACCENT, font=theme.FONT_TITLE)
        text.tag_configure("sub", foreground=theme.DIM)
        text.tag_configure("code", foreground=theme.YELLOW, font=theme.FONT_CODE_SMALL)

        text.insert("end", "OGABOOGA CODER\n", "title")
        text.insert("end", "the home cave of Cave Man Code (CMC) v" + cmc.VERSION + "\n\n", "sub")
        text.insert(
            "end",
            "The compiler lives right inside this window.  When you press RUN, "
            "the same CMC engine that powers the `cmc` command runs your "
            "program.\n\n"
            "Two ways to code:\n"
            "  - Blocks: drag colorful blocks and watch the code write itself\n"
            "  - Text: type CMC words like oga, grunk, binga, and unga\n\n"
            "Made for brand new coders, but with real power underneath:\n"
            "numbers, text, piles, clumps, recursion, and drawing.\n\n",
        )
        text.insert("end", 'oga "Ooga booga!"\n', "code")
        text.insert("end", "\nMIT License. Go make something silly.\n", "sub")
        text.configure(state="disabled")