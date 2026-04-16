import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""

    implicitHeight: 18
    implicitWidth: Math.max(18, label.implicitWidth + 8)
    radius: root.theme.radiusSmall - 2
    color: Qt.rgba(root.theme.textSecondary.r, root.theme.textSecondary.g, root.theme.textSecondary.b, 0.1)
    border.width: 1
    border.color: Qt.rgba(root.theme.textSecondary.r, root.theme.textSecondary.g, root.theme.textSecondary.b, 0.14)

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root.theme.textSecondary
        font.pixelSize: 10
        font.bold: true
    }
}
