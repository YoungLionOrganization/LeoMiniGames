#!/usr/bin/env python3
"""Require the image plugins promised to games and themes in desktop bundles."""
from pathlib import Path
import argparse


def require_image_backends(names):
    plugins = {Path(name).stem.lower() for name in names
               if 'imageformats' in {part.lower() for part in Path(name).parts}
               and Path(name).suffix.lower() in ('.dll', '.so', '.dylib')}
    for codec in ('qsvg', 'qwebp'):
        if not plugins.intersection({codec, 'lib'+codec, codec+'d', 'lib'+codec+'d'}):
            raise ValueError(f'Missing Qt image plugin {codec}: supported game/theme assets cannot decode.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('stage', type=Path)
    args = parser.parse_args()
    require_image_backends(str(path.relative_to(args.stage)) for path in args.stage.rglob('*')
                           if path.is_file() and path.stat().st_size > 0)
    print('PASS: SVG and WebP image plugins are included in desktop payload')


if __name__ == '__main__':
    main()
