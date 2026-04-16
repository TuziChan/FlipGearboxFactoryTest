import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""
    property string variant: "default"
    property string iconName: ""
    property bool focusable: false

    implicitHeight: 24
    implicitWidth: Math.max(52, label.implicitWidth + (icon.visible ? 32 : 24))
    radius: 999
    color: variant === "secondary"
           ? theme.surface
           : variant === "outline"
             ? theme.cardColor
             : variant === "destructive"
               ? theme.dangerWeak
               : variant === "ghost"
                 ? "transparent"
                 : theme.accent
    border.width: variant === "outline" ? 1 : 0
    border.color: variant === "outline" ? theme.stroke : "transparent"
    focus: focusable

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: root.activeFocus ? 3 : 0
        border.color: Qt.rgba(root.theme.accent.r, root.theme.accent.g, root.theme.accent.b, 0.5)
        visible: root.activeFocus
    }

    Row {
        anchors.centerIn: parent
        spacing: 4

        AppIcon {
            id: icon
            visible: root.iconName.length > 0
            name: root.iconName
            color: label.color
            iconSize: 12
        }

        Text {
            id: label
            text: root.text
            color: root.variant === "default"
                   ? root.theme.primaryForeground
                   : root.variant === "destructive"
                     ? root.theme.danger
                     : root.variant === "secondary"
                       ? root.theme.textSecondary
                       : root.variant === "ghost"
                         ? root.theme.textPrimary
                         : root.variant === "link"
                           ? root.theme.accent
                           : root.theme.textPrimary
            font.pixelSize: 12
            font.bold: true
            font.underline: root.variant === "link"
        }
    }
}
