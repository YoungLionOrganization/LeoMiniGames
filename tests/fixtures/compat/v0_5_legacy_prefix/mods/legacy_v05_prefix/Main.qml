import QtQuick
Item { Component.onCompleted: { Settings.contains("legacyValue"); Audio.stopAll(); Lifecycle.save() } }
