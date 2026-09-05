#!/usr/bin/env python3
"""Complete project-owned Doriax -> Nova mechanical rebrand.

The migration excludes vendored third-party code under libs/. It updates
project-owned source/configuration/documentation text, with explicit handling
for the C++ Nova namespace, and renames project-owned files containing the
old product name. The mapping is idempotent and safe to rerun.
"""
from __future__ import annotations

from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SKIP_DIRS = {
    ".git", "libs", "build", "build-rhi", "Build", "cmake-build-debug",
    "cmake-build-release", "out", "dist",
}
SKIP_FILES = {Path("tools/nova/rebrand.py")}
TEXT_EXTS = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".m", ".mm",
    ".cmake", ".md", ".txt", ".in", ".py", ".sh", ".yml", ".yaml", ".json",
    ".glsl", ".frag", ".vert", ".xml", ".plist", ".rc", ".rc2", ".ini", ".cfg",
    ".lua", ".js", ".ts", ".html", ".css", ".svg", ".cs", ".java", ".toml",
}
TEXT_FILENAMES = {"CMakeLists.txt", "Dockerfile", ".gitignore", ".gitmodules"}

# Product branding / macros / filenames.
BRAND_REPLACEMENTS = (
    ("DORIAX", "NOVA"),
    ("Doriax", "Nova"),
)
# C++ namespace spelling is intentionally Nova (capital N), while lowercase
# doriax remains available for filesystem/package-style names such as targets.
CPP_REPLACEMENTS = (
    ("namespace doriax", "namespace Nova"),
    ("::doriax::", "::Nova::"),
    ("::doriax", "::Nova"),
    ("doriax::", "Nova::"),
    ("doriax_h", "nova_h"),
)
LOWERCASE_REPLACEMENTS = (("doriax", "nova"),)


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


def rebrand_content(data: str) -> str:
    updated = data
    # Namespace references first so the generic lowercase replacement does not
    # turn the C++ namespace into `nova`.
    for old, new in CPP_REPLACEMENTS:
        updated = updated.replace(old, new)
    for old, new in BRAND_REPLACEMENTS:
        updated = updated.replace(old, new)
    for old, new in LOWERCASE_REPLACEMENTS:
        updated = updated.replace(old, new)
    return updated


def rewrite_content(path: Path) -> bool:
    if should_skip(path) or not path.exists() or not is_text_candidate(path):
        return False
    try:
        data = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        return False
    updated = rebrand_content(data)
    if updated == data:
        return False
    path.write_text(updated, encoding="utf-8", newline="")
    return True


def renamed_name(name: str) -> str:
    result = name
    for old, new in BRAND_REPLACEMENTS:
        result = result.replace(old, new)
    result = result.replace("doriax", "nova")
    return result


def rename_paths() -> int:
    renamed = 0
    paths = sorted(tracked_files(), key=lambda p: len(p.parts), reverse=True)
    for path in paths:
        if should_skip(path) or not path.exists():
            continue
        target_name = renamed_name(path.name)
        if target_name == path.name:
            continue
        dest = path.with_name(target_name)
        if dest.exists():
            # Nova.h was introduced earlier as a compatibility umbrella. The
            # canonical fully renamed Doriax.h should become the one Nova.h.
            if path.name == "Doriax.h" and dest.name == "Nova.h":
                path_data = path.read_bytes()
                dest.write_bytes(path_data)
                path.unlink()
                renamed += 1
                continue
            raise RuntimeError(f"Refusing filename collision: {path} -> {dest}")
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
