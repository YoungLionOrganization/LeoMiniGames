import QtQuick
import QtQuick.Controls
Item {
 id: root; anchors.fill: parent
 Rectangle { anchors.fill: parent; color: "#120b07" }
 Column { anchors.centerIn: parent; spacing: 16
  Image { width: 96; height: 96; source: "assets/icon.png"; anchors.horizontalCenter: parent.horizontalCenter }
  Text { text: "Hello from an RCC mod"; color: "#f3dcb0"; font.pixelSize: 22 }
  Button { text: "Play mod sound"; anchors.horizontalCenter: parent.horizontalCenter; onClicked: Audio.playUrl(Qt.resolvedUrl("sfx/click.wav")) }
 }
 Component.onCompleted: Audio.preload(Qt.resolvedUrl("sfx/click.wav"))
}
