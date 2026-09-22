"""Entry point for the PyInstaller build of the cmc command."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src"))

from cmc.cli import main  # noqa: E402

if __name__ == "__main__":
    raise SystemExit(main())