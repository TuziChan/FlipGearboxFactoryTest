pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string text: ""
    property string variant: "default"
    property string size: "default"
    property bool checked: false
    property bool autoToggle: true
    property bool disabled: false
    property string iconName: ""
    property string iconPosition: "start"
    signal clicked
    signal toggled(bool checked)
    signal changed(bool checked)

    readonly property bool outlineVariant: variant === "outline"
    readonly property bool smallSize: size === "sm"
    readonly property bool largeSize: size === "lg"
    readonly property bool iconOnly: text.length === 0 && iconName.length > 0
    readonly property bool pressed: toggleTap.pressed
    readonly property bool hovered: toggleHover.hovered
    readonly property int controlHeight: smallSize ? 32 : (largeSize ? 40 : 36)
    readonly property int minimumWidth: smallSize ? 32 : (largeSize ? 40 : 36)
    readonly property int horizontalPadding: smallSize ? 6 : (largeSize ? 10 : 8)
    readonly property int iconSize: smallSize ? 12 : (largeSize ? 18 : 16)
    readonly property int labelSize: smallSize ? 11 : 12
    readonly property int cornerRadius: smallSize ? Math.min(theme.radiusMedium, 8) : theme.radiusMedium
    readonly property color baseInteractiveColor: checked ? theme.accentWeak : "transparent"
    readonly property color hoverInteractiveColor: checked ? theme.accentWeak : theme.muted
    readonly property color pressInteractiveColor: checked
                                               ? Qt.darker(theme.accentWeak, theme.darkMode ? 1.08 : 1.04)
                                               : Qt.darker(theme.muted, theme.darkMode ? 1.08 : 1.04)
    readonly property color currentBackground: disabled
                                               ? (checked ? theme.accentWeak : "transparent")
                                               : (pressed
                                                  ? pressInteractiveColor
                                                  : (hovered ? hoverInteractiveColor : baseInteractiveColor))
    readonly property color currentBorderColor: disabled
                                                ? theme.stroke
                                                : (outlineVariant ? theme.stroke : "transparent")
    readonly property color contentColor: disabled
                                          ? theme.textMuted
                                          : (checked ? theme.accentForeground : theme.textPrimary)

    function activate() {
        if (root.disabled)
            return

        const nextChecked = !root.checked

        if (root.autoToggle)
            root.checked = nextChecked

        root.toggled(nextChecked)
        root.changed(nextChecked)
        root.clicked()
    }

    implicitWidth: Math.max(minimumWidth, contentRow.implicitWidth + (horizontalPadding * 2))
    implicitHeight: controlHeight
    radius: cornerRadius
    color: currentBackground
    border.width: outlineVariant ? 1 : 0
    border.color: currentBorderColor
    opacity: disabled ? 0.5 : 1

    Behavior on color {
        ColorAnimation { duration: 120 }
    }

    Behavior on border.color {
        ColorAnimation { duration: 120 }
    }

    activeFocusOnTab: !disabled
    focus: false
    Keys.onSpacePressed: function(event) {
        root.activate()
        event.accepted = true
    }
    Keys.onReturnPressed: function(event) {
        root.activate()
        event.accepted = true
    }
    Keys.onEnterPressed: function(event) {
        root.activate()
        event.accepted = true
    }

    RowLayout {
        id: contentRow

        anchors.centerIn: parent
        spacing: root.text.length > 0 && root.iconName.length > 0 ? 6 : 0

        AppIcon {
            visible: root.iconName.length > 0 && root.iconPosition === "start"
            name: root.iconName
            color: root.contentColor
            iconSize: root.iconSize
        }

        Text {
            visible: root.text.length > 0
            text: root.text
            color: root.contentColor
            font.pixelSize: root.labelSize
            font.bold: true
            verticalAlignment: Text.AlignVCenter
        }

        AppIcon {
            visible: root.iconName.length > 0 && root.iconPosition === "end"
            name: root.iconName
            color: root.contentColor
            iconSize: root.iconSize
        }
    }
}
