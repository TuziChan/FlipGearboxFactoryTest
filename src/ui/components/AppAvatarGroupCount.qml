import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""
    property string size: "default"

    readonly property int avatarSize: size === "sm" ? 24 : size === "lg" ? 40 : 32

    width: avatarSize
    height: avatarSize
    radius: width / 2
    color: root.theme.muted
    border.width: 2
    border.color: root.theme.bgColor

    Text {
        anchors.centerIn: parent
        text: root.text
        color: root.theme.textMuted
        font.pixelSize: size === "sm" ? 9 : 11
        font.bold: true
    }
}
