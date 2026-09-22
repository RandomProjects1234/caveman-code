"""Cave Man Code (CMC) -- the super easy caveman programming language.

Quick start:

    import cmc
    cmc.run_source('oga "Hello cave!"')
"""

from .errors import (  # noqa: F401
    CmcError,
    CmcLexError,
    CmcParseError,
    CmcRuntimeError,
    CmcStopped,
    CmcStepLimit,
    CmcTooDeep,
)
from .interpreter import Interpreter, Env, run_source  # noqa: F401
from .parser import parse_source, parse_expression_source  # noqa: F401
from .codegen import to_python_source, expr_to_cmc  # noqa: F401
from . import words  # noqa: F401
from . import paths  # noqa: F401

VERSION = words.VERSION
__version__ = VERSION