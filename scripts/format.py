"""Custom-Target "format": wendet clang-format auf src/ und include/ an.

Aufruf:  pio run -t format

clang-format wird in dieser Reihenfolge gesucht:
  1. im PATH
  2. in ~/.platformio/penv/bin  (pip install clang-format)
  3. neben dem laufenden Python-Interpreter
"""

import glob
import os
import shutil
import sys

Import("env")  # noqa: F821

# compile_commands.json inkl. Toolchain-Include-Pfaden erzeugen,
# damit clangd die System-Header (sys/reent.h etc.) findet.
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)  # noqa: F821


def _clean_compiledb():
    """Entfernt die Include-Pfade der RISC-V-Toolchain (ULP-Coprozessor)
    aus der compile_commands.json. Deren newlib/libstdc++-Header mischen
    sich sonst mit den Xtensa-Headern und produzieren in clangd
    Phantom-Fehler (z. B. "no member named 'acoshl'")."""
    import json
    import re

    path = os.path.join(env["PROJECT_DIR"], "compile_commands.json")  # noqa: F821

    if not os.path.isfile(path):
        return

    with open(path) as f:
        db = json.load(f)

    for entry in db:
        entry["command"] = re.sub(r" -I\S*riscv\S*", "", entry["command"])

    with open(path, "w") as f:
        json.dump(db, f, indent=1)

    print("compile_commands.json: RISC-V-Include-Pfade entfernt")


if "compiledb" in COMMAND_LINE_TARGETS:  # noqa: F821
    import atexit

    atexit.register(_clean_compiledb)


def find_clang_format():
    candidates = [
        shutil.which("clang-format"),
        os.path.expanduser("~/.platformio/penv/bin/clang-format"),
        os.path.join(os.path.dirname(sys.executable), "clang-format"),
    ]

    for candidate in candidates:
        if candidate and os.path.isfile(candidate):
            return candidate

    return None


CLANG_FORMAT = find_clang_format()

PROJECT_DIR = env["PROJECT_DIR"]  # noqa: F821

SOURCES = sorted(
    glob.glob(os.path.join(PROJECT_DIR, "src", "*.cpp"))
    + glob.glob(os.path.join(PROJECT_DIR, "src", "*.h"))
    + glob.glob(os.path.join(PROJECT_DIR, "include", "*.h"))
)

if CLANG_FORMAT:
    format_action = '"%s" -i %s' % (CLANG_FORMAT, " ".join('"%s"' % s for s in SOURCES))
else:
    format_action = (
        'echo "clang-format nicht gefunden.'
        ' Installieren mit: pip3 install clang-format'
        ' (oder: brew install clang-format)"'
    )

env.AddCustomTarget(  # noqa: F821
    name="format",
    dependencies=None,
    actions=[format_action],
    title="Format",
    description="clang-format auf alle Projektquellen anwenden",
)
