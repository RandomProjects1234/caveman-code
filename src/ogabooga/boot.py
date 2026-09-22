"""Make sure the compiler is importable, even when running from the repo."""

import os
import sys


def ensure_cmc():
    try:
        import cmc  # noqa: F401
        return cmc
    except ImportError:
        here = os.path.dirname(os.path.abspath(__file__))
        src = os.path.dirname(here)
        if src not in sys.path:
            sys.path.insert(0, src)
        import cmc
        return cmc


cmc = ensure_cmc()