# Publishing Cave Man Code to GitHub (complete guide)

Follow this once and your language will have a home on the internet: a GitHub
repository, a live wiki website, and downloadable zips.

Time needed: about 15 minutes, plus a little waiting for uploads.

---

## 0. What you need

| Thing | Why | Check |
| --- | --- | --- |
| A GitHub account | the home for the project | github.com/join |
| Git | uploads your files | `git --version` |
| GitHub CLI (`gh`) | creates the repo in one command | `gh --version` |
| Windows 10+ | builds docs, site, zips | `python --version` |

Install GitHub CLI: `winget install GitHub.cli` (Windows),
`brew install gh` (macOS), or see cli.github.com for Linux.

Everything below is typed in the `caveman-code` folder unless it says
otherwise. On Windows, use **PowerShell**.

---

## 1. Get the project ready

### 1.1 Check the tests pass

```text
powershell -File native\tests\run_tests.ps1
```

You should see `OK`. If not, fix that first -- a repo that fails its own tests
is a sad cave.

### 1.2 Build the docs and website

```text
python tools/write_docs.py
python tools/build_site.py --check
```

`--check` verifies every internal link in the website. It should say
`Link check: all internal links look good.`

### 1.3 Build the download zips

```text
powershell -File tools/build_release.ps1
```

This writes zips into `dist/` **and** into `website/downloads/`, so the
website's Download page has real files to hand out.

---

## 2. Enter the identity you want on the commits

If you have never used git on this machine:

```text
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

If you want to keep your email private, GitHub has a special address:
**Settings -> Emails -> Keep my email addresses private** shows something like
`12345678+yourname@users.noreply.github.com`. Use that.

---

## 3. Make the first commit

```text
git init
git add .
git commit -m "Cave Man Code v0.1.0: the first fire"
```

Check what will be uploaded:

```text
git status
git ls-files | wc -l        (PowerShell: (git ls-files).Count)
```

You should **not** see `__pycache__/`, `.venv/`, or anything secret. The
`.gitignore` handles that. If you ever see a password or key in `git status`,
stop and fix it before committing.

---

## 4. Log in to GitHub

```text
gh auth login
```

Answer the questions like this:

```text
What account do you want to log into?      GitHub.com
What is your preferred protocol?           HTTPS
Authenticate Git with your GitHub credentials?  Yes
How would you like to authenticate?        Login with a web browser
```

Copy the one-time code, press Enter, paste it in the browser, and approve.
When it says "Logged in as YOUR-NAME", you are ready.

Check anytime with `gh auth status`.

---

## 5. Create the repository and push

One command creates the repo, sets the remote, and pushes:

```text
gh repo create caveman-code --public --source . --push
```

- `--public` means everyone can see it. Use `--private` for a first draft.
- A private repo can still use GitHub Pages on paid plans; on the free plan,
  Pages needs a public repo (or use `--public` when you are ready).

Your repo now exists at:

```text
https://github.com/YOUR-NAME/caveman-code
```

Open it in a browser to admire the README. Future uploads are just:

```text
git add .
git commit -m "what changed"
git push
```

---

## 6. No `gh`? Do it with the website instead

1. On github.com, click **+ -> New repository**.
2. Name: `caveman-code`. Visibility: **Public**. Do **not** tick "Add a
   README" (you already have one).
3. Click **Create repository**. GitHub shows a page with commands.
4. Copy the two lines under *"…or push an existing repository from the command
   line"*:

```text
git remote add origin https://github.com/YOUR-NAME/caveman-code.git
git branch -M main
git push -u origin main
```

5. When git asks for a password, use a **Personal Access Token**, not your
   account password: GitHub -> Settings -> Developer settings ->
   Personal access tokens -> Tokens (classic) -> Generate new token ->
   tick **repo** -> copy the token and paste it as the password.

### Prefer SSH keys?

```text
ssh-keygen -t ed25519 -C "you@example.com"
# press Enter three times, then:
cat ~/.ssh/id_ed25519.pub      (Windows: type $env:USERPROFILE\.ssh\id_ed25519.pub)
```

Paste that public key into GitHub -> Settings -> SSH and GPG keys -> New SSH
key. Then use the SSH remote URL:

```text
git remote set-url origin git@github.com:YOUR-NAME/caveman-code.git
```

---

## 7. Turn on the website (GitHub Pages)

1. Open the repo on GitHub.
2. **Settings** (top bar) -> **Pages** (left sidebar).
3. Under *Build and deployment*: Source = **Deploy from a branch**.
4. Branch: **main**, folder: **/website**. Click **Save**.
5. Wait a minute or two, then visit:

```text
https://YOUR-NAME.github.io/caveman-code/
```

That URL now shows `website/index.html` (the main page), and the wiki is at
`/wiki/index.html`. Every time you push changes inside `website/`, the site
updates automatically.

### Optional: a custom domain

Buy a domain (for example `cavemancode.fun`), then:

1. Settings -> Pages -> Custom domain -> type the domain -> Save.
2. At your domain registrar, add a CNAME record pointing `www` to
   `YOUR-NAME.github.io`, or the four A records GitHub shows for the apex
   domain.
3. Tick **Enforce HTTPS** once the certificate is issued.

---

## 8. Publish the first release (the download page files)

### 8.1 Tag the version

```text
git tag v0.1.0
git push origin v0.1.0
```

### 8.2 Create the release with the zip files attached

```text
gh release create v0.1.0 --title "Cave Man Code v0.1.0 - the first fire" ^
  --notes "The first release: compiler, IDE, 31 examples, full wiki and docs." ^
  dist/*.zip
```

(On macOS/Linux use `\` instead of `^` at the end of the line.)

Now `https://github.com/YOUR-NAME/caveman-code/releases` has your zips, and
each release gets its own download URL that never changes for that version.

### 8.3 Point the website at the release (optional but nice)

`website/download.html` currently links to `downloads/…zip` inside the repo,
which works fine. If you would rather use release links, edit
`tools/build_site.py` (search for `downloads/`) to use:

```text
https://github.com/YOUR-NAME/caveman-code/releases/latest/download/CMC-Compiler-v0.1.0.zip
```

Then rebuild the site and push.

---

## 9. Optional: let GitHub build things for you

Automatic site rebuild + release on every tag. Create
`.github/workflows/release.yml`:

```yaml
name: release

on:
  push:
    tags: ["v*"]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
        with:
          python-version: "3.12"
      - name: Tests
        run: powershell -File native\tests\run_tests.ps1
      - name: Docs and site
        run: |
          python tools/write_docs.py
          python tools/build_site.py --check
      - name: Release zips
        run: |
          python tools/build_release.py
      - uses: softprops/action-gh-release@v2
        with:
          files: dist/*.zip
```

Note: `tools/build_release.ps1` is PowerShell. For the Linux runner, a tiny
`tools/build_release.py` equivalent is easy to add -- or run the workflow on
`windows-latest` instead.

---

## 10. Everyday life after publishing

### Making a change

```text
# edit files...
powershell -File native\tests\run_tests.ps1
python tools/build_site.py --check
git add .
git commit -m "Add a new example about boats"
git push
```

### Bumping the version

The version lives in exactly one place: `src/cmc/words.py` (`VERSION =
"0.1.0"`). Everything else reads it. To release `0.2.0`:

1. Change `VERSION` in `src/cmc/words.py`, and the version in
   `pyproject.toml`.
2. Add a section to `site_content/pages/changelog.md`.
3. Run:

```text
python tools/write_docs.py
python tools/build_site.py --check
powershell -File tools/build_release.ps1
git add .
git commit -m "Version 0.2.0"
git push
git tag v0.2.0
git push origin v0.2.0
gh release create v0.2.0 --title "Cave Man Code v0.2.0" --notes "See the changelog." dist/*.zip
```

### Getting found

- Add topics on the repo page: `programming-language`, `education`, `kids`,
  `beginner`, `python`, `interpreter`, `scratch-like`.
- Write a good one-paragraph description in **About**.
- Pin the repo to your profile.
- Post the live wiki link when you tell people about it.

### Taking care of people

- Watch the Issues tab. Answer beginners with the same kindness CMC's error
  messages use.
- "good first issue" labels attract helpers.
- Thank every contributor in the changelog.

---

## 11. Troubleshooting

| Problem | Fix |
| --- | --- |
| `gh: not logged in` | run `gh auth login` again |
| `remote origin already exists` | `git remote set-url origin <url>` |
| push rejected, non-fast-forward | `git pull --rebase` then push |
| Pages shows 404 | wait 2 minutes; check Settings -> Pages branch/folder; make sure `website/index.html` exists in the repo |
| Site updates but download links 404 | the zips are in `website/downloads/`; rebuild with `build_release.ps1` and commit them |
| Accidentally committed a secret | change the secret immediately, then remove it from history (`git filter-repo`) -- deleting the file is not enough |
| `src refspec main does not match any` | you have no commit yet: `git add .` and `git commit` first |
| Wrong author on commits | fix `git config user.name/user.email`, then `git commit --amend --reset-author` |

---

## 12. The checklist

- [ ] Tests pass
- [ ] `write_docs.py` and `build_site.py --check` pass
- [ ] `build_release.ps1` made zips in `dist/` and `website/downloads/`
- [ ] `git init`, first commit
- [ ] `gh auth login`
- [ ] `gh repo create caveman-code --public --source . --push`
- [ ] Pages enabled from `/website`
- [ ] Tag `v0.1.0` pushed
- [ ] Release created with zips
- [ ] Repo description and topics filled in
- [ ] You told one human about it

Welcome to open source, cave publisher. Ugg is proud of you.