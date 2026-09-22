"""OGABOOGA CODER -- the friendly home cave for Cave Man Code."""

import os
import sys
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog, ttk

from . import blocks as B
from . import theme
from .blockcanvas import BlockCanvas
from .boot import cmc
from .dialogs import AboutDialog, DictionaryDialog, ExamplesDialog
from .drawing import DrawingPanel
from .editor import CodeEditor
from .output import OutputPane
from .runner import RunManager

VERSION = cmc.VERSION

TEXT_TAB = "Text (caveman code)"
BLOCKS_TAB = "Blocks (drag and drop)"
DRAW_TAB = "Draw (picture)"


class OgaboogaApp(tk.Tk):
    def __init__(self, selftest=False):
        super().__init__()
        self.selftest = selftest
        self.title("OGABOOGA CODER  -  Cave Man Code v" + VERSION)
        self.configure(bg=theme.BG)
        self.geometry("1260x850")
        self.minsize(1000, 660)

        self.current_path = None
        self._out_queue = []
        self._done_queue = []
        self._input_open = False

        self._build_menu()
        self._build_toolbar()
        self._build_body()
        self._build_status()

        self.runner = RunManager(self._program_output, None, self._run_done)

        self.bind("<F5>", lambda e: self.run_program())
        self.bind("<Control-n>", lambda e: self.new_file())
        self.bind("<Control-o>", lambda e: self.open_file())
        self.bind("<Control-s>", lambda e: self.save_file())
        self.bind("<Escape>", lambda e: self.stop_program())
        self.protocol("WM_DELETE_WINDOW", self._on_close)

        self._pump_id = self.after(40, self._pump)

    # ------------------------------------------------------------- build

    def _build_menu(self):
        menubar = tk.Menu(self)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New", accelerator="Ctrl+N", command=self.new_file)
        file_menu.add_command(label="Open...", accelerator="Ctrl+O", command=self.open_file)
        file_menu.add_command(label="Save", accelerator="Ctrl+S", command=self.save_file)
        file_menu.add_command(label="Save as...", command=self.save_file_as)
        file_menu.add_separator()
        file_menu.add_command(label="Export as Python (.py)...", command=self.export_python)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self._on_close)
        menubar.add_cascade(label="File", menu=file_menu)

        run_menu = tk.Menu(menubar, tearoff=0)
        run_menu.add_command(label="Run", accelerator="F5", command=self.run_program)
        run_menu.add_command(label="Stop", accelerator="Esc", command=self.stop_program)
        menubar.add_cascade(label="Run", menu=run_menu)

        help_menu = tk.Menu(menubar, tearoff=0)
        help_menu.add_command(label="Example caves...", command=self.open_examples)
        help_menu.add_command(label="Caveman dictionary...", command=self.open_dictionary)
        help_menu.add_separator()
        help_menu.add_command(label="Cheat sheet", command=self.open_cheatsheet)
        help_menu.add_command(label="Open the wiki", command=self.open_wiki)
        help_menu.add_separator()
        help_menu.add_command(label="About OGABOOGA CODER", command=lambda: AboutDialog(self))
        menubar.add_cascade(label="Help", menu=help_menu)

        self.configure(menu=menubar)

    def _tool_button(self, parent, text, command, color=theme.PANEL2, fg=theme.TEXT, width=None):
        button = tk.Button(
            parent,
            text=text,
            command=command,
            bg=color,
            fg=fg,
            activebackground=theme.PANEL3,
            activeforeground=theme.WHITE,
            relief="flat",
            font=theme.FONT_UI_BOLD,
            padx=12,
            pady=6,
            cursor="hand2",
        )
        if width:
            button.configure(width=width)
        button.pack(side="left", padx=4, pady=6)
        return button

    def _build_toolbar(self):
        bar = tk.Frame(self, bg=theme.PANEL)
        bar.pack(fill="x")
        self._tool_button(bar, "RUN  (F5)", self.run_program, theme.GREEN, "#10261a")
        self._tool_button(bar, "STOP (Esc)", self.stop_program, theme.RED, "#2b0f0f")
        tk.Frame(bar, bg=theme.PANEL3, width=2).pack(side="left", fill="y", padx=8, pady=8)
        self._tool_button(bar, "New", self.new_file)
        self._tool_button(bar, "Open", self.open_file)
        self._tool_button(bar, "Save", self.save_file)
        tk.Frame(bar, bg=theme.PANEL3, width=2).pack(side="left", fill="y", padx=8, pady=8)
        self._tool_button(bar, "Examples", self.open_examples, theme.PANEL2)
        self._tool_button(bar, "Dictionary", self.open_dictionary, theme.PANEL2)
        self._tool_button(bar, "Export .py", self.export_python, theme.PANEL2)
        self._tool_button(bar, "A+", lambda: self.change_font(1), theme.PANEL2)
        self._tool_button(bar, "A-", lambda: self.change_font(-1), theme.PANEL2)

    def _build_body(self):
        style = ttk.Style(self)
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass
        style.configure("TNotebook", background=theme.PANEL, borderwidth=0)
        style.configure("TNotebook.Tab", background=theme.PANEL2, foreground=theme.TEXT,
                        padding=(14, 8), font=theme.FONT_UI_BOLD)
        style.map("TNotebook.Tab",
                  background=[("selected", theme.BG)],
                  foreground=[("selected", theme.ACCENT)])
        style.configure("TPanedwindow", background=theme.PANEL)

        body = tk.Frame(self, bg=theme.BG)
        body.pack(fill="both", expand=True)

        self.notebook = ttk.Notebook(body)
        self.notebook.pack(fill="both", expand=True)

        # ---- blocks tab
        blocks_tab = tk.Frame(self.notebook, bg=theme.BG)
        row = tk.Frame(blocks_tab, bg=theme.PANEL)
        row.pack(fill="x")
        self._small_button(row, "Run blocks", self.run_program, theme.GREEN, "#10261a")
        self._small_button(row, "Stop", self.stop_program, theme.RED, "#2b0f0f")
        self._small_button(row, "Send to Text tab", self.send_blocks_to_text)
        self._small_button(row, "Load from Text tab", self.load_text_to_blocks)
        self._small_button(row, "Undo", lambda: self.blockcanvas.undo())
        self._small_button(row, "Clear blocks", self.clear_blocks)

        pane = tk.PanedWindow(blocks_tab, orient="horizontal", bg=theme.PANEL, sashwidth=6)
        pane.pack(fill="both", expand=True)
        self.blockcanvas = BlockCanvas(pane, on_change=self._blocks_changed, on_status=self._set_status)
        pane.add(self.blockcanvas, minsize=560)

        code_holder = tk.Frame(pane, bg=theme.BG)
        tk.Label(code_holder, text="YOUR CODE (it writes itself!)", bg=theme.PANEL,
                 fg=theme.ACCENT, font=theme.FONT_UI_BOLD, anchor="w", padx=10, pady=6).pack(fill="x")
        self.block_code = tk.Text(
            code_holder, bg=theme.BG, fg=theme.TEXT, font=theme.FONT_CODE_SMALL,
            wrap="none", border=0, highlightthickness=0, padx=10, pady=6, state="disabled",
        )
        self.block_code.pack(fill="both", expand=True)
        from . import highlight as highlighter
        highlighter.configure(self.block_code)
        pane.add(code_holder, minsize=260)
        self.notebook.add(blocks_tab, text=BLOCKS_TAB)

        # ---- text tab
        text_tab = tk.Frame(self.notebook, bg=theme.BG)
        self.editor = CodeEditor(text_tab, on_change=self._editor_changed)
        self.editor.pack(fill="both", expand=True)
        self.notebook.add(text_tab, text=TEXT_TAB)

        # ---- draw tab
        draw_tab = tk.Frame(self.notebook, bg=theme.BG)
        self.draw_panel = DrawingPanel(draw_tab)
        self.draw_panel.pack(fill="both", expand=True)
        self.notebook.add(draw_tab, text=DRAW_TAB)

        self.output = OutputPane(body)
        self.output.pack(fill="x", side="bottom")

    def _small_button(self, parent, text, command, color=theme.PANEL2, fg=theme.TEXT):
        button = tk.Button(
            parent, text=text, command=command, bg=color, fg=fg,
            activebackground=theme.PANEL3, activeforeground=theme.WHITE,
            relief="flat", font=theme.FONT_SMALL, padx=8, pady=3, cursor="hand2",
        )
        button.pack(side="left", padx=4, pady=4)
        return button

    def _build_status(self):
        bar = tk.Frame(self, bg=theme.PANEL)
        bar.pack(fill="x", side="bottom")
        self.status_label = tk.Label(
            bar, text="Welcome to OGABOOGA CODER! Press RUN to try your code.",
            bg=theme.PANEL, fg=theme.TEXT, font=theme.FONT_SMALL, anchor="w", padx=10, pady=4,
        )
        self.status_label.pack(side="left", fill="x", expand=True)
        self.position_label = tk.Label(
            bar, text="compiler inside - v" + VERSION, bg=theme.PANEL, fg=theme.DIM,
            font=theme.FONT_SMALL, padx=10,
        )
        self.position_label.pack(side="right")

    # -------------------------------------------------------- small hooks

    def _set_status(self, message, color=None):
        self.status_label.configure(text=message, fg=color or theme.TEXT)

    def _editor_changed(self):
        self.position_label.configure(text=self.editor.position_text())

    def _blocks_changed(self):
        source = self.blockcanvas.source()
        self.block_code.configure(state="normal")
        self.block_code.delete("1.0", "end")
        self.block_code.insert("1.0", source)
        self.block_code.configure(state="disabled")
        from . import highlight as highlighter
        highlighter.highlight(self.block_code)
        problems = B.check_blocks(self.blockcanvas.get_blocks())
        if problems:
            self._set_status("Block check: " + problems[0], theme.YELLOW)
        else:
            self._set_status("Blocks look good! Press RUN.")

    def _program_output(self, line, tag="out"):
        self._out_queue.append((line, tag))

    def _run_done(self, outcome):
        self._done_queue.append(outcome)

    def _pump(self):
        while self._out_queue:
            line, tag = self._out_queue.pop(0)
            self.output.write(line, tag)

        while self._done_queue:
            outcome = self._done_queue.pop(0)
            self._finish_run(outcome)

        if self.runner is not None and self.runner.pending_input() and not self._input_open:
            prompt, _line = self.runner.pending_input()
            self.runner.clear_pending_input()
            self._input_open = True
            try:
                answer = simpledialog.askstring(
                    "Ooga! A question",
                    prompt if prompt else "Type an answer:",
                    parent=self,
                )
            finally:
                self._input_open = False
            self.runner.answer_input(answer if answer is not None else "")

        if self.runner is not None and self.runner.running:
            self.position_label.configure(text="running...")

        try:
            if self.winfo_exists():
                self._pump_id = self.after(40, self._pump)
        except tk.TclError:
            self._pump_id = None

    def _finish_run(self, outcome):
        if outcome is None:
            self.output.write("Program finished! Well done, cave coder.", "ok")
            self._set_status("Program finished!", theme.GREEN)
        elif outcome == "stopped":
            self.output.write("Stopped! (you pressed stop)", "err")
            self._set_status("Stopped.", theme.RED)
        elif isinstance(outcome, cmc.CmcError):
            self.output.write(outcome.format(), "err")
            self._set_status("Ooga! The program needs a fix.", theme.RED)
        else:
            self.output.write("CMC had a bug: " + repr(outcome), "err")
            self._set_status("CMC bug -- please report it.", theme.RED)
        if self.current_path:
            self.position_label.configure(text=self.current_path)
        else:
            self.position_label.configure(text="compiler inside - v" + VERSION)

    # ------------------------------------------------------------- actions

    def _current_tab(self):
        try:
            return self.notebook.tab(self.notebook.select(), "text")
        except tk.TclError:
            return TEXT_TAB

    def run_program(self):
        if self.runner.running:
            self._set_status("Already running! Press STOP first.", theme.YELLOW)
            return
        tab = self._current_tab()
        if tab == BLOCKS_TAB:
            problems = B.check_blocks(self.blockcanvas.get_blocks())
            if problems:
                self.output.write("The blocks need a little fix first:\n" + B.describe_problems(problems), "err")
                self._set_status("Fix the block slots first.", theme.RED)
                return
            source = self.blockcanvas.source()
            where = "Blocks"
        else:
            source = self.editor.get_text()
            where = "Text"
        if not source.strip():
            self._set_status("There is no code to run yet.", theme.YELLOW)
            return
        problem = B.validate_source(source)
        if problem:
            self.output.write(problem, "err")
            self._set_status("Ooga! The program needs a fix.", theme.RED)
            return
        self.output.clear()
        self.output.write("--- running (" + where + ") ---", "info")
        self.draw_panel.wipe()
        name = self.current_path or "<your program>"
        self._set_status("Running...", theme.GREEN)
        self.runner.start(source, filename=name, draw=self.draw_panel)

    def stop_program(self):
        if self.runner.running:
            self.runner.stop()
            self._set_status("Stopping...", theme.YELLOW)

    def new_file(self):
        self.editor.clear()
        self.current_path = None
        self.notebook.select(1)
        self._set_status("New empty cave. Start typing!")
        self.editor.focus_editor()

    def open_file(self):
        path = filedialog.askopenfilename(
            title="Open a CMC program",
            filetypes=[("Cave Man Code", "*.cmc"), ("All files", "*.*")],
        )
        if not path:
            return
        try:
            with open(path, "r", encoding="utf-8") as handle:
                source = handle.read()
        except OSError as error:
            messagebox.showerror("OOGA!", "Could not read that file:\n" + str(error))
            return
        self.editor.set_text(source)
        self.current_path = path
        self.notebook.select(1)
        self._set_status("Opened " + os.path.basename(path))

    def save_file(self):
        if self.current_path is None:
            return self.save_file_as()
        return self._write_current(self.current_path)

    def save_file_as(self):
        path = filedialog.asksaveasfilename(
            title="Save your program",
            defaultextension=".cmc",
            filetypes=[("Cave Man Code", "*.cmc"), ("All files", "*.*")],
        )
        if not path:
            return False
        return self._write_current(path)

    def _write_current(self, path):
        if self._current_tab() == BLOCKS_TAB:
            source = self.blockcanvas.source()
        else:
            source = self.editor.get_text()
        try:
            with open(path, "w", encoding="utf-8") as handle:
                handle.write(source)
        except OSError as error:
            messagebox.showerror("OOGA!", "Could not save:\n" + str(error))
            return False
        self.current_path = path
        self._set_status("Saved " + os.path.basename(path), theme.GREEN)
        return True

    def export_python(self):
        if self._current_tab() == BLOCKS_TAB:
            source = self.blockcanvas.source()
        else:
            source = self.editor.get_text()
        if not source.strip():
            self._set_status("Nothing to export yet.", theme.YELLOW)
            return
        try:
            python_source = cmc.to_python_source(source, filename=self.current_path or "<your program>")
        except cmc.CmcError as error:
            self.output.write(error.format(), "err")
            return
        path = filedialog.asksaveasfilename(
            title="Export as Python",
            defaultextension=".py",
            filetypes=[("Python", "*.py"), ("All files", "*.*")],
        )
        if not path:
            return
        try:
            with open(path, "w", encoding="utf-8") as handle:
                handle.write(python_source)
        except OSError as error:
            messagebox.showerror("OOGA!", "Could not save:\n" + str(error))
            return
        self._set_status("Exported " + os.path.basename(path), theme.GREEN)
        self.output.write("Made a real Python file: " + path, "ok")

    def open_examples(self):
        ExamplesDialog(self, self._example_picked)

    def _example_picked(self, path, source, mode):
        if mode == "blocks":
            try:
                loaded = B.source_to_blocks(source)
            except B.BlockError as error:
                messagebox.showwarning(
                    "Almost!",
                    "This example uses ideas that do not have blocks yet:\n\n" + str(error)
                    + "\n\nOpening it in the Text tab instead.",
                    parent=self,
                )
                mode = "text"
            else:
                self.blockcanvas.set_blocks(loaded)
                self.notebook.select(0)
                self._set_status("Loaded " + os.path.basename(path) + " as blocks!")
                return
        self.editor.set_text(source)
        self.current_path = None
        self.notebook.select(1)
        self._set_status("Loaded example " + os.path.basename(path) + " (Save as to keep changes)")

    def open_dictionary(self):
        DictionaryDialog(self)

    def send_blocks_to_text(self):
        source = self.blockcanvas.source()
        if not source.strip():
            self._set_status("No blocks yet.", theme.YELLOW)
            return
        self.editor.set_text(source)
        self.notebook.select(1)
        self._set_status("Blocks sent to the Text tab.")

    def load_text_to_blocks(self):
        source = self.editor.get_text()
        if not source.strip():
            self._set_status("The Text tab is empty.", theme.YELLOW)
            return
        try:
            loaded = B.source_to_blocks(source)
        except B.BlockError as error:
            messagebox.showwarning("Almost!", str(error), parent=self)
            return
        self.blockcanvas.set_blocks(loaded)
        self.notebook.select(0)
        self._set_status("Text loaded into blocks!")

    def clear_blocks(self):
        self.blockcanvas.clear_all()
        self._set_status("Blocks cleared. Undo still works!")

    def change_font(self, delta):
        for widget, name in (
            (self.editor.text, "code"),
            (self.editor.gutter, "code"),
            (self.block_code, "code_small"),
            (self.output.text, "code_small"),
        ):
            family, size = theme.FONT_CODE if name == "code" else theme.FONT_CODE_SMALL
            size = max(8, min(26, size + delta))
            widget.configure(font=(family, size))
        self._set_status("Font size changed.")

    def open_cheatsheet(self):
        path = cmc.paths.repo_root()
        target = os.path.join(path, "CHEATSHEET.md") if path else None
        self._open_local(target)

    def open_wiki(self):
        folder = cmc.paths.website_dir()
        target = os.path.join(folder, "index.html") if folder else None
        self._open_local(target)

    def _open_local(self, target):
        if not target or not os.path.exists(target):
            messagebox.showinfo(
                "Not found",
                "I could not find that file in this copy of CMC.",
                parent=self,
            )
            return
        try:
            if sys.platform.startswith("win"):
                os.startfile(target)
            else:
                import webbrowser
                webbrowser.open("file://" + target)
        except OSError as error:
            messagebox.showerror("OOGA!", str(error), parent=self)

    def _on_close(self):
        try:
            self.runner.stop()
        except Exception:
            pass
        try:
            if self._pump_id is not None:
                self.after_cancel(self._pump_id)
        except Exception:
            pass
        self.destroy()


def selftest():
    app = OgaboogaApp(selftest=True)
    app.update()

    problems = []
    lines = []
    cmc.run_source('oga "hello from selftest"\ngrunk xs = snorf(1, 2)\nplop(xs, 3)\noga join(xs, "-")\n',
                   output=lines.append)
    if lines != ["hello from selftest", "1-2-3"]:
        problems.append("interpreter output was " + repr(lines))

    source = "grunk x = 1\nbooga 3\n    oga lap\nunga\n"
    try:
        loaded = B.source_to_blocks(source)
        generated = B.blocks_to_source(loaded)
    except Exception as error:
        problems.append("block round trip failed: " + repr(error))
    else:
        lines = []
        cmc.run_source(generated, output=lines.append)
        if lines != ["1", "2", "3"]:
            problems.append("generated block code printed " + repr(lines))

    app.destroy()
    if problems:
        print("OGABOOGA SELFTEST FAILED:")
        for problem in problems:
            print("  - " + problem)
        return 1
    print("OGABOOGA SELFTEST OK")
    return 0


def main(argv=None):
    args = list(sys.argv[1:] if argv is None else argv)
    if "--selftest" in args:
        return selftest()
    app = OgaboogaApp()
    app.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())