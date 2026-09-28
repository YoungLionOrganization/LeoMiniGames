function Component()
{
    if (!installer.isCommandLineInstance() && installer.isInstaller()) {
        if (installer.value("LMGExistingInstallDir")) {
            installer.addWizardPage(component, "ExistingInstallationPage", QInstaller.TargetDirectory);
            installer.setValidatorForCustomPage(component, "ExistingInstallationPage", "validateExistingInstallationPage");
        }
        installer.addWizardPage(component, "InstallOptionsPage", QInstaller.ReadyForInstallation);
        installer.addWizardPageItem(component, "InstallationSummary", QInstaller.ReadyForInstallation, 10);
    }
}

Component.prototype.validateExistingInstallationPage = function()
{
    // The validator runs on Next, before QtIFW validates the occupied TargetDir.
    // Never advance this installer to the fresh-install pages for an existing app.
    var dir = installer.value("LMGExistingInstallDir");
    var page = gui.pageWidgetByObjectName("DynamicExistingInstallationPage");
    var action = "upgrade";
    if (page && page.UninstallRadioButton && page.UninstallRadioButton.checked) action = "uninstall";
    else if (page && page.ModifyRadioButton && page.ModifyRadioButton.checked) action = "modify";
    var tool = dir + "/LeoMiniGamesMaintenance.exe";
    var args = action === "uninstall" ? ["--start-uninstaller"] :
        ["--set-temp-repository", installer.value("LMGUpdateRepository"),
            action === "modify" ? "--start-package-manager" : "--start-updater"];
    if (!dir || !installer.fileExists(tool) || !installer.executeDetached(tool, args, dir)) {
        QMessageBox.critical("MaintenanceLaunchError", "LeoMiniGames Setup",
            "The installed Maintenance Tool could not be started from " + dir +
            ". Open LeoMiniGamesMaintenance.exe in that folder to update or repair LeoMiniGames.");
        return false;
    }
    gui.rejectWithoutPrompt();
    return false;
}

Component.prototype.DynamicExistingInstallationPageCallback = function()
{
    var page = gui.pageWidgetByObjectName("DynamicExistingInstallationPage");
    if (!page) return;
    page.windowTitle = "Existing LeoMiniGames Installation";
    if (page.IntroLabel)
        page.IntroLabel.text = "An existing LeoMiniGames installation was detected. Choose an action for the Maintenance Tool.";
    if (page.DetectedVersionLabel)
        page.DetectedVersionLabel.text = "Installed version: " + installer.value("LMGExistingVersion");
    if (page.DetectedPathLabel)
        page.DetectedPathLabel.text = "Installation folder: " + installer.value("LMGExistingInstallDir");
}

Component.prototype.DynamicInstallOptionsPageCallback = function()
{
    var page = gui.pageWidgetByObjectName("DynamicInstallOptionsPage");
    if (!page) return;
    page.windowTitle = "Windows Integration";
    if (page.BuildLabel)
        page.BuildLabel.text = "Build: " + installer.value("LMGBuildLabel") + "\nDetected processor: " + installer.value("LMGCpuDescription");
}

Component.prototype.createOperations = function()
{
    component.createOperations();
    if (systemInfo.productType !== "windows") return;

    var options = typeof gui !== "undefined" && gui ? gui.pageWidgetByObjectName("DynamicInstallOptionsPage") : null;
    if (!options) options = component.userInterface("InstallOptionsPage");
    function selected(name) {
        var checkbox = options && (options[name] ||
            (typeof gui !== "undefined" && gui && gui.findChild ? gui.findChild(options, name) : null));
        return !checkbox || checkbox.checked;
    }
    var desktopShortcut = selected("DesktopShortcutCheckBox");
    var startMenuShortcut = selected("StartMenuShortcutCheckBox");
    var maintenanceShortcut = selected("MaintenanceShortcutCheckBox");

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
