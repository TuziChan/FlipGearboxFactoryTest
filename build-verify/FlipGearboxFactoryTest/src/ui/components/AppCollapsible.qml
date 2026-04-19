pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property bool disabled: false
    property bool showIndicator: true
    property bool fillWidth: true
    property string title: ""
    property string description: ""
    property int headerPadding: 10
    property int contentPadding: 10
    default property alias content: contentColumn.data
    property alias triggerContent: triggerContentRow.data
    signal toggled(bool open)

    readonly property bool hovered: triggerArea.hovered
    readonly property bool pressed: triggerTap.pressed
    readonly property color headerColor: disabled
                                        ? theme.muted
                                        : (pressed
                                           ? Qt.darker(theme.muted, theme.darkMode ? 1.08 : 1.03)
                                           : (hovered ? theme.accentWeak : "transparent"))
    readonly property color borderColor: activeFocus
                                        ? theme.accent
                                        : theme.dividerColor

    function setOpen(nextOpen) {
        if (root.disabled || root.open === nextOpen)
            return false
        root.open = nextOpen
        root.toggled(root.open)
        return true
    }

    function toggle() {
        return root.setOpen(!root.open)
    }

    function openCollapsible() {
        return root.setOpen(true)
    }

    function closeCollapsible() {
        return root.setOpen(false)
    }

    implicitWidth: Math.max(triggerBackground.implicitWidth, contentFrame.implicitWidth)
    implicitHeight: layout.implicitHeight
    activeFocusOnTab: !root.disabled

    Keys.onPressed: function(event) {
        if (root.disabled)
            return

        if (event.key === Qt.Key_Space || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            root.toggle()
            event.accepted = true
        }
    }

    Column {
        id: layout

        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: triggerBackground

            width: root.fillWidth && root.width > 0 ? parent.width : implicitWidth
            implicitWidth: Math.max(titleColumn.implicitWidth, triggerContentRow.implicitWidth) + indicator.implicitWidth + (root.headerPadding * 2) + 12
            implicitHeight: Math.max(40, Math.max(titleColumn.implicitHeight, triggerContentRow.implicitHeight) + (root.headerPadding * 2))
            radius: root.theme.radiusMedium
            color: root.headerColor
            border.width: 1
            border.color: root.borderColor
            opacity: root.disabled ? 0.6 : 1

            Behavior on color {
                ColorAnimation { duration: 120 }
            }

            Behavior on border.color {
                ColorAnimation { duration: 120 }
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: root.headerPadding
                spacing: 12

                Column {
                    id: titleColumn

                    Layout.fillWidth: true
                    spacing: 2
                    visible: root.title.length > 0 || root.description.length > 0

                    Text {
                        visible: root.title.length > 0
                        text: root.title
                        color: root.theme.textPrimary
                        font.pixelSize: 12
                        font.bold: true
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        visible: root.description.length > 0
                        text: root.description
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    id: triggerContentRow

                    Layout.fillWidth: true
                    spacing: 8
                }

                AppIcon {
                    id: indicator

                    Layout.alignment: Qt.AlignTop
                    visible: root.showIndicator
                    name: "chevron-down"
                    color: root.disabled ? root.theme.textMuted : root.theme.textSecondary
                    iconSize: 14
                    rotation: root.open ? 180 : 0

                    Behavior on rotation {
                        NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
                    }
                }
            }

            TapHandler {
                id: triggerTap
                enabled: !root.disabled
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onTapped: root.toggle()
            }

            HoverHandler {
                id: triggerArea
                enabled: !root.disabled
            }
        }

        Item {
            id: contentViewport

            width: root.fillWidth && root.width > 0 ? parent.width : contentFrame.implicitWidth
            implicitHeight: height
            height: root.open ? contentFrame.implicitHeight : 0
            clip: true
            visible: height > 0 || contentOpacity.running
            opacity: root.open ? 1 : 0

            Behavior on height {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            Behavior on opacity {
                NumberAnimation {
                    id: contentOpacity
                    duration: root.open ? 120 : 90
                    easing.type: Easing.OutCubic
                }
            }

            Rectangle {
                id: contentFrame

                width: parent.width
                implicitWidth: contentColumn.implicitWidth + (root.contentPadding * 2)
                implicitHeight: contentColumn.implicitHeight + (root.contentPadding * 2) + 4
                y: 4
                radius: root.theme.radiusMedium
                color: Qt.rgba(root.theme.cardColor.r, root.theme.cardColor.g, root.theme.cardColor.b, root.theme.darkMode ? 0.9 : 1)
                border.width: 1
                border.color: root.theme.dividerColor

                Column {
                    id: contentColumn

                    anchors.fill: parent
                    anchors.margins: root.contentPadding
                    spacing: root.theme.spacingSmall
                }
            }
        }
    }
}
