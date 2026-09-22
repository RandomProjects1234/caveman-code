import time
import unittest

from tests.context import SRC  # noqa: F401

from ogabooga import blocks as B


def make_app():
    import tkinter as tk

    try:
        from ogabooga.app import OgaboogaApp

        app = OgaboogaApp(selftest=True)
        app.update()
        return app
    except tk.TclError as error:
        raise unittest.SkipTest("no display for tkinter: " + str(error))


class TestBlockCanvas(unittest.TestCase):
    def setUp(self):
        self.app = make_app()
        self.canvas = self.app.blockcanvas

    def tearDown(self):
        try:
            self.app.destroy()
        except Exception:
            pass

    def test_click_add_and_generate(self):
        self.canvas.add_block("say")
        self.assertEqual(len(self.canvas.blocks), 1)
        code = self.canvas.source()
        self.assertIn("oga", code)

    def test_drop_targets(self):
        repeat_block = B.new_block("repeat")
        self.canvas.set_blocks([repeat_block])
        self.canvas.redraw()
        entry = self.canvas._entry_for(repeat_block)
        bx, by, _bw, _bh = entry["body_rect"]
        target = self.canvas._drop_target(bx + 30, by + 20)
        self.assertEqual(target, ("insert", repeat_block, "body", 0))

        fx, fy, _fw, _fh = entry["fields"]["count"]
        target = self.canvas._drop_target(fx + 5, fy + 5)
        self.assertEqual(target[0], "field")
        self.assertEqual(target[2], "count")

        say_block = B.new_block("say")
        repeat_block.body.append(say_block)
        self.canvas.redraw()
        sequence = self.canvas.source().splitlines()
        self.assertEqual(sequence[0], "booga 3")
        self.assertTrue(sequence[1].startswith("    oga"))

    def test_value_drop_into_field(self):
        say_block = B.new_block("say")
        self.canvas.set_blocks([say_block])
        self.canvas.redraw()
        entry = self.canvas._entry_for(say_block)
        fx, fy, _fw, _fh = entry["fields"]["expr"]
        target = self.canvas._drop_target(fx + 5, fy + 5)
        self.assertEqual(target[0], "field")
        number = B.new_block("value_number")
        number.fields["value"] = "42"
        target[1].fields[target[2]] = B.value_expression(number)
        self.assertEqual(self.canvas.source().strip(), "oga 42")

    def test_undo_after_add(self):
        self.canvas.add_block("say")
        self.canvas.add_block("grunk")
        self.assertEqual(len(self.canvas.blocks), 2)
        self.canvas.undo()
        self.assertEqual(len(self.canvas.blocks), 1)

    def test_field_validation(self):
        say_block = B.new_block("say")
        self.assertTrue(self.canvas._valid_field(say_block, "expr", "1 + 2"))
        self.assertFalse(self.canvas._valid_field(say_block, "expr", "1 +"))
        grunk = B.new_block("grunk")
        self.assertFalse(self.canvas._valid_field(grunk, "name", "nom"))
        self.assertTrue(self.canvas._valid_field(grunk, "name", "my_box"))

    def test_if_else_drop(self):
        if_block = B.new_block("if")
        self.canvas.set_blocks([if_block])
        self.canvas.redraw()
        entry = self.canvas._entry_for(if_block)
        ex, ey, _ew, _eh = entry["else_rect"]
        target = self.canvas._drop_target(ex + 20, ey + 14)
        self.assertEqual(target, ("insert", if_block, "else", 0))

    def test_delete_selected(self):
        block = B.new_block("say")
        self.canvas.set_blocks([block])
        self.canvas.selected = block
        self.canvas.delete_selected()
        self.assertEqual(self.canvas.blocks, [])


class TestIdeRun(unittest.TestCase):
    def setUp(self):
        self.app = make_app()

    def tearDown(self):
        try:
            self.app.destroy()
        except Exception:
            pass

    def test_run_text_program(self):
        self.app.notebook.select(1)
        self.app.editor.set_text('oga "from the ide"\n')
        self.app.run_program()
        deadline = time.time() + 15
        while time.time() < deadline:
            self.app.update()
            if not self.app.runner.running and not self.app._out_queue and not self.app._done_queue:
                break
            time.sleep(0.02)
        output = self.app.output.text.get("1.0", "end")
        self.assertIn("from the ide", output)
        self.assertIn("Program finished", output)

    def test_run_bad_program_shows_ooga(self):
        self.app.notebook.select(1)
        self.app.editor.set_text("unga\n")
        self.app.run_program()
        deadline = time.time() + 15
        while time.time() < deadline:
            self.app.update()
            if not self.app.runner.running and not self.app._out_queue and not self.app._done_queue:
                break
            time.sleep(0.02)
        output = self.app.output.text.get("1.0", "end")
        self.assertIn("OOGA!", output)


if __name__ == "__main__":
    unittest.main()