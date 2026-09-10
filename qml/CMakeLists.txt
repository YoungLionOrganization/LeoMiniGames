set_source_files_properties(Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties(Constants.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties(GameRules.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(LeoMiniGames
    URI LeoMiniGames
    VERSION 1.0
    NO_GENERATE_QMLTYPES
    QML_FILES
        Main.qml
        Theme.qml
        Constants.qml
        GameRules.qml
        ThemeSurface.qml
        ThemePolygonSurface.qml
        TokenText.qml
        TokenPanel.qml
        PressableSurface.qml
        ThemeButton.qml
        ThemeIconButton.qml
        ThemeTextField.qml
        ThemeBadge.qml
        PublisherBadge.qml
        ThemeDivider.qml
        ThemeProgressBar.qml
        ThemeEmptyState.qml
        ThemeToast.qml
        ThemeMarketCard.qml
        FocusRing.qml
        StateLayer.qml
        LoadingShimmer.qml
        KeyHint.qml
        OverlayScrim.qml
        AutoFitText.qml
        NumberTicker.qml
        VirtualJoystick.qml
        ThemeListItem.qml
        DialogueBubble.qml
        SafeAreaItem.qml
        AspectFrame.qml
        TouchTarget.qml
        AdaptiveGrid.qml
        GameBoardSurface.qml
        GameTileSurface.qml
        GameHudBar.qml
        InventorySlot.qml
        VirtualActionPad.qml
        AppBackground.qml
        AppIcon.qml
        GameIcon.qml
        LogoMark.qml
        BrandLogo.qml
        BronzeButton.qml
        BronzeSwitch.qml
        StatChip.qml
        SectionCard.qml
        GameCard.qml
        ModCard.qml
        LibraryPage.qml
        ModsPage.qml
        SettingsPage.qml
        DeveloperLabPage.qml
        ThemesPage.qml
        GameHost.qml
        ModHost.qml
        CardVisual.qml
        XoxPage.qml
        BlackjackPage.qml
        MinesweeperPage.qml
        TwentyFortyEightPage.qml
        MemoryMatchPage.qml
        ReactionTapPage.qml
)
