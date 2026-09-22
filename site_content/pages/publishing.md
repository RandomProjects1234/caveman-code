# Publishing CMC to GitHub

This page is a short version. The complete, copy-paste guide lives in the repo
at `docs/09_PUBLISHING_GITHUB.md`.

## The short version (10 minutes)

### 1. Make a GitHub account and log in

Install GitHub CLI (`gh`) from cli.github.com, then:

```text
gh auth login
```

Choose **GitHub.com**, **HTTPS**, and log in through the browser.

### 2. Start the repo

From the `caveman-code` folder:

```text
git init
git add .
git commit -m "Cave Man Code v0.1.0: the first fire"
```

### 3. Create the repo and push in one command

```text
gh repo create caveman-code --public --source . --push
```

Your code is now at `https://github.com/YOUR-NAME/caveman-code`.

### 4. Turn on the website (GitHub Pages)

1. Open the repo on GitHub.
2. **Settings** -> **Pages**.
3. Source: **Deploy from a branch**.
4. Branch: `main`, folder: `/website`.
5. Save. In a minute your wiki is live at
   `https://YOUR-NAME.github.io/caveman-code/`.

### 5. Make a release with the downloads

```text
powershell -File tools/build_release.ps1
git add website/downloads
git commit -m "Add v0.1.0 download zips"
git push
git tag v0.1.0
git push origin v0.1.0
```

Create the release on GitHub with your zips attached:

```text
gh release create v0.1.0 --title "Cave Man Code v0.1.0" --notes "The first fire!" dist/*.zip
```

## What is in this repo

```text
caveman-code/
  README.md            the front door
  LICENSE              MIT
  CHEATSHEET.md        one page reference
  src/cmc/             the compiler
  src/ogabooga/        the IDE
  examples/            31 example programs
  tests/               the test suite
  docs/                the full manual
  site_content/        wiki words (markdown)
  website/             the built website (Pages serves this)
  tools/               site and release builders
```

## Before you publish

- Run the tests: `python -m unittest discover -s tests -t .`
- Build the site: `python tools/build_site.py --check`
- Build the zips: `powershell -File tools/build_release.ps1`
- Check the license and your name in `pyproject.toml`.

## After publishing

- Add the real GitHub link to the website top bar (`site_content` then rebuild).
- Point the download page buttons at your GitHub Releases if you prefer
  releases over repo files.
- Watch the issues tab and answer with kindness.

Full details, including SSH setup, GitHub Actions, version bumps, and how to
update a release, are in `docs/09_PUBLISHING_GITHUB.md`.