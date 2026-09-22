"""The `cmc` command line.

    cmc                  start the talk-back REPL
    cmc run hello.cmc    run a program
    cmc compile hello.cmc    make a .py file from it
    cmc init hello.cmc   make a starter program
    cmc examples         list the example programs
    cmc version          who am I?
"""

import os
import sys

from .errors import CmcError, CmcStopped
from .interpreter import Interpreter
from .codegen import to_python_source
from .draw import TkDrawSurface
from .lexer import tokenize, Token  # noqa: F401
from .paths import examples_dir
from .words import VERSION

HELP = """\
Cave Man Code (CMC) v{version}

  cmc                    talk to CMC line by line (REPL)
  cmc run <file.cmc>     run a program
  cmc compile <file.cmc> [out.py]
                         turn a program into Python
  cmc init [file.cmc]    make a starter program (default hello.cmc)
  cmc examples           show the example programs
  cmc version            show the version
  cmc help               show this

Learn to talk caveman in 30 minutes: read docs/01_TUTORIAL_30_MINUTES.md
or open the wiki in website/index.html
"""

STARTER = '''\
ugg Welcome to Cave Man Code!
ugg This is a note: the computer skips everything after "ugg".

oga "Ooga booga! Me make first program!"

grunk name = blorp "What is your name?"
oga "Hello", name, "! You are a cave coder now."

grunk rocks = snorf()
plop(rocks, "big rock")
plop(rocks, "shiny rock")

booga 3
    oga "Me bang rock!"
unga

oga "Me have", nom(rocks), "rocks."
zoop rock in rocks
    oga "  -", rock
unga
'''

BLOCK_OPENERS = {"binga", "booga", "zug", "zoop", "clump"}


def _setup_console():
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass


def _examples_dir():
    return examples_dir()


def _read_file(path):
    if not os.path.exists(path):
        print("OOGA! I cannot find the file '" + path + "'.")
        print("Hint: check the name, or make a new one with: cmc init " + path)
        return None
    try:
        with open(path, "r", encoding="utf-8") as handle:
            return handle.read()
    except OSError as exc:
        print("OOGA! I could not read '" + path + "': " + str(exc))
        return None


def cmd_run(path):
    source = _read_file(path)
    if source is None:
        return 1
    draw = TkDrawSurface(title="CMC picture - " + os.path.basename(path))
    interp = Interpreter(draw=draw)
    try:
        interp.run(source, filename=path)
    except CmcStopped:
        print("Stopped!")
        return 1
    except CmcError as error:
        print(error.format(), file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nStopped!")
        return 1
    draw.hold_open()
    return 0


def cmd_compile(path, out_path=None):
    source = _read_file(path)
    if source is None:
        return 1
    try:
        python_source = to_python_source(source, filename=path)
    except CmcError as error:
        print(error.format(), file=sys.stderr)
        return 1
    if out_path is None:
        base = os.path.splitext(path)[0]
        out_path = base + ".py"
    try:
        with open(out_path, "w", encoding="utf-8") as handle:
            handle.write(python_source)
    except OSError as exc:
        print("OOGA! I could not write '" + out_path + "': " + str(exc))
        return 1
    print("Made " + out_path + " from " + path)
    print("Run it with: python " + out_path)
    return 0


def cmd_init(path=None):
    if path is None:
        path = "hello.cmc"
    if os.path.exists(path):
        print("'" + path + "' already exists, so I left it alone.")
        return 1
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(STARTER)
    print("Made " + path + " -- have a look and run it:")
    print("  cmc run " + path)
    return 0


def cmd_examples():
    folder = _examples_dir()
    if folder is None:
        print("I could not find the examples folder in this copy of CMC.")
        return 1
    names = sorted(f for f in os.listdir(folder) if f.endswith(".cmc"))
    print("Example caves to explore (" + str(len(names)) + "):")
    for name in names:
        print("  " + os.path.join(folder, name))
    print("\nRun one with: cmc run " + os.path.join(folder, names[0] if names else "hello.cmc"))
    return 0


def _block_depth(buffer):
    try:
        tokens = tokenize("\n".join(buffer))
    except CmcError:
        return 0
    depth = 0
    for i, tok in enumerate(tokens):
        if tok.kind != "kw":
            continue
        if tok.value in BLOCK_OPENERS:
            depth += 1
        elif tok.value == "unga":
            depth -= 1
        elif tok.value == "wonga" and i + 1 < len(tokens):
            nxt = tokens[i + 1]
            if nxt.kind == "kw" and nxt.value == "binga":
                depth += 1
    return depth


def cmd_repl():
    print("Cave Man Code (CMC) v" + VERSION + " -- talk to me!")
    print('Try: oga "hello cave"   (type quit when you are done)')
    interp = Interpreter(output=lambda line: print(line))
    buffer = []

    while True:
        prompt = "oga> " if not buffer else "...> "
        try:
            line = input(prompt)
        except EOFError:
            print()
            break
        except KeyboardInterrupt:
            print()
            buffer = []
            continue
        if not buffer and line.strip().lower() in ("quit", "exit", "bye"):
            break
        if not line.strip() and not buffer:
            continue
        buffer.append(line)
        if _block_depth(buffer) > 0:
            continue
        source = "\n".join(buffer)
        buffer = []
        try:
            interp.run(source, filename="<talk>")
        except CmcStopped:
            print("Stopped!")
        except CmcError as error:
            print(error.format())
        except KeyboardInterrupt:
            print("Stopped!")
    print("Bye bye, cave friend!")
    return 0


def main(argv=None):
    _setup_console()
    args = list(sys.argv[1:] if argv is None else argv)

    if not args:
        return cmd_repl()

    command = args[0].lower()

    if command in ("help", "-h", "--help", "?"):
        print(HELP.format(version=VERSION))
        return 0

    if command in ("version", "-v", "--version"):
        print("Cave Man Code (CMC) v" + VERSION)
        return 0

    if command == "run":
        if len(args) < 2:
            print("Which program should I run? Try: cmc run hello.cmc")
            return 1
        return cmd_run(args[1])

    if command == "compile":
        if len(args) < 2:
            print("Which program should I compile? Try: cmc compile hello.cmc")
            return 1
        out = args[2] if len(args) > 2 else None
        return cmd_compile(args[1], out)

    if command == "init":
        return cmd_init(args[1] if len(args) > 1 else None)

    if command == "examples":
        return cmd_examples()

    # Maybe they just typed a file name.
    if command.endswith(".cmc"):
        return cmd_run(args[0])

    print("OOGA! I do not know the command '" + args[0] + "'.")
    print("Try: cmc help")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())