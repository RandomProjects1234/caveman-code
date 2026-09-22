"""The helper words (builtin clumps) of CMC.

Every helper checks its arguments and complains with a friendly OOGA! when
something is the wrong kind of thing.
"""

import math
import random

from .errors import CmcRuntimeError
from .values import show, type_name, number, whole
from . import words

CONSTANTS = {
    "gronk": True,
    "nork": False,
    "plop": None,
    "pi": math.pi,
}

# Arity for every helper, pulled from the caveman dictionary.
ARITY = {}
for _w in words.BUILTIN_WORDS:
    _call_name = _w.get("display", _w["name"])
    ARITY[_call_name] = _w["arity"]


def _need_text(value, line, extra):
    if not isinstance(value, str):
        raise CmcRuntimeError(
            extra + " needs text (goop), but got " + type_word(value) + ".",
            line=line,
            hint='put quotes around it, like "hello".',
        )
    return value


def _need_pile(value, line, extra):
    if not isinstance(value, list):
        raise CmcRuntimeError(
            extra + " needs a pile (snorf), but got " + type_word(value) + ".",
            line=line,
            hint="make a pile like this: grunk xs = snorf(1, 2, 3)",
        )
    return value


def _spot(value, line, extra):
    """A pile spot number (whole, may be negative)."""
    idx = whole(value, line=line, extra=extra)
    return idx


def bi_goop(interp, args, line):
    return show(args[0])


def bi_snorf(interp, args, line):
    return list(args)


def bi_nom(interp, args, line):
    value = args[0]
    if isinstance(value, (str, list)):
        return len(value)
    raise CmcRuntimeError(
        "nom counts piles and text, but got " + type_word(value) + ".",
        line=line,
        hint="nom(pets) counts a pile. nom(\"cave\") counts letters.",
    )


def bi_skoop(interp, args, line):
    value, idx = args[0], _spot(args[1], line, "skoop")
    if not isinstance(value, (str, list)):
        raise CmcRuntimeError(
            "skoop reaches into piles and text, but got " + type_word(value) + ".",
            line=line,
            hint="make a pile like this: grunk xs = snorf(1, 2, 3)",
        )
    if idx < 0:
        idx += len(value)
    if idx < 0 or idx >= len(value):
        raise CmcRuntimeError(
            "There is no spot " + show(idx) + " in that " + ("text" if isinstance(value, str) else "pile")
            + " (it has " + str(len(value)) + " spots).",
            line=line,
            hint="spots start at 0, so the last spot is " + str(len(value) - 1) + ".",
        )
    return value[idx]


def bi_yoink(interp, args, line):
    pile = _need_pile(args[0], line, "yoink")
    if not pile:
        raise CmcRuntimeError(
            "The pile is empty, so there is nothing to yoink!",
            line=line,
            hint="check nom(pile) > 0 before yoinking.",
        )
    return pile.pop()


def bi_plop(interp, args, line):
    pile = _need_pile(args[0], line, "plop")
    pile.append(args[1])
    return None


def bi_shout(interp, args, line):
    return _need_text(args[0], line, "shout").upper()


def bi_whisper(interp, args, line):
    return _need_text(args[0], line, "whisper").lower()


def bi_flip(interp, args, line):
    return _need_text(args[0], line, "flip")[::-1]


def bi_find(interp, args, line):
    text = _need_text(args[0], line, "find")
    piece = _need_text(args[1], line, "find")
    return text.find(piece)


def bi_split(interp, args, line):
    text = _need_text(args[0], line, "split")
    sep = _need_text(args[1], line, "split")
    if sep == "":
        raise CmcRuntimeError(
            "split needs something to cut on, and an empty text cuts nothing.",
            line=line,
        )
    return text.split(sep)


def bi_join(interp, args, line):
    pile = _need_pile(args[0], line, "join")
    sep = show(args[1]) if not isinstance(args[1], str) else args[1]
    return sep.join(show(item) for item in pile)


def bi_what(interp, args, line):
    return type_name(args[0])


def bi_numba(interp, args, line):
    value = args[0]
    if isinstance(value, bool):
        raise CmcRuntimeError(
            "numba cannot turn gronk/nork into a number.",
            line=line,
            hint="use 1 for gronk or 0 for nork.",
        )
    if isinstance(value, (int, float)):
        return value
    if isinstance(value, str):
        text = value.strip()
        try:
            return int(text)
        except ValueError:
            pass
        try:
            return float(text)
        except ValueError:
            raise CmcRuntimeError(
                "numba cannot turn \"" + value + "\" into a number.",
                line=line,
                hint='answers that are numbers look like "42" or "3.5".',
            )
    raise CmcRuntimeError(
        "numba needs text or a number, but got " + type_word(value) + ".",
        line=line,
        hint='use it like this: numba("42")',
    )


def bi_munga(interp, args, line):
    lo = number(args[0], line=line, extra="munga")
    hi = number(args[1], line=line, extra="munga")
    if lo > hi:
        lo, hi = hi, lo
    if isinstance(lo, int) and isinstance(hi, int):
        return random.randint(lo, hi)
    return random.uniform(lo, hi)


def bi_round(interp, args, line):
    value = number(args[0], line=line, extra="round")
    return int(math.floor(value + 0.5)) if value >= 0 else -int(math.floor(-value + 0.5))


def bi_flat(interp, args, line):
    return math.floor(number(args[0], line=line, extra="flat"))


def bi_roof(interp, args, line):
    return math.ceil(number(args[0], line=line, extra="roof"))


def bi_abs(interp, args, line):
    return abs(number(args[0], line=line, extra="abs"))


def bi_small(interp, args, line):
    a = number(args[0], line=line, extra="small")
    b = number(args[1], line=line, extra="small")
    return a if a <= b else b


def bi_big(interp, args, line):
    a = number(args[0], line=line, extra="big")
    b = number(args[1], line=line, extra="big")
    return a if a >= b else b


def bi_root(interp, args, line):
    value = number(args[0], line=line, extra="root")
    if value < 0:
        raise CmcRuntimeError(
            "root cannot work on negative numbers (nothing times itself makes " + show(value) + ").",
            line=line,
            hint="use abs(value) first if you want the size without the minus.",
        )
    result = math.sqrt(value)
    return int(result) if result.is_integer() else result


def bi_pow(interp, args, line):
    base = number(args[0], line=line, extra="pow")
    times = number(args[1], line=line, extra="pow")
    result = base ** times
    if isinstance(result, float) and result.is_integer():
        return int(result)
    return result


def bi_wait(interp, args, line):
    ms = number(args[0], line=line, extra="wait")
    if ms < 0:
        ms = 0
    interp.wait_ms(ms)
    return None


IMPLS = {
    "goop": bi_goop,
    "snorf": bi_snorf,
    "nom": bi_nom,
    "skoop": bi_skoop,
    "yoink": bi_yoink,
    "plop": bi_plop,
    "shout": bi_shout,
    "whisper": bi_whisper,
    "flip": bi_flip,
    "find": bi_find,
    "split": bi_split,
    "join": bi_join,
    "what": bi_what,
    "numba": bi_numba,
    "munga": bi_munga,
    "round": bi_round,
    "flat": bi_flat,
    "roof": bi_roof,
    "abs": bi_abs,
    "small": bi_small,
    "big": bi_big,
    "root": bi_root,
    "pow": bi_pow,
    "wait": bi_wait,
}

HELPER_NAMES = set(IMPLS)


def call_builtin(interp, name, args, line):
    impl = IMPLS.get(name)
    if impl is None:
        return None, False
    low, high = ARITY[name]
    if len(args) < low or len(args) > high:
        if low == high:
            wanted = str(low)
        else:
            wanted = str(low) + " to " + str(high)
        raise CmcRuntimeError(
            "'" + name + "' wants " + wanted + " thing" + ("s" if wanted != "1" else "")
            + ", but got " + str(len(args)) + ".",
            line=line,
            hint="look at the caveman dictionary (Help menu) for how " + name + " works.",
        )
    return impl(interp, args, line), True