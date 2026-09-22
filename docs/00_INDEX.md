# CMC docs -- start here

This folder is the manual for Cave Man Code (CMC) v0.1.0. The website in
`website/` has the same material in a friendlier shape, plus a page for every
word.

## Reading order for a new coder

1. `02_TUTORIAL_30_MINUTES.md` -- the whole language in half an hour
2. `../CHEATSHEET.md` -- one page to keep nearby
3. `03_KEYWORD_DICTIONARY.md` -- every word explained
4. `08_EXAMPLES_GALLERY.md` -- 31 programs with their output
5. `05_OGABOOGA_CODER_MANUAL.md` -- the IDE manual
6. `06_BLOCKS_GUIDE.md` -- the drag-and-drop workshop

## For adults and teachers

- `01_LANGUAGE_SPEC.md` -- the complete formal specification
- `04_ERROR_MESSAGES.md` -- every error with its cause and fix
- `07_DRAWING_GUIDE.md` -- the drawing system
- `10_CONTRIBUTING.md` -- how to help
- `09_PUBLISHING_GITHUB.md` -- how to put this project on GitHub
- `11_ROADMAP_CHANGELOG.md` -- where CMC is going, and where it has been
- `12_GLOSSARY.md` -- caveman to human translation
- `13_FAQ.md` -- questions people actually ask

## The one minute summary

CMC is a tiny, real programming language for first-time coders:

```cmc
oga "hello cave"

grunk rocks = 3
booga rocks
    oga "bang!", lap
unga
```

The compiler lives in `src/cmc/`, the IDE in `src/ogabooga/`, examples in
`examples/`, and the tests in `tests/`.

```text
python -m unittest discover -s tests -t .     run everything
python -m ogabooga                            open the IDE
python -m cmc run examples/15_fizzbuzz.cmc    run an example
python tools/build_site.py --check            rebuild the website
python tools/build_release.ps1                make the download zips
```