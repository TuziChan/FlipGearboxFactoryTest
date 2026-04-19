pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property real value: 0
    property real maximum: 100
    property bool indeterminate: false

    implicitWidth: 180
    implicitHeight: 8
    radius: 999
    color: root.theme.muted

    Rectangle {
        visible: !root.indeterminate
        width: Math.max(0, Math.min(root.width, root.maximum > 0 ? root.width * (root.value / root.maximum) : 0))
        height: parent.height
        radius: parent.radius
        color: root.theme.accent
    }

    Rectangle {
        id: indeterminateBar
        visible: root.indeterminate
        width: parent.width * 0.3
        height: parent.height
        radius: parent.radius
        color: root.theme.accent

        SequentialAnimation on x {
            running: root.indeterminate
            loops: Animation.Infinite
            NumberAnimation {
                from: -indeterminateBar.width
                to: root.width
                duration: 1200
                easing.type: Easing.InOutCubic
            }
        }
    }
}
