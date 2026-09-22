"""Running a CMC program without freezing the window.

The program runs in its own thread.  Everything it says goes into a queue
that the main window drains.  Questions pop up friendly dialog windows.
"""

import threading

from .boot import cmc


class RunManager:
    def __init__(self, on_output, on_input_request, on_done):
        self.on_output = on_output
        self.on_input_request = on_input_request
        self.on_done = on_done

        self.thread = None
        self.running = False
        self.stopping = False
        self.stop_event = threading.Event()
        self.filename = "<your program>"

        self._pending_input = None
        self._input_event = None
        self._input_value = None

    # -------------------------------------------------------------- start

    def start(self, source, filename="<your program>", draw=None):
        if self.running:
            return False
        self.running = True
        self.stopping = False
        self.stop_event = threading.Event()
        self.filename = filename

        interpreter = cmc.Interpreter(
            output=lambda line: self.on_output(line, "out"),
            input_func=self._ask,
            draw=draw,
            should_stop=lambda: self.stopping,
            sleep=self._sleep,
        )

        def work():
            try:
                interpreter.run(source, filename=filename)
                outcome = None
            except cmc.CmcStopped:
                outcome = "stopped"
            except cmc.CmcError as error:
                outcome = error
            except Exception as error:  # a bug in CMC itself
                outcome = error
            self.running = False
            self._pending_input = None
            if self._input_event is not None:
                self._input_event.set()
            self.on_done(outcome)

        self.thread = threading.Thread(target=work, daemon=True)
        self.thread.start()
        return True

    def stop(self):
        if not self.running:
            return
        self.stopping = True
        self.stop_event.set()
        if self._input_event is not None:
            self._input_event.set()

    # ------------------------------------------------- thread-side helpers

    def _sleep(self, seconds):
        self.stop_event.wait(seconds)

    def _ask(self, prompt, line):
        event = threading.Event()
        self._input_event = event
        self._pending_input = (prompt, line)
        self._input_value = None
        while not event.wait(0.05):
            if self.stopping:
                raise cmc.CmcStopped()
        self._input_event = None
        if self.stopping:
            raise cmc.CmcStopped()
        return self._input_value if self._input_value is not None else ""

    # -------------------------------------------------- main-side helpers

    def answer_input(self, value):
        self._input_value = value
        if self._input_event is not None:
            self._input_event.set()

    def pending_input(self):
        return self._pending_input

    def clear_pending_input(self):
        self._pending_input = None