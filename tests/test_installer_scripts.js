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

// QtIFW validates the custom page before it enters TargetDirectory; this is
// the actual Next-button path for an existing installation.
for (const action of ['upgrade', 'modify', 'uninstall']) {
    const values = { LMGExistingInstallDir: 'C:/Program Files/LeoMiniGames',
        LMGUpdateRepository: 'https://example.test/updates' };
    const calls = [];
    const component = {};
    const page = { UpgradeRadioButton: { checked: action === 'upgrade' },
        ModifyRadioButton: { checked: action === 'modify' },
        UninstallRadioButton: { checked: action === 'uninstall' } };
    const installer = {
        isCommandLineInstance: () => false, isInstaller: () => true,
        value: key => values[key] || '', fileExists: () => true,
        addWizardPage: () => true,
        addWizardPageItem: () => true,
        setValidatorForCustomPage: (comp, name, method) => calls.push(['validator', comp, name, method]),
        executeDetached: (exe, args, dir) => { calls.push(['launch', exe, args, dir]); return true; },
    };
    const gui = { pageWidgetByObjectName: () => page,
        rejectWithoutPrompt: () => calls.push(['quit']) };
    const context = { installer, gui, component,
        QInstaller: { TargetDirectory: 1, ReadyForInstallation: 2 },
        QMessageBox: { critical: () => { throw Error('Unexpected maintenance error'); } } };
    vm.runInNewContext(script, context);
    const instance = new context.Component();
    assert.equal(calls[0][3], 'validateExistingInstallationPage');
    assert.equal(instance.validateExistingInstallationPage(), false);
    assert.ok(calls.some(c => c[0] === 'quit'));
    assert.equal(calls.find(c => c[0] === 'launch')[2].at(-1),
        action === 'upgrade' ? '--start-updater' :
            action === 'modify' ? '--start-package-manager' : '--start-uninstaller');
}

// A fresh installation must still use the normal target directory and show
// the application even if QtIFW's built-in component tree is initially empty.
{
    const values = { TargetDir: 'C:/Apps/LeoMiniGames', ProductVersion: '0.7.2',
        LMGBuildLabel: 'Windows x64 baseline' };
    let summaryText = '';
    let installText = '';
    const page = { InstallMsgLabel: { setText: text => { installText = text; } },
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
    assert.match(installText, /LeoMiniGames 0\.7\.2/);
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
{
    const operations = [];
    const widget = {};
    const selected = { DesktopShortcutCheckBox: { checked: false },
        StartMenuShortcutCheckBox: { checked: true }, MaintenanceShortcutCheckBox: { checked: false } };
    const component = { createOperations: () => {}, userInterface: () => widget,
        addOperation: (name, ...args) => operations.push([name, ...args]),
        addElevatedOperation: () => {} };
    const context = { component, installer: { isInstaller: () => false, isCommandLineInstance: () => false },
        gui: { pageWidgetByObjectName: () => widget, findChild: (_widget, name) => selected[name] },
        systemInfo: { productType: 'windows' } };
    vm.runInNewContext(script, context);
    new context.Component().createOperations();
    assert.equal(operations.filter(x => x[0] === 'CreateShortcut').length, 1);
}
console.log('PASS: QtIFW existing install routing, launch failure, and shortcut options');
