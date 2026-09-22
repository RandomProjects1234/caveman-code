"""Values and how CMC sees them.

CMC values map onto plain Python values:

    number  -> int or float
    text    -> str
    truth   -> bool   (gronk / nork)
    plop    -> None   (nothing)
    pile    -> list   (a pile of things)
"""


def type_name(value):
    if value is None:
        return "plop"
    if isinstance(value, bool):
        return "truth"
    if isinstance(value, (int, float)):
        return "number"
    if isinstance(value, str):
        return "text"
    if isinstance(value, list):
        return "pile"
    return "thing"


def type_word(value):
    """A kid-friendly sentence for 'this thing is a ...'."""
    names = {
        "plop": "nothing (plop)",
        "truth": "a truth (gronk or nork)",
        "number": "a number",
        "text": "text (goop)",
        "pile": "a pile (snorf)",
        "thing": "a strange thing",
    }
    return names[type_name(value)]


def show(value):
    """Turn any CMC value into the text that 'oga' should print."""
    if value is None:
        return "plop"
    if value is True:
        return "gronk"
    if value is False:
        return "nork"
    if isinstance(value, list):
        return "[" + ", ".join(show(item) for item in value) + "]"
    if isinstance(value, float) and value.is_integer():
        return str(int(value))
    return str(value)


def truthy(value):
    """What counts as 'gronk' in a binga/zug check."""
    if value is None:
        return False
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return value != 0
    if isinstance(value, str):
        return len(value) > 0
    if isinstance(value, list):
        return len(value) > 0
    return True


def equal(a, b):
    """== that never crashes: different kinds of things are just not equal."""
    if isinstance(a, bool) or isinstance(b, bool):
        return isinstance(a, bool) and isinstance(b, bool) and a == b
    if a is None or b is None:
        return a is None and b is None
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return a == b
    if isinstance(a, str) and isinstance(b, str):
        return a == b
    if isinstance(a, list) and isinstance(b, list):
        return len(a) == len(b) and all(equal(x, y) for x, y in zip(a, b))
    return False


def number(value, line=None, extra=None):
    """Make sure a value is a number, or raise a friendly error."""
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        from .errors import CmcRuntimeError

        what = (extra + " needs") if extra else "This needs"
        raise CmcRuntimeError(
            what + " a number, but got " + type_word(value) + ".",
            line=line,
            hint="use numbers like 1, 2, or 3.5 -- or wrap things in 'goop' to make text.",
        )
    return value


def whole(value, line=None, extra=None):
    """Make sure a value is a whole number (like a pile spot or a count)."""
    n = number(value, line=line, extra=extra)
    if isinstance(n, float):
        if n.is_integer():
            return int(n)
        from .errors import CmcRuntimeError

        what = (extra + " needs") if extra else "This needs"
        raise CmcRuntimeError(
            what + " a whole number, but got " + show(n) + ".",
            line=line,
            hint="try 'round' or 'flat' to make it whole.",
        )
    return n


def add(a, b):
    """+ for CMC: numbers add, text sticks together."""
    if isinstance(a, str) and isinstance(b, str):
        return a + b
    if isinstance(a, bool) or isinstance(b, bool):
        return None  # signal: not allowed
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return a + b
    return None


def isnum(v):
    return isinstance(v, (int, float)) and not isinstance(v, bool)