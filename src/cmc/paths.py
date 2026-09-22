"""Finding the examples folder, whether CMC is installed or run from the repo."""

import os


def repo_root():
    """Walk up from this file looking for the repo (the folder with examples/)."""
    here = os.path.dirname(os.path.abspath(__file__))
    for _ in range(5):
        if os.path.isdir(os.path.join(here, "examples")):
            return here
        parent = os.path.dirname(here)
        if parent == here:
            break
        here = parent
    return None


def examples_dir():
    root = repo_root()
    if root is None:
        return None
    folder = os.path.join(root, "examples")
    return folder if os.path.isdir(folder) else None


def example_files():
    folder = examples_dir()
    if folder is None:
        return []
    return sorted(
        os.path.join(folder, name)
        for name in os.listdir(folder)
        if name.endswith(".cmc")
    )


def website_dir():
    root = repo_root()
    if root is None:
        return None
    folder = os.path.join(root, "website")
    return folder if os.path.isdir(folder) else None


def docs_dir():
    root = repo_root()
    if root is None:
        return None
    folder = os.path.join(root, "docs")
    return folder if os.path.isdir(folder) else None