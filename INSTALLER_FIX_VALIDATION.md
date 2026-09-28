# v0.7.2 Windows installer follow-up

The user screenshots showed (1) the Qt Installer Framework refusing an
occupied installation directory even after selecting Update, and (2) a script
exception when reading the `checked` property of an absent shortcut checkbox.

Setup now sends Update, Modify, and Uninstall to the registered installation's
Maintenance Tool. It never offers an in-place repair using a second Setup
process. If the tool cannot start, Setup reports the installed tool's location
and exits before writing to the installation folder. For an existing install,
open `LeoMiniGamesMaintenance.exe` directly if Setup cannot launch it.

The component is selected by default. The ready screen has a separate
application/version/destination summary, and shortcut options tolerate pages
or controls that QtIFW has not instantiated. Both CI and Build Release
Artifacts run the QtIFW script contract smoke test and JavaScript parse check.

Verification in this environment: 291 distribution checks, installer routing
and shortcut tests, script syntax, workflow YAML parse, and ZIP integrity.
The earlier v0.7.2 native Qt build had 8/8 CTest passes; no C++ sources changed
in this follow-up. A Windows machine with QtIFW is still required to click
through first install, installed Update/Modify/Uninstall, and installer rollback.
Live GitHub Actions job logs were not accessible here, so additional remote
runner failures cannot be identified from the user screenshots alone.
