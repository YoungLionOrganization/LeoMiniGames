#!/usr/bin/env python3
"""Reject desktop payloads that link Multimedia but omit its media backend."""
from pathlib import Path
import argparse


def require_audio_backend(names):
    candidates = [Path(name) for name in names]
    if not any('multimedia' in {part.lower() for part in path.parts}
               and 'mediaplugin' in path.name.lower()
               and path.suffix.lower() in ('.dll', '.so', '.dylib') for path in candidates):
        raise ValueError('Missing Qt Multimedia media backend: effects fallback and music cannot work.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('stage', type=Path)
    args = parser.parse_args()
    require_audio_backend(str(path.relative_to(args.stage)) for path in args.stage.rglob('*')
                          if path.is_file() and path.stat().st_size > 0)
    print('PASS: Qt Multimedia media backend is included in desktop payload')


if __name__ == '__main__':
    main()
