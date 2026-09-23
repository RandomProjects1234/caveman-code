# Installing CMC

Cave Man Code is tiny and free. Everything is built in C++ with no
dependencies -- the compiler, the IDE, and the tests.

## Option 1: The zip (easiest)

1. Go to the [download page](../download.html).
2. Download **CMC-Compiler-v0.1.0.zip** or the all-in-one zip.
3. Right-click the zip and choose **Extract All**.
4. Open the folder and double-click `run_ogabooga.bat` (the IDE) or
   `cmc_run.bat` (the terminal runner).

That is it. Nothing else to install. The programs are already built:

- `bin\cmc.exe` -- the compiler, runner, and talk-back REPL
- `bin\ogabooga.exe` -- OGABOOGA CODER, the friendly IDE

## Option 2: Build it from the repo (for tinkerers)

You need a C++17 compiler (g++ or clang). Then:

```text
cd caveman-code
native\build.bat                    # builds bin\cmc.exe and bin\ogabooga.exe
bin\cmc.exe                         # talk to CMC in the terminal
bin\cmc.exe run examples\01_hello_oga.cmc
bin\ogabooga.exe                    # open the IDE
```

On macOS or Linux use the same g++ command found in `native\build.bat`.
The compiler and REPL build anywhere; the IDE and its drawing window
need Windows.

## Checking it works

```cmc
oga "hello cave"
```

Save that as `hello.cmc` and run it:

```text
bin\cmc.exe run hello.cmc
```

You should see:

```text
hello cave
```

If you do, welcome to the cave. Go try the [tutorial](tutorial.md).

## Building stand-alone programs (optional)

`native\build.bat` already makes two stand-alone programs with no
dependencies. Share the whole folder (or a zip) and the other person can
double-click the launchers right away.

## Troubleshooting installs

- **"cmc.exe is not recognized"** -- run it with its path: `bin\cmc.exe`.
- **The launcher says the compiler is not built** -- run `native\build.bat`
  first (you need g++ or clang).
- **A window does not appear** -- you may be on a computer with no desktop.
  `bin\cmc.exe run` works without the IDE.
