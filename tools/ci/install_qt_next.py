#!/usr/bin/env python3
"""Run pinned aqt with Qt 6.12's split Windows MSVC repository layout.

Qt's public Updates.xml now lives below qt6_6120_msvc2022_64 on Windows.
aqt 3.3.0 still requests qt6_6120. Delegate parsing, extraction, relocation
and checksum verification to aqt; adjust only this exact repository path.
Used on disposable GitHub runners, not on the maintainer's computer.
"""
from importlib.metadata import version


def main():
    from aqt import main as aqt_main
    from aqt.archives import QtArchives

    if version('aqtinstall') != '3.3.0':
        raise SystemExit('Recheck the Qt repository compatibility adapter before changing aqt 3.3.0')
    original = QtArchives._get_archives

    def archives(self):
        if (self.os_name, self.target, str(self.version), self.arch) == (
                'windows', 'desktop', '6.12.0', 'win64_msvc2022_64'):
            return self._get_archives_base(
                'qt6_6120/qt6_6120_msvc2022_64', self._target_packages())
        return original(self)

    QtArchives._get_archives = archives
    return aqt_main()


if __name__ == '__main__':
    raise SystemExit(main())
