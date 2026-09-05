#!/usr/bin/env python3
"""Rewrite legacy root-build paths from engine/ to runtime/.

This script is intentionally narrow: it changes build-path references only and
leaves source identifiers (Doriax namespace/classes) untouched. Run it from the
repository root after verifying the working tree is clean.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FILES = [ROOT / "CMakeLists.txt"]

REPLACEMENTS = (
    ("${CMAKE_CURRENT_SOURCE_DIR}/engine", "${CMAKE_CURRENT_SOURCE_DIR}/runtime"),
    ('"${CMAKE_SOURCE_DIR}/engine/', '"${CMAKE_SOURCE_DIR}/runtime/'),
    ('${CMAKE_SOURCE_DIR}/engine/', '${CMAKE_SOURCE_DIR}/runtime/'),
)


def rewrite(path: Path) -> bool:
    original = path.read_text(encoding="utf-8")
    updated = original
    for old, new in REPLACEMENTS:
        updated = updated.replace(old, new)
    if updated == original:
        return False
    path.write_text(updated, encoding="utf-8")
    return True


def main() -> int:
    changed = [path for path in FILES if rewrite(path)]
    if changed:
        for path in changed:
            print(f"updated: {path.relative_to(ROOT)}")
    else:
        print("no changes needed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
