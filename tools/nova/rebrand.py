#!/usr/bin/env python3
"""Mechanical Doriax -> Nova rebrand.

Rewrites project-owned text/source files and filenames while excluding vendored
third-party trees. The script is idempotent and intended for the one-time
migration on feature/nova-foundation.
"""
from __future__ import annotations

from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SKIP_DIRS = {".git", "libs", "build", "build-rhi", "cmake-build-debug", "cmake-build-release"}
TEXT_EXTS = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".m", ".mm",
    ".cmake", ".md", ".txt", ".in", ".py", ".sh", ".yml", ".yaml", ".json",
    ".glsl", ".frag", ".vert", ".xml", ".plist", ".rc", ".rc2", ".ini", ".cfg",
    ".lua", ".js", ".ts", ".html", ".css", ".svg"
}
NAME_REPLACEMENTS = (("DORIAX", "NOVA"), ("Doriax", "Nova"), ("doriax", "nova"))
CONTENT_REPLACEMENTS = NAME_REPLACEMENTS


def tracked_files() -> list[Path]:
    raw = subprocess.check_output(["git", "ls-files", "-z"], cwd=ROOT)
    paths = [ROOT / item.decode("utf-8") for item in raw.split(b"\0") if item]
    return paths


def should_skip(path: Path) -> bool:
    try:
        rel = path.relative_to(ROOT)
    except ValueError:
        return True
    return any(part in SKIP_DIRS for part in rel.parts)


def is_text_candidate(path: Path) -> bool:
    return path.suffix.lower() in TEXT_EXTS


def rewrite_content(path: Path) -> bool:
    if should_skip(path) or not is_text_candidate(path):
        return False
    try:
        data = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        return False
    updated = data
    for old, new in CONTENT_REPLACEMENTS:
        updated = updated.replace(old, new)
    if updated == data:
        return False
    path.write_text(updated, encoding="utf-8", newline="")
    return True


def rename_paths() -> int:
    renamed = 0
    paths = sorted(tracked_files(), key=lambda p: len(p.parts), reverse=True)
    for path in paths:
        if should_skip(path):
            continue
        name = path.name
        updated = name
        for old, new in NAME_REPLACEMENTS:
            updated = updated.replace(old, new)
        if updated != name:
            dest = path.with_name(updated)
            if not dest.exists():
                path.rename(dest)
                renamed += 1
    return renamed


def main() -> int:
    changed = 0
    for path in tracked_files():
        if path.exists() and rewrite_content(path):
            changed += 1
    renamed = rename_paths()
    print(f"rebrand: content_files={changed} renamed_paths={renamed}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
