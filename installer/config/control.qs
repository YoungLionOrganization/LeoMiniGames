// LeoMiniGames installer controller. QtIFW 4.11 / QJSEngine compatible.
function Controller()
{
    installer.setValue("LMGWindowsArch", "__LMG_WINDOWS_ARCH__");
    installer.setValue("LMGCpuProfile", "__LMG_WINDOWS_CPU_PROFILE__");
    installer.setValue("LMGBuildLabel", "__LMG_WINDOWS_BUILD_LABEL__");
    installer.setValue("LMGUpdateRepository", "__LMG_UPDATE_REPOSITORY_URL__");
    installer.setValue("LMGCpuDescription", this.detectCpuDescription());
    this.detectExistingInstallation();
}

Controller.prototype.cleanPath = function(path)
{
    if (!path) return "";
    return String(path).replace(/^\s+|\s+$/g, "").replace(/\\/g, "/").replace(/\/+$/g, "");
}

Controller.prototype.registryValue = function(root, name, view)
{
    if (systemInfo.productType !== "windows") return "";
    var result = installer.execute("reg.exe", ["query", root, "/v", name, view]);
    if (!result || result.length < 2 || Number(result[1]) !== 0) return "";
    var lines = String(result[0]).split(/\r?\n/);
    for (var i = 0; i < lines.length; ++i) {
        var line = lines[i];
        if (line.indexOf(name) < 0) continue;
        var match = line.match(/^\s*[^\s]+\s+REG_[A-Z0-9_]+\s+(.+?)\s*$/i);
        if (match && match[1]) return this.cleanPath(match[1]);
    }
    return "";
}

Controller.prototype.detectCpuDescription = function()
{
    if (systemInfo.productType !== "windows") return systemInfo.currentCpuArchitecture;
    var result = installer.execute("powershell.exe", [
        "-NoLogo", "-NoProfile", "-NonInteractive", "-Command",
        "$c=Get-CimInstance Win32_Processor | Select-Object -First 1; if($c){\"$($c.Manufacturer)|$($c.Name)\"}"
    ]);
    if (result && result.length >= 2 && Number(result[1]) === 0) {
        var value = String(result[0]).replace(/^\s+|\s+$/g, "");
        if (value) return value.replace("|", " — ");
    }
    return systemInfo.currentCpuArchitecture;
}

Controller.prototype.detectLegacyInstalledProduct = function()
{
    if (systemInfo.productType !== "windows") return { path: "", version: "" };
    var command = "$roots=@('HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*','HKLM:\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*','HKCU:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*');$p=Get-ItemProperty $roots -ErrorAction SilentlyContinue | Where-Object { $_.DisplayName -like 'LeoMiniGames*' } | Select-Object -First 1;if($p -and $p.InstallLocation){Write-Output ($p.InstallLocation + '|' + $p.DisplayVersion)}";
    var result = installer.execute("powershell.exe", ["-NoLogo", "-NoProfile", "-NonInteractive", "-Command", command]);
    if (!result || result.length < 2 || Number(result[1]) !== 0) return { path: "", version: "" };
    var value = String(result[0]).replace(/^\s+|\s+$/g, "");
    if (!value) return { path: "", version: "" };
    var split = value.lastIndexOf("|");
    if (split < 0) return { path: this.cleanPath(value), version: "" };
    return { path: this.cleanPath(value.substring(0, split)), version: value.substring(split + 1).replace(/^\s+|\s+$/g, "") };
}

Controller.prototype.detectExistingInstallation = function()
{
    if (systemInfo.productType !== "windows") return;

    var roots = [
        ["HKLM\\Software\\YoungLion\\LeoMiniGames", "/reg:64"],
        ["HKCU\\Software\\YoungLion\\LeoMiniGames", "/reg:64"],
        ["HKLM\\Software\\YoungLion\\LeoMiniGames", "/reg:32"],
        ["HKCU\\Software\\YoungLion\\LeoMiniGames", "/reg:32"]
    ];
    var candidates = [];
    for (var i = 0; i < roots.length; ++i) {
        var found = this.registryValue(roots[i][0], "InstallDir", roots[i][1]);
        if (found) candidates.push(found);
    }
    var legacy = this.detectLegacyInstalledProduct();
    if (legacy.path) candidates.push(legacy.path);
    candidates.push(this.cleanPath(installer.value("TargetDir")));

    var programFiles = installer.environmentVariable("ProgramFiles");
    if (programFiles) candidates.push(this.cleanPath(programFiles + "/LeoMiniGames"));
    var programFilesX86 = installer.environmentVariable("ProgramFiles(x86)");
    if (programFilesX86) candidates.push(this.cleanPath(programFilesX86 + "/LeoMiniGames"));

    for (var c = 0; c < candidates.length; ++c) {
        var dir = this.cleanPath(candidates[c]);
        if (!dir) continue;
        var maintenance = dir + "/LeoMiniGamesMaintenance.exe";
        var app = dir + "/LeoMiniGames.exe";
        if (installer.fileExists(maintenance) && installer.fileExists(app)) {
            installer.setValue("LMGExistingInstallDir", dir);
            installer.setValue("TargetDir", dir);
            var version = "";
            for (var r = 0; r < roots.length && !version; ++r)
                version = this.registryValue(roots[r][0], "Version", roots[r][1]);
            if (!version && legacy.path === dir) version = legacy.version;
            installer.setValue("LMGExistingVersion", version || "installed version");
            return;
        }
    }
}

Controller.prototype.IntroductionPageCallback = function()
{
    var page = gui.currentPageWidget();
    if (!page) return;
    page.title = "LeoMiniGames Setup";
    var existing = installer.value("LMGExistingInstallDir");
    if (page.MessageLabel) {
        if (existing) {
            page.MessageLabel.setText("An existing LeoMiniGames installation was detected. Continue to choose Update, Repair / Reinstall, Modify, or Uninstall.");
        } else {
            page.MessageLabel.setText("Install LeoMiniGames " + installer.value("ProductVersion") + " for Windows.");
        }
    }
    if (page.InformationLabel) {
        page.InformationLabel.setText("Build: " + installer.value("LMGBuildLabel") + "\nProcessor: " + installer.value("LMGCpuDescription"));
    }
}

Controller.prototype.existingAction = function()
{
    var page = gui.pageWidgetByObjectName("DynamicExistingInstallationPage");
    if (!page) return "repair";
    if (page.UninstallRadioButton && page.UninstallRadioButton.checked) return "uninstall";
    if (page.ModifyRadioButton && page.ModifyRadioButton.checked) return "modify";
    if (page.RepairRadioButton && page.RepairRadioButton.checked) return "repair";
    return "upgrade";
}

Controller.prototype.launchMaintenance = function(action)
{
    var dir = installer.value("LMGExistingInstallDir");
    if (!dir) return false;
    var tool = this.cleanPath(dir) + "/LeoMiniGamesMaintenance.exe";
    if (!installer.fileExists(tool)) return false;

    var args = [];
    if (action === "upgrade") {
        args = ["--set-temp-repository", installer.value("LMGUpdateRepository"), "--start-updater"];
    } else if (action === "modify") {
        args = ["--set-temp-repository", installer.value("LMGUpdateRepository"), "--start-package-manager"];
    } else if (action === "uninstall") {
        args = ["--start-uninstaller"];
    } else {
        return false;
    }
    if (!installer.executeDetached(tool, args, dir)) return false;
    gui.rejectWithoutPrompt();
    return true;
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    var page = gui.currentPageWidget();
    var existing = installer.value("LMGExistingInstallDir");
    if (existing) {
        var action = this.existingAction();
        if (action !== "repair") {
            if (!this.launchMaintenance(action) && page && page.WarningLabel)
                page.WarningLabel.setText("Could not launch the installed Maintenance Tool. Choose Repair / Reinstall or close Setup and start LeoMiniGamesMaintenance.exe manually.");
            return;
        }
        if (page && page.TargetDirectoryLineEdit)
            page.TargetDirectoryLineEdit.setText(existing);
        try { installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes); } catch (e) {}
    }
    if (page) {
        page.title = existing ? "Repair / Reinstall" : "Installation Folder";
        if (page.MessageLabel)
            page.MessageLabel.setText(existing ? "Repair uses the existing installation folder and reinstalls the files from this Setup package." : "Choose where LeoMiniGames will be installed.");
    }
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
    installer.selectComponent("xyz.younglion.leominigames");
    var page = gui.currentPageWidget();
    if (page) page.title = "LeoMiniGames Components";
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    installer.selectComponent("xyz.younglion.leominigames");
    var page = gui.currentPageWidget();
    if (!page) return;
    page.title = installer.value("LMGExistingInstallDir") ? "Ready to Repair" : "Ready to Install";
    if (page.InstallMsgLabel)
        page.InstallMsgLabel.setText("You are installing LeoMiniGames " + installer.value("ProductVersion") + " — " + installer.value("LMGBuildLabel"));
    if (page.InstallComponentsTreeview) {
        page.InstallComponentsTreeview.visible = true;
        page.InstallComponentsTreeview.minimumHeight = 130;
    }
    if (page.ComponentSummaryScrollArea)
        page.ComponentSummaryScrollArea.minimumHeight = 180;
}

Controller.prototype.FinishedPageCallback = function()
{
    var page = gui.currentPageWidget();
    if (!page) return;
    page.title = "LeoMiniGames is ready";
    if (page.MessageLabel && installer.status === QInstaller.Success)
        page.MessageLabel.setText("LeoMiniGames " + installer.value("ProductVersion") + " was installed successfully.");
    if (page.RunItCheckBox)
        page.RunItCheckBox.text = "Launch LeoMiniGames";
}
