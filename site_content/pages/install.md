# Installing CMC

Cave Man Code is tiny and free. There are two ways to get it.

## Option 1: The zip (easiest)

1. Go to the [download page](../download.html).
2. Download **CMC-Compiler-v0.1.0.zip** or the all-in-one zip.
3. Right-click the zip and choose **Extract All**.
4. Open the folder and double-click `run_ogabooga.bat`.

That is it. Python 3.9 or newer is the only requirement (it comes with
`tkinter`, which CMC uses for its windows).

## Option 2: From the repo (for tinkerers)

If you cloned the GitHub repo:

```text
cd caveman-code
python -m cmc                       # talk to CMC in the terminal
python -m cmc run examples/01_hello_oga.cmc
python -m ogabooga                  # open the IDE
```

The commands need `src` on the import path. Two ways:

```text
# Windows PowerShell
$env:PYTHONPATH = "src"

# macOS / Linux
export PYTHONPATH=src
```

Or install it as a normal Python package, which creates the `cmc` and
`ogabooga` commands:

```text
pip install .
cmc version
ogabooga
```

## Checking it works

```cmc
oga "hello cave"
```

Save that as `hello.cmc` and run it:

```text
cmc run hello.cmc
```

You should see:

```text
hello cave
```

If you do, welcome to the cave. Go try the [tutorial](tutorial.md).

## Making a stand-alone .exe (optional)

If you want to share CMC with someone who does not have Python, the repo has a
PyInstaller script:

```text
pip install pyinstaller
powershell -File tools/build_exe.ps1
```

This makes `OgaboogaCoder.exe` (the IDE) and `cmc.exe` (the compiler). It takes
a couple of minutes the first time.

## Troubleshooting installs

- **"python is not recognized"** -- Python is not on the PATH. Reinstall
  Python and tick *Add Python to PATH*.
- **A window does not appear** -- you may be on a computer with no desktop.
  The `cmc run` command works without the IDE.
- **"No module named tkinter"** -- on Linux install it with
  `sudo apt install python3-tk`.