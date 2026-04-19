pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""
    property string variant: "default"
    property string iconName: ""
    property bool focusable: false

    implicitHeight: 22
    implicitWidth: Math.max(48, label.implicitWidth + (icon.visible ? 28 : 20))
    radius: height / 2

    color: {
        if (variant === "default") return theme.accent
        if (variant === "secondary") return theme.secondary
        if (variant === "destructive") return theme.danger
        if (variant === "outline") return theme.cardColor
        if (variant === "success") return theme.ok
        if (variant === "warning") return theme.warn
        if (variant === "ghost") return "transparent"
        if (variant === "link") return "transparent"
        return theme.accent
    }

    border.width: (variant === "outline" || variant === "ghost") ? 1 : 0
    border.color: variant === "outline" ? theme.borderColor : "transparent"

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
            color: {
                if (root.variant === "default") return root.theme.primaryForeground
                if (root.variant === "secondary") return root.theme.secondaryForeground
                if (root.variant === "destructive") return root.theme.dangerForeground
                if (root.variant === "outline") return root.theme.textPrimary
                if (root.variant === "success") return root.theme.okForeground
                if (root.variant === "warning") return root.theme.warnForeground
                if (root.variant === "ghost") return root.theme.textPrimary
                if (root.variant === "link") return root.theme.accent
                return root.theme.textPrimary
            }
            font.pixelSize: 11
            font.weight: Font.Medium
            font.underline: root.variant === "link"
        }
    }
}
