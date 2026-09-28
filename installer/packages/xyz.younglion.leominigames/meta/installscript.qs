function Component()
{
    if (!installer.isCommandLineInstance() && installer.isInstaller()) {
        if (installer.value("LMGExistingInstallDir"))
            installer.addWizardPage(component, "ExistingInstallationPage", QInstaller.TargetDirectory);
        installer.addWizardPage(component, "InstallOptionsPage", QInstaller.ReadyForInstallation);
        installer.addWizardPageItem(component, "InstallationSummary", QInstaller.ReadyForInstallation, 10);
    }
}

Component.prototype.DynamicExistingInstallationPageCallback = function()
{
    var page = gui.pageWidgetByObjectName("DynamicExistingInstallationPage");
    if (!page) return;
    page.windowTitle = "Existing LeoMiniGames Installation";
    page.IntroLabel.text = "An existing LeoMiniGames installation was detected. Choose an action for the Maintenance Tool.";
    page.DetectedVersionLabel.text = "Installed version: " + installer.value("LMGExistingVersion");
    page.DetectedPathLabel.text = "Installation folder: " + installer.value("LMGExistingInstallDir");
}

Component.prototype.DynamicInstallOptionsPageCallback = function()
{
    var page = gui.pageWidgetByObjectName("DynamicInstallOptionsPage");
    if (!page) return;
    page.windowTitle = "Windows Integration";
    page.BuildLabel.text = "Build: " + installer.value("LMGBuildLabel") + "\nDetected processor: " + installer.value("LMGCpuDescription");
}

Component.prototype.createOperations = function()
{
    component.createOperations();
    if (systemInfo.productType !== "windows") return;

    var options = typeof gui !== "undefined" && gui ? gui.pageWidgetByObjectName("DynamicInstallOptionsPage") : null;
    if (!options) options = component.userInterface("InstallOptionsPage");
    var desktopShortcut = !options || !options.DesktopShortcutCheckBox || options.DesktopShortcutCheckBox.checked;
    var startMenuShortcut = !options || !options.StartMenuShortcutCheckBox || options.StartMenuShortcutCheckBox.checked;
    var maintenanceShortcut = !options || !options.MaintenanceShortcutCheckBox || options.MaintenanceShortcutCheckBox.checked;

    if (startMenuShortcut) {
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGames.exe",
            "@StartMenuDir@/LeoMiniGames.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Launch LeoMiniGames");
    }
    if (desktopShortcut) {
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGames.exe",
            "@DesktopDir@/LeoMiniGames.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Launch LeoMiniGames");
    }
    if (maintenanceShortcut) {
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGamesMaintenance.exe",
            "@StartMenuDir@/LeoMiniGames Update and Repair.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Update, repair, modify or uninstall LeoMiniGames");
    }

    // These QSettings-backed markers let a future Setup locate a non-default install.
    component.addElevatedOperation("GlobalConfig", "SystemScope", "YoungLion", "LeoMiniGames", "InstallDir", "@TargetDir@");
    component.addElevatedOperation("GlobalConfig", "SystemScope", "YoungLion", "LeoMiniGames", "Version", "@ProductVersion@");
    component.addElevatedOperation("GlobalConfig", "SystemScope", "YoungLion", "LeoMiniGames", "Architecture", "__LMG_WINDOWS_ARCH__");
    component.addElevatedOperation("GlobalConfig", "SystemScope", "YoungLion", "LeoMiniGames", "CpuProfile", "__LMG_WINDOWS_CPU_PROFILE__");
    component.addOperation("GlobalConfig", "YoungLion", "LeoMiniGames", "InstallDir", "@TargetDir@");
    component.addOperation("GlobalConfig", "YoungLion", "LeoMiniGames", "Version", "@ProductVersion@");
}
