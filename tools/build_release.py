"""Build the download zips (and copy them into website/downloads/).

    python tools/build_release.py

Makes three zips in dist/:
    CaveManCode-vX.Y.Z.zip      everything (the recommended download)
    CMC-Compiler-vX.Y.Z.zip     compiler + examples + docs
    OGABOOGA-CODER-vX.Y.Z.zip   the full app package (IDE + compiler)
"""

import os
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "src"))

from cmc.words import VERSION  # noqa: E402

DIST = os.path.join(ROOT, "dist")
STAGE = os.path.join(DIST, "_stage")
WEB_DOWNLOADS = os.path.join(ROOT, "website", "downloads")
PACKAGE = "caveman-code-" + VERSION

COMMON_FILES = [
    "README.md",
    "LICENSE",
    ".gitignore",
    "CHEATSHEET.md",
    "pyproject.toml",
    "run_ogabooga.bat",
    "cmc_run.bat",
]

FULL_DIRS = ["src", "examples", "docs", "tests", "tools", "site_content", "website"]
COMPILER_DIRS = ["src/cmc", "examples", "docs", "website"]
IDE_DIRS = ["src", "examples", "docs", "website"]


def _copy_file(relative, stage_root):
    source = os.path.join(ROOT, relative)
    if not os.path.exists(source):
        return
    target = os.path.join(stage_root, relative)
    os.makedirs(os.path.dirname(target), exist_ok=True)
    shutil.copy2(source, target)


def _copy_dir(relative, stage_root):
    source = os.path.join(ROOT, relative)
    target = os.path.join(stage_root, relative)
    if not os.path.isdir(source):
        return

    def ignore(_path, names):
        blocked = set()
        for name in names:
            if name in ("__pycache__", ".pytest_cache"):
                blocked.add(name)
            if name.endswith((".pyc", ".pyo")):
                blocked.add(name)
            if name == "downloads" and _path.endswith("website"):
                blocked.add(name)
        return blocked

    shutil.copytree(source, target, ignore=ignore, dirs_exist_ok=True)


def _stage_package(name, dirs):
    stage_root = os.path.join(STAGE, name)
    if os.path.isdir(stage_root):
        shutil.rmtree(stage_root)
    os.makedirs(stage_root, exist_ok=True)
    for relative in COMMON_FILES:
        _copy_file(relative, stage_root)
    for relative in dirs:
        _copy_dir(relative, stage_root)
    with open(os.path.join(stage_root, "START_HERE.txt"), "w", encoding="utf-8") as handle:
        handle.write(
            "Cave Man Code (CMC) v" + VERSION + "\n"
            "===================================\n\n"
            "To start the IDE (OGABOOGA CODER):\n"
            "  Windows: double-click run_ogabooga.bat\n"
            "  macOS/Linux:\n"
            "    PYTHONPATH=src python3 -m ogabooga\n\n"
            "To run a program in a terminal:\n"
            "  Windows: drag a .cmc file onto cmc_run.bat\n"
            "  macOS/Linux:\n"
            "    PYTHONPATH=src python3 -m cmc run examples/01_hello_oga.cmc\n\n"
            "Python 3.9 or newer is the only requirement.\n"
            "Docs: open docs/00_INDEX.md, or website/index.html in a browser.\n"
        )
    return stage_root


def _zip_folder(stage_root, zip_path):
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as archive:
        for base, _dirs, files in os.walk(stage_root):
            for name in files:
                full = os.path.join(base, name)
                inside = os.path.relpath(full, os.path.dirname(stage_root))
                archive.write(full, inside)
    return zip_path


def build():
    os.makedirs(DIST, exist_ok=True)
    os.makedirs(WEB_DOWNLOADS, exist_ok=True)

    packages = [
        ("CaveManCode-v" + VERSION, FULL_DIRS),
        ("CMC-Compiler-v" + VERSION, COMPILER_DIRS),
        ("OGABOOGA-CODER-v" + VERSION, IDE_DIRS),
    ]

    made = []
    for name, dirs in packages:
        stage_root = _stage_package(name, dirs)
        zip_path = os.path.join(DIST, name + ".zip")
        _zip_folder(stage_root, zip_path)
        made.append(zip_path)
        print("built " + os.path.relpath(zip_path, ROOT))

    for zip_path in made:
        shutil.copy2(zip_path, os.path.join(WEB_DOWNLOADS, os.path.basename(zip_path)))
    shutil.rmtree(STAGE, ignore_errors=True)

    print("")
    print("Copied " + str(len(made)) + " zip(s) into website/downloads/")
    print("Now run: python tools/build_site.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(build())