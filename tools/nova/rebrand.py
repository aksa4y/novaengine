#!/usr/bin/env python3
"""Complete project-owned Doriax -> Nova mechanical rebrand.

The migration intentionally excludes vendored third-party code under libs/.
It updates source/configuration/documentation text and renames project-owned
files whose names contain Doriax/doriax/DORIAX. Run from the repository root.
The mapping is idempotent and safe to rerun.
"""
from __future__ import annotations

from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SKIP_DIRS = {
    ".git",
    "libs",
    "build",
    "build-rhi",
    "Build",
    "cmake-build-debug",
    "cmake-build-release",
    "out",
    "dist",
}
SKIP_FILES = {Path("tools/nova/rebrand.py")}
TEXT_EXTS = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".m", ".mm",
    ".cmake", ".md", ".txt", ".in", ".py", ".sh", ".yml", ".yaml", ".json",
    ".glsl", ".frag", ".vert", ".xml", ".plist", ".rc", ".rc2", ".ini", ".cfg",
    ".lua", ".js", ".ts", ".html", ".css", ".svg", ".cs", ".java", ".toml",
}
TEXT_FILENAMES = {"CMakeLists.txt", "Dockerfile"}
NAME_REPLACEMENTS = (("DORIAX", "NOVA"), ("Doriax", "Nova"), ("doriax", "nova"))


def tracked_files() -> list[Path]:
    raw = subprocess.check_output(["git", "ls-files", "-z"], cwd=ROOT)
    return [ROOT / item.decode("utf-8") for item in raw.split(b"\0") if item]


def should_skip(path: Path) -> bool:
    try:
        rel = path.relative_to(ROOT)
    except ValueError:
        return True
    return rel in SKIP_FILES or any(part in SKIP_DIRS for part in rel.parts)


def is_text_candidate(path: Path) -> bool:
    return path.name in TEXT_FILENAMES or path.suffix.lower() in TEXT_EXTS


def rewrite_content(path: Path) -> bool:
    if should_skip(path) or not path.exists() or not is_text_candidate(path):
        return False
    try:
        data = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        return False
    updated = data
    for old, new in NAME_REPLACEMENTS:
        updated = updated.replace(old, new)
    if updated == data:
        return False
    path.write_text(updated, encoding="utf-8", newline="")
    return True


def rename_paths() -> int:
    renamed = 0
    for path in sorted(tracked_files(), key=lambda p: len(p.parts), reverse=True):
        if should_skip(path) or not path.exists():
            continue
        updated = path.name
        for old, new in NAME_REPLACEMENTS:
            updated = updated.replace(old, new)
        if updated != path.name:
            dest = path.with_name(updated)
            if not dest.exists():
                path.rename(dest)
                renamed += 1
    return renamed


def main() -> int:
    changed = sum(1 for path in tracked_files() if rewrite_content(path))
    renamed = rename_paths()
    print(f"Nova rebrand: content_files={changed} renamed_paths={renamed}")
    print("Vendored/generated exclusions: libs/, .git/, build trees, out/, dist/.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
