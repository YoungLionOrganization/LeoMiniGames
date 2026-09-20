function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGames.exe",
            "@StartMenuDir@/LeoMiniGames.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Launch LeoMiniGames");
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGames.exe",
            "@DesktopDir@/LeoMiniGames.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Launch LeoMiniGames");
        component.addOperation("CreateShortcut",
            "@TargetDir@/LeoMiniGamesMaintenance.exe",
            "@StartMenuDir@/LeoMiniGames Maintenance.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/LeoMiniGames.exe",
            "iconId=0",
            "description=Update, repair, modify or uninstall LeoMiniGames");
    }
}
