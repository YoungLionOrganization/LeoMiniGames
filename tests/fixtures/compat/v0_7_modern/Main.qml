import QtQuick
Item { Component.onCompleted: { if (GameRuntime.supports("resources")) GameResources.exists("manifest.json") } }
