#!/usr/bin/env python3
"""Validate candidate QtIFW versions and every advertised payload checksum."""
import argparse
import hashlib
import json
import re
from pathlib import Path
import xml.etree.ElementTree as ET

ARCHES = ('x86_64', 'x86_64-AVX2', 'ARM64')


def validate(directory, version):
    repositories = directory / 'windows'
    if {p.name for p in repositories.iterdir() if p.is_dir()} != set(ARCHES):
        raise ValueError('QtIFW architecture set differs from release policy')
    for arch in ARCHES:
        repository = repositories / arch
        tree = ET.parse(repository / 'Updates.xml')
        updates = tree.findall('PackageUpdate')
        if len(updates) != 1 or updates[0].findtext('Name') != 'xyz.younglion.leominigames':
            raise ValueError(f'{arch}: unexpected QtIFW component set')
        update = updates[0]
        if update.findtext('Version') != version:
            raise ValueError(f'{arch}: stale QtIFW component version')
        sizes = update.find('UpdateFile')
        if sizes is None or any(int(sizes.get(key, '0')) <= 0 for key in ('CompressedSize', 'UncompressedSize')):
            raise ValueError(f'{arch}: missing payload size metadata')
        archives = [name.strip() for name in (update.findtext('DownloadableArchives') or '').split(',') if name.strip()]
        if not archives or len(set(archives)) != len(archives):
            raise ValueError(f'{arch}: missing or duplicate payload archives')
        component = repository / 'xyz.younglion.leominigames'
        for name in archives:
            if Path(name).name != name or name in ('.', '..'):
                raise ValueError(f'{arch}: invalid archive name')
            archive = component / (version + name)
            if not archive.is_file() or archive.stat().st_size == 0:
                raise ValueError(f'{arch}: missing payload {archive.name}')
            expected = Path(str(archive) + '.sha1').read_text().strip().lower()
            if not re.fullmatch('[0-9a-f]{40}', expected):
                raise ValueError(f'{arch}: invalid payload checksum')
            sha = hashlib.sha1()
            with archive.open('rb') as stream:
                for chunk in iter(lambda: stream.read(1024 * 1024), b''): sha.update(chunk)
            if sha.hexdigest() != expected:
                raise ValueError(f'{arch}: payload checksum mismatch')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    validate(args.directory, json.loads((root/'release/release.json').read_text())['version'])
    print('PASS: three candidate QtIFW repositories, payload sizes and checksums')
