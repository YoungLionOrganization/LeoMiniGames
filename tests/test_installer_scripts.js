// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
// QtIFW script contract smoke test. Native Windows UI still needs a release gate.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const root = path.resolve(__dirname, '..');
const control = fs.readFileSync(path.join(root, 'installer/config/control.qs'), 'utf8');
const script = fs.readFileSync(path.join(root, 'installer/packages/xyz.younglion.leominigames/meta/installscript.qs'), 'utf8');

function controllerHarness(action, launched = true) {
    const values = { TargetDir: 'C:/Program Files/LeoMiniGames' };
    const calls = [];
    const existing = 'D:/Applications/LeoMiniGames';
    const page = action === null ? undefined : {
        UpgradeRadioButton: { checked: action === 'upgrade' },
        ModifyRadioButton: { checked: action === 'modify' },
        UninstallRadioButton: { checked: action === 'uninstall' },
    };
    const installer = {
        setValue: (k, v) => { values[k] = v; calls.push(['setValue', k, v]); },
        value: k => values[k] || '',
        fileExists: p => p.startsWith(existing + '/') &&
            (p.endsWith('LeoMiniGamesMaintenance.exe') || p.endsWith('LeoMiniGames.exe')),
        execute: () => ['', 1],
        executeDetached: (exe, args, dir) => { calls.push(['executeDetached', exe, args, dir]); return launched; },
        environmentVariable: () => '',
    };
    const gui = {
        pageWidgetByObjectName: () => page,
        currentPageWidget: () => ({}),
        rejectWithoutPrompt: () => calls.push(['rejectWithoutPrompt']),
    };
    const context = { installer, gui, systemInfo: { productType: 'windows', currentCpuArchitecture: 'x86_64' },
        QMessageBox: { critical: (...args) => calls.push(['critical', ...args]) } };
    vm.runInNewContext(control, context, { filename: 'control.qs' });
    values.LMGExistingInstallDir = existing;
    values.LMGUpdateRepository = 'https://example.test/updates';
    context.Controller.prototype.TargetDirectoryPageCallback.call(new context.Controller());
    assert.equal(values.TargetDir, 'C:/Program Files/LeoMiniGames', 'setup must not retarget to occupied installation');
    const launchedCall = calls.find(c => c[0] === 'executeDetached');
    assert.ok(launchedCall, 'maintenance tool must be used');
    assert.equal(launchedCall[1], existing + '/LeoMiniGamesMaintenance.exe');
    assert.equal(launchedCall[3], existing);
    assert.equal(calls.some(c => c[0] === 'rejectWithoutPrompt'), true);
    return { calls, args: launchedCall[2] };
}

assert.equal(controllerHarness('upgrade').args[2], '--start-updater');
assert.equal(controllerHarness('modify').args[2], '--start-package-manager');
assert.equal(controllerHarness('uninstall').args[0], '--start-uninstaller');
assert.equal(controllerHarness(null).args[2], '--start-updater');
assert.equal(controllerHarness('upgrade', false).calls.some(c => c[0] === 'critical'), true);

// A fresh installation must still use the normal target directory and show
// the application even if QtIFW's built-in component tree is initially empty.
{
    const values = { TargetDir: 'C:/Apps/LeoMiniGames', ProductVersion: '0.7.2',
        LMGBuildLabel: 'Windows x64 baseline' };
    let summaryText = '';
    const page = { InstallMsgLabel: { setText: () => {} },
        InstallComponentsTreeview: {}, ComponentSummaryScrollArea: {} };
    const gui = { currentPageWidget: () => page,
        pageWidgetByObjectName: () => ({ SummaryLabel: { setText: text => { summaryText = text; } } }) };
    const installer = { setValue: (k, v) => { values[k] = v; }, value: k => values[k] || '',
        fileExists: () => false, execute: () => ['', 1], environmentVariable: () => '',
        selectComponent: () => {} };
    const context = { installer, gui, systemInfo: { productType: 'windows' } };
    vm.runInNewContext(control, context);
    const controller = new context.Controller();
    controller.TargetDirectoryPageCallback();
    assert.equal(page.title, 'Installation Folder');
    controller.ReadyForInstallationPageCallback();
    assert.match(summaryText, /LeoMiniGames 0\.7\.2/);
    assert.match(summaryText, /C:\/Apps\/LeoMiniGames/);
}

function optionsHarness(widget) {
    const operations = [];
    const component = {
        createOperations: () => {},
        userInterface: () => widget,
        addOperation: (name, ...args) => operations.push([name, ...args]),
        addElevatedOperation: (name, ...args) => operations.push([name, ...args]),
    };
    const context = { installer: { isCommandLineInstance: () => false, isInstaller: () => false },
        gui: { pageWidgetByObjectName: () => widget }, component,
        systemInfo: { productType: 'windows' } };
    vm.runInNewContext(script, context, { filename: 'installscript.qs' });
    context.Component.prototype.createOperations.call(new context.Component());
    return operations.filter(op => op[0] === 'CreateShortcut');
}

assert.equal(optionsHarness(undefined).length, 3, 'unset custom page uses safe default');
assert.equal(optionsHarness({}).length, 3, 'missing checkbox controls must not crash');
assert.equal(optionsHarness({ DesktopShortcutCheckBox: { checked: false },
    StartMenuShortcutCheckBox: { checked: true }, MaintenanceShortcutCheckBox: { checked: false } }).length, 1);
console.log('PASS: QtIFW existing install routing, launch failure, and shortcut options');
