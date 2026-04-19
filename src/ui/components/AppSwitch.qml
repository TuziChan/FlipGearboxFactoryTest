pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property bool checked: false
    property string size: "default"
    signal toggled(bool checked)

    readonly property real switchWidth: size === "sm" ? 24 : 28
    readonly property real switchHeight: size === "sm" ? 14 : 16.6
    readonly property real thumbSize: size === "sm" ? 10 : 12

    implicitWidth: switchWidth
    implicitHeight: switchHeight
    radius: height / 2
    color: checked ? root.theme.accent : root.theme.stroke
    border.width: 0

    Rectangle {
        width: root.thumbSize
        height: root.thumbSize
        radius: width / 2
        y: (root.height - height) / 2
        x: root.checked ? root.width - width - 2 : 2
        color: root.checked ? root.theme.primaryForeground : root.theme.cardColor

        Behavior on x {
            NumberAnimation { duration: 120 }
        }
    }

    TapHandler {
        onTapped: {
            root.checked = !root.checked
            root.toggled(root.checked)
        }
    }
}
