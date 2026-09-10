function Component() {}
Component.prototype.createOperations = function() {
    component.createOperations();
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/LeoMiniGames.exe", "@StartMenuDir@/LeoMiniGames.lnk");
        component.addOperation("CreateShortcut", "@TargetDir@/LeoMiniGames.exe", "@DesktopDir@/LeoMiniGames.lnk");
    }
}
