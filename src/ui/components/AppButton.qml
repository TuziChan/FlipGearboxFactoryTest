import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""
    property string variant: "default"
    property string size: "default"
    property bool disabled: false
    property bool block: false
    property string iconName: ""
    property string iconPosition: "start"
    signal clicked

    readonly property bool primaryVariant: variant === "default" || variant === "primary"
    readonly property bool dangerVariant: variant === "destructive" || variant === "danger"
    readonly property bool secondaryVariant: variant === "secondary"
    readonly property bool outlineVariant: variant === "outline"
    readonly property bool ghostVariant: variant === "ghost"
    readonly property bool linkVariant: variant === "link"
    readonly property bool iconOnly: size === "icon" || size === "icon-sm" || size === "icon-lg" || size === "icon-xs"

    implicitWidth: block ? 160 :
                   size === "xs" ? 68 :
                   size === "sm" ? 84 :
                   size === "lg" ? 118 :
                   size === "icon-xs" ? 24 :
                   size === "icon-sm" ? 32 :
                   size === "icon" ? 36 :
                   size === "icon-lg" ? 40 : 104
    implicitHeight: size === "xs" ? 24 :
                    size === "sm" ? 32 :
                    size === "lg" ? 40 :
                    size === "icon-xs" ? 24 :
                    size === "icon-sm" ? 32 :
                    size === "icon" ? 36 :
                    size === "icon-lg" ? 40 : 36
    radius: theme.radiusMedium
    color: disabled
           ? theme.muted
           : dangerVariant
             ? theme.dangerWeak
             : primaryVariant
               ? theme.accent
               : outlineVariant
                 ? theme.cardColor
                 : secondaryVariant
                   ? theme.surface
                   : ghostVariant
                     ? "transparent"
                     : linkVariant
                       ? "transparent"
                       : theme.cardColor
    border.width: outlineVariant || secondaryVariant ? 1 : 0
    border.color: disabled
                  ? theme.stroke
                  : outlineVariant
                    ? theme.stroke
                    : secondaryVariant
                      ? theme.stroke
                      : "transparent"
    opacity: disabled ? 0.6 : 1
    focus: true

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: root.activeFocus ? 3 : 0
        border.color: Qt.rgba(root.theme.accent.r, root.theme.accent.g, root.theme.accent.b, 0.5)
        visible: root.activeFocus
    }

    Behavior on color {
        ColorAnimation { duration: 150 }
    }

    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: root.size === "xs" ? 4 : 6

        AppIcon {
            visible: root.iconName.length > 0 && root.iconPosition === "start"
            name: root.iconName
            color: label.color
            iconSize: root.size === "xs" || root.size === "icon-xs" ? 12 :
                      root.size === "sm" || root.size === "icon-sm" ? 16 :
                      root.size === "lg" || root.size === "icon-lg" ? 16 : 16
        }

        Text {
            id: label
            visible: !root.iconOnly
            text: root.text
            color: root.disabled
                   ? root.theme.textMuted
                   : (root.primaryVariant
                      ? root.theme.primaryForeground
                      : root.dangerVariant
                        ? root.theme.danger
                        : root.linkVariant
                          ? root.theme.accent
                          : root.theme.textPrimary)
            font.pixelSize: root.size === "xs" ? 12 : 14
            font.bold: true
            font.underline: root.linkVariant
        }

        AppIcon {
            visible: root.iconName.length > 0 && root.iconPosition === "end"
            name: root.iconName
            color: label.color
            iconSize: root.size === "xs" || root.size === "icon-xs" ? 12 :
                      root.size === "sm" || root.size === "icon-sm" ? 16 :
                      root.size === "lg" || root.size === "icon-lg" ? 16 : 16
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.iconOnly && root.iconName.length === 0
        text: root.text
        color: root.disabled
               ? root.theme.textMuted
               : (root.primaryVariant
                  ? root.theme.primaryForeground
                  : root.dangerVariant
                    ? root.theme.danger
                    : root.theme.textPrimary)
        font.pixelSize: 14
        font.bold: true
    }

    AppIcon {
        anchors.centerIn: parent
        visible: root.iconOnly && root.iconName.length > 0
        name: root.iconName
        color: root.disabled
               ? root.theme.textMuted
               : (root.primaryVariant
                  ? root.theme.primaryForeground
                  : root.dangerVariant
                    ? root.theme.danger
                    : root.theme.textPrimary)
        iconSize: root.size === "icon-xs" ? 12 :
                  root.size === "icon-sm" ? 16 :
                  root.size === "icon-lg" ? 16 : 16
    }

    MouseArea {
        anchors.fill: parent
        enabled: !root.disabled
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
