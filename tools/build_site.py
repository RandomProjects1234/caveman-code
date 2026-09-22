"""Build the CMC website (wiki + download page + main page) with no tools but Python.

Usage:
    python tools/build_site.py            build the site into website/
    python tools/build_site.py --check    also check every internal link
"""

import html
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "src"))

from cmc import words  # noqa: E402
from cmc.paths import examples_dir  # noqa: E402

CONTENT = os.path.join(ROOT, "site_content", "pages")
SITE = os.path.join(ROOT, "website")
WIKI = os.path.join(SITE, "wiki")
WORD_DIR = os.path.join(WIKI, "words")
DOWNLOADS = os.path.join(SITE, "downloads")

VERSION = words.VERSION


# ----------------------------------------------------------------- markdown

LINK_RE = re.compile(r"\[([^\]]+)\]\(([^)\s]+)\)")
IMG_RE = re.compile(r"!\[([^\]]*)\]\(([^)\s]+)\)")
CODE_RE = re.compile(r"`([^`]+)`")
BOLD_RE = re.compile(r"\*\*([^*]+)\*\*")
ITALIC_RE = re.compile(r"(?<!\*)\*([^*]+)\*(?!\*)")


def _rewrite(target, link_prefix):
    if target.startswith(("http://", "https://", "mailto:", "#", "data:")):
        return target
    if target.endswith(".md"):
        target = target[:-3] + ".html"
    if not link_prefix:
        return target
    if target.startswith((link_prefix, "assets/", "downloads/")):
        return target
    if target in ("download.html", "index.html"):
        return target
    return link_prefix + target


def _md_link(match):
    text, target = match.group(1), match.group(2)
    return '<a href="' + _rewrite(target, _LINK_PREFIX[0]) + '">' + text + "</a>"


def inline(text):
    text = html.escape(text, quote=False)
    text = IMG_RE.sub(lambda m: '<img src="' + m.group(2) + '" alt="' + m.group(1) + '">', text)
    text = CODE_RE.sub(lambda m: "<code>" + m.group(1) + "</code>", text)
    text = BOLD_RE.sub(lambda m: "<strong>" + m.group(1) + "</strong>", text)
    text = ITALIC_RE.sub(lambda m: "<em>" + m.group(1) + "</em>", text)
    text = LINK_RE.sub(_md_link, text)
    return text


_LINK_PREFIX = [""]


def md_to_html(markdown, link_prefix=""):
    _LINK_PREFIX[0] = link_prefix
    try:
        return _md_to_html(markdown)
    finally:
        _LINK_PREFIX[0] = ""


def _md_to_html(markdown):
    lines = markdown.replace("\r\n", "\n").split("\n")
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        if stripped.startswith("```"):
            lang = stripped[3:].strip()
            i += 1
            code_lines = []
            while i < len(lines) and not lines[i].strip().startswith("```"):
                code_lines.append(lines[i])
                i += 1
            i += 1
            out.append(
                '<pre><code class="lang-' + (lang or "cmc") + '">'
                + html.escape("\n".join(code_lines)) + "</code></pre>"
            )
            continue

        if not stripped:
            i += 1
            continue

        heading = re.match(r"^(#{1,6})\s+(.*)$", stripped)
        if heading:
            level = len(heading.group(1))
            out.append("<h" + str(level) + ">" + inline(heading.group(2)) + "</h" + str(level) + ">")
            i += 1
            continue

        if stripped in ("---", "***", "___"):
            out.append("<hr>")
            i += 1
            continue

        if stripped.startswith(">"):
            quote = []
            while i < len(lines) and lines[i].strip().startswith(">"):
                quote.append(lines[i].strip()[1:].strip())
                i += 1
            out.append("<blockquote>" + _md_to_html("\n".join(quote)) + "</blockquote>")
            continue

        if re.match(r"^[-*]\s+", stripped):
            items = []
            while i < len(lines) and re.match(r"^[-*]\s+", lines[i].strip()):
                items.append(lines[i].strip()[2:])
                i += 1
            out.append("<ul>" + "".join("<li>" + inline(item) + "</li>" for item in items) + "</ul>")
            continue

        if re.match(r"^\d+\.\s+", stripped):
            items = []
            while i < len(lines) and re.match(r"^\d+\.\s+", lines[i].strip()):
                items.append(re.sub(r"^\d+\.\s+", "", lines[i].strip()))
                i += 1
            out.append("<ol>" + "".join("<li>" + inline(item) + "</li>" for item in items) + "</ol>")
            continue

        if stripped.startswith("|") and i + 1 < len(lines) and re.match(r"^\|[\s:\-|]+\|$", lines[i + 1].strip()):
            rows = []
            header = [c.strip() for c in stripped.strip("|").split("|")]
            i += 2
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            table = ["<table><thead><tr>"]
            table += ["<th>" + inline(c) + "</th>" for c in header]
            table.append("</tr></thead><tbody>")
            for row in rows:
                table.append("<tr>" + "".join("<td>" + inline(c) + "</td>" for c in row) + "</tr>")
            table.append("</tbody></table>")
            out.append("".join(table))
            continue

        paragraph = [stripped]
        i += 1
        while i < len(lines):
            nxt = lines[i].strip()
            if not nxt or nxt.startswith(("#", "```", ">", "|", "-", "*")) or re.match(r"^\d+\.\s+", nxt):
                break
            paragraph.append(nxt)
            i += 1
        out.append("<p>" + inline(" ".join(paragraph)) + "</p>")
    return "\n".join(out)


# -------------------------------------------------------------------- pages

def read_pages():
    pages = {}
    for name in sorted(os.listdir(CONTENT)):
        if not name.endswith(".md"):
            continue
        slug = name[:-3]
        with open(os.path.join(CONTENT, name), "r", encoding="utf-8") as handle:
            text = handle.read()
        title = slug
        body = text
        match = re.match(r"^#\s+(.*)$", text.strip().split("\n")[0])
        if match:
            title = match.group(1).strip()
            body = "\n".join(text.strip().split("\n")[1:])
        pages[slug] = {"slug": slug, "title": title, "body": body, "html": md_to_html(body)}
    return pages


def load_site_json():
    path = os.path.join(ROOT, "site_content", "site.json")
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def nav_html(pages, site, rel, current):
    parts = ['<nav class="sidebar">']
    for section in site["sections"]:
        parts.append('<div class="nav-section">' + html.escape(section["title"]) + "</div>")
        parts.append("<ul>")
        for slug in section["pages"]:
            if slug == "words":
                href = rel + "wiki/words.html"
                title = "All words A-Z"
                active = ' class="active"' if current == "words" else ""
                parts.append("<li><a" + active + ' href="' + href + '">' + title + "</a></li>")
                for word in words.all_words():
                    pass
                continue
            page = pages.get(slug)
            if page is None:
                continue
            active = ' class="active"' if current == slug else ""
            parts.append(
                "<li><a" + active + ' href="' + rel + "wiki/" + slug + '.html">'
                + html.escape(page["title"]) + "</a></li>"
            )
        parts.append("</ul>")
    parts.append("</nav>")
    return "\n".join(parts)


def page_shell(title, body, rel, nav, description="Cave Man Code (CMC)"):
    return """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title} - Cave Man Code</title>
<meta name="description" content="{description}">
<link rel="stylesheet" href="{rel}style.css">
<link rel="icon" href="{rel}assets/logo.svg" type="image/svg+xml">
</head>
<body>
<header class="topbar">
  <a class="brand" href="{rel}index.html">
    <img src="{rel}assets/logo.svg" alt="CMC logo" width="40" height="40">
    <span>Cave Man Code</span>
  </a>
  <div class="searchbox">
    <input id="search" type="search" placeholder="Search the wiki... (try grunk)" autocomplete="off">
    <div id="search-results" class="search-results"></div>
  </div>
  <nav class="topnav">
    <a href="{rel}index.html">Home</a>
    <a href="{rel}wiki/index.html">Wiki</a>
    <a href="{rel}download.html">Download</a>
    <a href="https://github.com/" target="_blank" rel="noopener">GitHub</a>
  </nav>
</header>
<div class="layout">
{nav}
<main class="content">
{body}
</main>
</div>
<footer class="footer">
  <span>Cave Man Code (CMC) v{version} - MIT License - made for tiny hands and big ideas.</span>
  <span>Ugg say: small steps, big rocks.</span>
</footer>
<script src="{rel}app.js"></script>
</body>
</html>
""".format(title=html.escape(title), body=body, rel=rel, nav=nav, version=VERSION,
           description=html.escape(description))


# ------------------------------------------------------------------- words

def word_page_html(word):
    name = word.get("display", word["name"])
    parts = ['<h1>' + html.escape(name) + "</h1>"]
    if word.get("aliases"):
        parts.append('<p class="aka">also written: ' + ", ".join(word["aliases"]) + "</p>")
    parts.append('<p class="chip">' + html.escape(word.get("category", "")) + "</p>")
    parts.append("<p><strong>" + inline(word.get("summary", "")) + "</strong></p>")
    parts.append("<h2>How to write it</h2>")
    parts.append("<pre><code>" + html.escape(word.get("syntax", "")) + "</code></pre>")
    for extra in word.get("extra_syntax", []):
        parts.append("<pre><code>" + html.escape(extra) + "</code></pre>")
    parts.append("<h2>What it does</h2>")
    parts.append(md_to_html(word.get("doc", "")))
    parts.append("<h2>Example</h2>")
    parts.append("<pre><code>" + html.escape(word.get("example", "")) + "</code></pre>")
    if word.get("tips"):
        parts.append("<h2>Tips</h2>")
        parts.append("<ul>" + "".join("<li>" + inline(t) + "</li>" for t in word["tips"]) + "</ul>")
    if word.get("related"):
        links = []
        for related in word["related"]:
            info = words.word_info(related)
            if info:
                label = info.get("display", info["name"])
                links.append('<a href="' + info["name"] + '.html">' + html.escape(label) + "</a>")
            else:
                links.append(html.escape(related))
        parts.append("<h2>See also</h2><p>" + " &middot; ".join(links) + "</p>")
    parts.append('<p class="back"><a href="../words.html">Back to all words</a></p>')
    return "\n".join(parts)


def build_word_pages(pages, site):
    os.makedirs(WORD_DIR, exist_ok=True)
    by_category = {}
    for word in words.all_words():
        by_category.setdefault(word["category"], []).append(word)

    index_parts = ["<h1>All the CMC words</h1>",
                   "<p>Every word in the language, one page each. Start with the ones in "
                   "<strong>Talking</strong> and <strong>Boxes</strong>.</p>"]
    for category in words.CATEGORIES:
        if category not in by_category:
            continue
        index_parts.append("<h2>" + html.escape(category) + "</h2><ul class='wordlist'>")
        for word in sorted(by_category[category], key=lambda w: w["name"]):
            name = word.get("display", word["name"])
            alias = (" <span class='dim'>(" + word["aliases"][0] + ")</span>") if word.get("aliases") else ""
            index_parts.append(
                "<li><a href='words/" + word["name"] + ".html'>" + html.escape(name) + "</a>"
                + alias + " - " + inline(word.get("summary", "")) + "</li>"
            )
        index_parts.append("</ul>")

    nav = nav_html(pages, site, "../", "words")
    with open(os.path.join(WIKI, "words.html"), "w", encoding="utf-8") as handle:
        handle.write(page_shell("All words A-Z", "\n".join(index_parts), "../", nav))

    for word in words.all_words():
        nav = nav_html(pages, site, "../../", "word-" + word["name"])
        body = word_page_html(word)
        with open(os.path.join(WORD_DIR, word["name"] + ".html"), "w", encoding="utf-8") as handle:
            handle.write(page_shell(word.get("display", word["name"]), body, "../../", nav))


# --------------------------------------------------------------- download

def build_download_page(pages, site):
    rel = ""
    nav = nav_html(pages, site, rel, "download")
    zips = []
    if os.path.isdir(DOWNLOADS):
        zips = sorted(f for f in os.listdir(DOWNLOADS) if f.endswith(".zip"))

    cards = []
    for name in zips:
        pretty = name.replace("_", " ").replace(".zip", "")
        cards.append(
            '<div class="card"><h3>' + html.escape(pretty) + "</h3>"
            '<p>Ready to run. Unzip it anywhere and double-click the launcher.</p>'
            '<a class="button" href="downloads/' + name + '" download>Download ' + html.escape(name) + "</a></div>"
        )
    if not cards:
        cards.append(
            '<div class="card"><h3>Builds not generated yet</h3>'
            "<p>Run <code>powershell -File tools/build_release.ps1</code> in the repo to make "
            "the zips, then build the site again.</p></div>"
        )

    body = """
<h1>Download Cave Man Code</h1>
<p>CMC is free, tiny, and runs on any computer with Python 3.9 or newer.
Everything you need is in one zip: the compiler, OGABOOGA CODER, all examples, and the docs.</p>
<div class="cards">
{cards}
</div>
<h2>How to install (Windows)</h2>
<pre><code>1. Download a zip above and right-click -> Extract All...
2. Open the extracted folder.
3. Double-click run_ogabooga.bat  (or cmc_run.bat to run programs in a terminal)</code></pre>
<h2>How to install (macOS / Linux)</h2>
<pre><code>unzip CMC-Compiler-v{version}.zip
cd caveman-code
PYTHONPATH=src python3 -m cmc run examples/01_hello_oga.cmc
PYTHONPATH=src python3 -m ogabooga</code></pre>
<h2>What is inside</h2>
<table>
<thead><tr><th>File</th><th>What it does</th></tr></thead>
<tbody>
<tr><td>src/cmc/</td><td>the compiler (lexer, parser, interpreter, Python generator)</td></tr>
<tr><td>src/ogabooga/</td><td>OGABOOGA CODER, the IDE with the compiler inside</td></tr>
<tr><td>run_ogabooga.bat</td><td>starts the IDE on Windows</td></tr>
<tr><td>cmc_run.bat</td><td>drag any .cmc file onto it to run it</td></tr>
<tr><td>examples/</td><td>{count} example programs from hello world to recursion</td></tr>
<tr><td>docs/</td><td>the full manual</td></tr>
</tbody>
</table>
<h2>Make your own .exe (optional)</h2>
<p>If you want a stand-alone <code>OgaboogaCoder.exe</code> with no Python installed,
use the included PyInstaller script:</p>
<pre><code>pip install pyinstaller
powershell -File tools/build_exe.ps1</code></pre>
<h2>System needed</h2>
<ul>
<li>Python 3.9 or newer (tkinter comes with the normal installer)</li>
<li>Windows, macOS, or Linux</li>
<li>About 5 MB of space, no internet needed after download</li>
</ul>
""".format(cards="\n".join(cards), version=VERSION, count=_example_count())
    with open(os.path.join(SITE, "download.html"), "w", encoding="utf-8") as handle:
        handle.write(page_shell("Download", body, rel, nav))


def _example_count():
    folder = examples_dir()
    if not folder:
        return "30+"
    return str(len([f for f in os.listdir(folder) if f.endswith(".cmc")]))


# ----------------------------------------------------------------- landing

def build_landing(pages, site):
    home = pages.get("index")
    home_html = md_to_html(home["body"], link_prefix="wiki/") if home else ""
    nav = nav_html(pages, site, "", "index")
    body = """
<section class="hero">
  <div class="hero-text">
    <h1>Talk caveman. Make computer do things.</h1>
    <p class="lead">Cave Man Code (CMC) is a real programming language a 4 year old can
    start using in 10 minutes, with enough power underneath to grow for years.
    Drag blocks or type words like <code>oga</code>, <code>grunk</code>, and <code>unga</code>.</p>
    <p>
      <a class="button big" href="download.html">Download CMC free</a>
      <a class="button ghost" href="wiki/getting-started.html">Start the wiki</a>
    </p>
  </div>
  <pre class="hero-code"><code>ugg your first program
oga "Ooga booga!"

grunk rocks = 3
booga rocks
    oga "bang rock", lap
unga

skrib circle 200, 200, 100</code></pre>
</section>
{navigation_cards}
<section class="homewiki">
{home}
</section>
""".format(home=home_html, navigation_cards="""
<section class="cards">
  <div class="card"><h3>Learn in 30 minutes</h3>
    <p>One tutorial takes you from nothing to loops, clumps, and piles.</p>
    <a class="button ghost" href="wiki/tutorial.html">Open tutorial</a></div>
  <div class="card"><h3>Big wiki</h3>
    <p>Every word has its own page, with examples and tips.</p>
    <a class="button ghost" href="wiki/words.html">Open the dictionary</a></div>
  <div class="card"><h3>OGABOOGA CODER</h3>
    <p>The IDE with the compiler inside: drag-and-drop blocks plus a real text editor and drawing canvas.</p>
    <a class="button ghost" href="wiki/ogabooga-coder.html">Meet the IDE</a></div>
</section>""")
    with open(os.path.join(SITE, "index.html"), "w", encoding="utf-8") as handle:
        handle.write(page_shell("Cave Man Code", body, "", nav))


# -------------------------------------------------------------------- assets

STYLE = """\
:root {
  --bg: #1b1410;
  --panel: #2a1f17;
  --panel2: #362818;
  --text: #f4e7d4;
  --dim: #b39b80;
  --accent: #ffb340;
  --green: #4cc38a;
  --purple: #c792ea;
  --code: #ffd166;
}
* { box-sizing: border-box; }
body {
  margin: 0; background: var(--bg); color: var(--text);
  font-family: "Segoe UI", system-ui, sans-serif; line-height: 1.6;
}
a { color: var(--accent); }
code, pre { font-family: Consolas, "Courier New", monospace; }
pre {
  background: #120d08; border: 1px solid #3d2d1f; border-radius: 10px;
  padding: 14px 16px; overflow-x: auto; color: var(--code); font-size: 14px;
}
:not(pre) > code {
  background: #120d08; border-radius: 5px; padding: 1px 6px; color: var(--code);
}
.topbar {
  display: flex; align-items: center; gap: 16px; padding: 10px 18px;
  background: var(--panel); border-bottom: 2px solid #120d08; position: sticky; top: 0; z-index: 5;
}
.brand { display: flex; align-items: center; gap: 10px; text-decoration: none;
  color: var(--text); font-weight: 700; font-size: 18px; }
.topnav { margin-left: auto; display: flex; gap: 14px; }
.topnav a { text-decoration: none; color: var(--text); font-weight: 600; }
.topnav a:hover { color: var(--accent); }
.searchbox { position: relative; flex: 0 1 380px; }
.searchbox input {
  width: 100%; padding: 8px 12px; border-radius: 8px; border: 1px solid #4a3728;
  background: #120d08; color: var(--text); font-size: 14px;
}
.search-results {
  position: absolute; top: 110%; left: 0; right: 0; background: var(--panel2);
  border-radius: 8px; border: 1px solid #4a3728; max-height: 320px; overflow-y: auto;
  display: none; z-index: 10;
}
.search-results a { display: block; padding: 8px 12px; text-decoration: none; color: var(--text); }
.search-results a:hover { background: #4a3728; }
.search-results .dim { color: var(--dim); font-size: 12px; display: block; }
.layout { display: flex; max-width: 1280px; margin: 0 auto; }
.sidebar {
  flex: 0 0 250px; padding: 18px 10px; border-right: 1px solid #362818;
  position: sticky; top: 62px; align-self: flex-start; max-height: calc(100vh - 62px);
  overflow-y: auto;
}
.sidebar ul { list-style: none; margin: 0 0 12px; padding: 0; }
.sidebar li a { display: block; padding: 3px 8px; color: var(--text); text-decoration: none;
  border-radius: 6px; font-size: 14px; }
.sidebar li a:hover { background: var(--panel2); }
.sidebar li a.active { background: var(--accent); color: #26180a; font-weight: 700; }
.nav-section { color: var(--dim); font-size: 12px; text-transform: uppercase;
  letter-spacing: 0.08em; margin: 14px 0 6px; padding: 0 8px; }
.content { flex: 1; padding: 26px 34px 60px; min-width: 0; }
.content h1 { font-size: 34px; margin-top: 0; }
.content h2 { color: var(--accent); border-bottom: 1px solid #362818; padding-bottom: 6px; margin-top: 36px; }
.content h3 { color: var(--green); }
.content table { border-collapse: collapse; width: 100%; margin: 14px 0; }
.content th, .content td { border: 1px solid #4a3728; padding: 8px 10px; text-align: left; }
.content th { background: var(--panel2); }
.content blockquote { border-left: 4px solid var(--accent); margin: 14px 0;
  padding: 6px 16px; background: var(--panel); border-radius: 0 8px 8px 0; color: var(--dim); }
.content img { max-width: 100%; }
.hero { display: flex; gap: 30px; align-items: center; padding: 34px 34px 10px;
  max-width: 1280px; margin: 0 auto; flex-wrap: wrap; }
.hero-text { flex: 1 1 420px; }
.hero h1 { font-size: 42px; margin: 0 0 10px; }
.lead { font-size: 18px; color: var(--dim); }
.hero-code { flex: 1 1 380px; }
.button {
  display: inline-block; background: var(--accent); color: #26180a; font-weight: 700;
  padding: 9px 16px; border-radius: 9px; text-decoration: none; margin: 4px 6px 4px 0;
  border: 2px solid transparent;
}
.button.big { font-size: 19px; padding: 13px 22px; }
.button.ghost { background: transparent; color: var(--text); border-color: var(--accent); }
.cards { display: flex; gap: 16px; flex-wrap: wrap; padding: 10px 34px; max-width: 1280px; margin: 0 auto; }
.card { flex: 1 1 280px; background: var(--panel); border: 1px solid #362818;
  border-radius: 12px; padding: 16px 18px; }
.card h3 { margin-top: 0; color: var(--accent); }
.homewiki { max-width: 1280px; margin: 0 auto; padding: 0 34px 40px; }
.aka { color: var(--dim); }
.chip { display: inline-block; background: var(--panel2); border-radius: 20px;
  padding: 2px 12px; font-size: 13px; color: var(--green); }
.wordlist { columns: 2; }
.dim { color: var(--dim); }
.footer { display: flex; justify-content: space-between; gap: 10px; padding: 18px 26px;
  border-top: 1px solid #362818; color: var(--dim); font-size: 13px; flex-wrap: wrap; }
@media (max-width: 900px) {
  .sidebar { display: none; }
  .wordlist { columns: 1; }
}
"""

APP_JS_TEMPLATE = """\
// Cave Man Code website helpers: wiki search + little niceties.
window.CMC_SEARCH = {index};

(function () {{
  var box = document.getElementById("search");
  var results = document.getElementById("search-results");
  if (!box || !results) return;
  function render(matches) {{
    if (!matches.length) {{ results.style.display = "none"; results.innerHTML = ""; return; }}
    results.innerHTML = matches.slice(0, 12).map(function (m) {{
      return '<a href="' + m.u + '">' + m.t + '<span class="dim">' + (m.d || "") + "</span></a>";
    }}).join("");
    results.style.display = "block";
  }}
  box.addEventListener("input", function () {{
    var q = box.value.trim().toLowerCase();
    if (q.length < 2) return render([]);
    var matches = window.CMC_SEARCH.filter(function (item) {{
      return (item.t + " " + item.k + " " + (item.d || "")).toLowerCase().indexOf(q) !== -1;
    }});
    render(matches);
  }});
  document.addEventListener("click", function (event) {{
    if (!results.contains(event.target) && event.target !== box) render([]);
  }});
}})();
"""

LOGO = """\
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
  <rect x="4" y="12" width="56" height="44" rx="10" fill="#ffb340" stroke="#26180a" stroke-width="3"/>
  <path d="M14 12 Q20 2 26 12 Q32 0 38 12 Q44 2 50 12" fill="none" stroke="#ffb340" stroke-width="6" stroke-linecap="round"/>
  <circle cx="22" cy="32" r="4" fill="#26180a"/>
  <circle cx="42" cy="32" r="4" fill="#26180a"/>
  <path d="M18 44 Q32 54 46 44" fill="none" stroke="#26180a" stroke-width="4" stroke-linecap="round"/>
  <text x="32" y="60" font-family="Consolas, monospace" font-size="11" fill="#26180a" text-anchor="middle" font-weight="bold">CMC</text>
</svg>
"""


def build_assets(pages):
    os.makedirs(os.path.join(SITE, "assets"), exist_ok=True)
    with open(os.path.join(SITE, "style.css"), "w", encoding="utf-8") as handle:
        handle.write(STYLE)
    with open(os.path.join(SITE, "assets", "logo.svg"), "w", encoding="utf-8") as handle:
        handle.write(LOGO)

    index = []
    for page in pages.values():
        index.append({
            "t": page["title"],
            "u": "wiki/" + page["slug"] + ".html",
            "k": " ".join(sorted(set(re.findall(r"[a-zA-Z_]{3,}", page["body"])))[:60]),
            "d": page["body"][:140].replace("\n", " ").replace('"', "'"),
        })
    for word in words.all_words():
        index.append({
            "t": word.get("display", word["name"]) + " (word)",
            "u": "wiki/words/" + word["name"] + ".html",
            "k": " ".join([word["name"]] + list(word.get("aliases", [])) + [word.get("category", "")]),
            "d": word.get("summary", ""),
        })
    with open(os.path.join(SITE, "app.js"), "w", encoding="utf-8") as handle:
        handle.write(APP_JS_TEMPLATE.format(index=json.dumps(index, ensure_ascii=False, indent=1)))


# ------------------------------------------------------------------- build

def build(check=False):
    pages = read_pages()
    site = load_site_json()
    os.makedirs(WIKI, exist_ok=True)

    for page in pages.values():
        rel = "../"
        nav = nav_html(pages, site, rel, page["slug"])
        with open(os.path.join(WIKI, page["slug"] + ".html"), "w", encoding="utf-8") as handle:
            handle.write(page_shell(page["title"], page["html"], rel, nav))

    build_word_pages(pages, site)
    build_download_page(pages, site)
    build_landing(pages, site)
    build_assets(pages)

    built = 0
    for base, _dirs, files in os.walk(SITE):
        built += len([f for f in files if f.endswith((".html", ".css", ".js", ".svg"))])
    print("Built " + str(built) + " site files into website/")

    if check:
        problems = check_links()
        if problems:
            print("Link check found " + str(len(problems)) + " problem(s):")
            for problem in problems:
                print("  - " + problem)
            return 1
        print("Link check: all internal links look good.")
    return 0


HREF_RE = re.compile(r'(?:href|src)="([^"#]+?)"')


def check_links():
    problems = []
    for base, _dirs, files in os.walk(SITE):
        for name in files:
            if not name.endswith(".html"):
                continue
            path = os.path.join(base, name)
            with open(path, "r", encoding="utf-8") as handle:
                text = handle.read()
            for target in HREF_RE.findall(text):
                if target.startswith(("http://", "https://", "mailto:", "data:")):
                    continue
                full = os.path.normpath(os.path.join(base, target))
                if not os.path.exists(full):
                    problems.append(os.path.relpath(path, SITE) + " -> " + target)
    return problems


if __name__ == "__main__":
    raise SystemExit(build(check="--check" in sys.argv))