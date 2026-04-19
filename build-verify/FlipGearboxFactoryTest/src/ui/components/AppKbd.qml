pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""

    implicitHeight: 20
    implicitWidth: Math.max(20, label.implicitWidth + 8)
    radius: root.theme.radiusSmall
    color: root.theme.bgMuted
    border.width: 0

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root.theme.textMuted
        font.pixelSize: 12
        font.weight: Font.Medium
        font.family: "sans-serif"
    }
}
