"""Friendly errors for Cave Man Code.

Every error a kid can see should be easy to read, easy to fix, and a little
bit funny.  All CMC errors look like:

    OOGA! Line 4: You said 'unga' but nothing was open.
    Hint: every 'unga' closes one thing that started before it.
"""

OOGA = "OOGA!"


class CmcError(Exception):
    """A friendly mistake in a CMC program."""

    def __init__(self, message, line=None, hint=None, where=None):
        self.message = message
        self.line = line
        self.hint = hint
        self.where = where
        super().__init__(message)

    def format(self):
        parts = []
        location = ""
        if self.line is not None:
            location = " Line " + str(self.line) + ":"
        elif self.where:
            location = " " + self.where + ":"
        parts.append(OOGA + location + " " + self.message)
        if self.hint:
            parts.append("Hint: " + self.hint)
        return "\n".join(parts)

    def __str__(self):
        return self.format()


class CmcLexError(CmcError):
    pass


class CmcParseError(CmcError):
    pass


class CmcRuntimeError(CmcError):
    pass


class CmcStopped(CmcError):
    def __init__(self):
        super().__init__("Stopped!", hint=None)

    def format(self):
        return "Stopped! (you pushed the stop button)"


class CmcStepLimit(CmcRuntimeError):
    def __init__(self, line=None):
        super().__init__(
            "This program ran for a very long time. Maybe a loop never stops?",
            line=line,
            hint="check your 'zug' loops. Make sure the box they check gets changed inside the loop.",
        )


class CmcTooDeep(CmcRuntimeError):
    def __init__(self, name=None, line=None):
        who = ("'" + name + "'") if name else "A clump"
        super().__init__(
            who + " called itself too many times and went too deep.",
            line=line,
            hint="a clump can call itself, but it needs a way to stop. Use 'binga' to check.",
        )


def ooga(message, line=None, hint=None):
    """Raise a friendly runtime error."""
    raise CmcRuntimeError(message, line=line, hint=hint)