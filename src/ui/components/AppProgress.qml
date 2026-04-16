import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property real value: 0
    property real maximum: 100

    implicitWidth: 180
    implicitHeight: 8
    radius: 999
    color: root.theme.muted

    Rectangle {
        width: Math.max(0, Math.min(root.width, root.maximum > 0 ? root.width * (root.value / root.maximum) : 0))
        height: parent.height
        radius: parent.radius
        color: root.theme.accent
    }
}
