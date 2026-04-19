pragma ComponentBehavior: Bound

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
           ? Qt.tint(theme.muted, Qt.rgba(0, 0, 0, 0.3))
           : dangerVariant
             ? theme.danger
             : primaryVariant
               ? theme.accent
               : outlineVariant
                 ? Qt.tint(theme.cardColor, Qt.rgba(theme.textPrimary.r, theme.textPrimary.g, theme.textPrimary.b, 0.05))
                 : secondaryVariant
                   ? theme.secondary
                   : ghostVariant
                     ? "transparent"
                     : linkVariant
                       ? "transparent"
                       : theme.cardColor
    border.width: outlineVariant ? 1 : 0
    border.color: disabled
                  ? Qt.rgba(theme.stroke.r, theme.stroke.g, theme.stroke.b, 0.5)
                  : outlineVariant
                    ? theme.borderColor
                    : "transparent"
    opacity: disabled ? 0.5 : 1
    enabled: !disabled
    activeFocusOnTab: !disabled

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: root.activeFocus ? 2 : 0
        border.color: root.theme.accent
        visible: root.activeFocus
    }

    // shadcn/ui 风格的 hover 效果层
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: Qt.rgba(
            root.primaryVariant ? 1 : 0,
            root.primaryVariant ? 1 : 0,
            root.primaryVariant ? 1 : 0,
            root.primaryVariant ? 0.1 :
            root.dangerVariant ? 0.1 :
            root.secondaryVariant ? 0.1 :
            root.outlineVariant ? 0.05 :
            root.ghostVariant ? 0.05 :
            0.05
        )
        visible: buttonHover.hovered && !root.disabled
        opacity: 0.8
    }

    Behavior on color {
        ColorAnimation { duration: 150; easing.type: Easing.InOutQuad }
    }

    Behavior on opacity {
        NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
    }

    RowLayout {
        id: contentLayout
        anchors.centerIn: parent
        spacing: root.size === "xs" ? 4 : 6

        Behavior on spacing {
            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
        }

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
                        ? root.theme.dangerForeground
                        : root.linkVariant
                          ? root.theme.accent
                          : root.theme.textPrimary)
            font.pixelSize: root.size === "xs" ? 12 : 14
            font.weight: Font.Medium
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
                    ? root.theme.dangerForeground
                    : root.theme.textPrimary)
        font.pixelSize: 14
        font.weight: Font.Medium
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
                    ? root.theme.dangerForeground
                    : root.theme.textPrimary)
        iconSize: root.size === "icon-xs" ? 12 :
                  root.size === "icon-sm" ? 16 :
                  root.size === "icon-lg" ? 16 : 16
    }

    TapHandler {
        enabled: !root.disabled
        onTapped: root.clicked()
    }

    HoverHandler {
        id: buttonHover
        enabled: !root.disabled
    }
}
