pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property bool checked: false
    property bool disabled: false
    property bool invalid: false
    property string size: "default"
    signal toggled(bool checked)

    implicitWidth: root.size === "sm" ? 14 : 16
    implicitHeight: root.size === "sm" ? 14 : 16
    activeFocusOnTab: !root.disabled

    function setChecked(nextChecked) {
        if (root.disabled || root.checked === nextChecked)
            return false
        root.checked = nextChecked
        root.toggled(root.checked)
        return true
    }

    function toggle() {
        return root.setChecked(!root.checked)
    }

    Rectangle {
        id: indicator
        anchors.fill: parent
        radius: root.theme.radiusSmall
        color: root.checked ? root.theme.accent : root.theme.background
        border.width: root.checked ? 0 : 1
        border.color: root.invalid
                      ? root.theme.danger
                      : root.activeFocus
                        ? root.theme.ringColor
                        : root.theme.borderColor
        opacity: root.disabled ? 0.5 : 1

        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        AppIcon {
            anchors.centerIn: parent
            visible: root.checked
            name: "check"
            color: root.theme.primaryForeground
            iconSize: root.size === "sm" ? 10 : 12
            opacity: root.checked ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: -3
        radius: root.theme.radiusSmall
        color: "transparent"
        border.width: root.activeFocus ? 3 : 0
        border.color: Qt.rgba(root.theme.ringColor.r, root.theme.ringColor.g, root.theme.ringColor.b, 0.5)
        visible: root.activeFocus

        Behavior on border.width {
            NumberAnimation { duration: 150 }
        }
    }

    MouseArea {
        anchors.fill: parent
        anchors.margins: -8
        enabled: !root.disabled
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.toggle()
    }

    Keys.onPressed: function(event) {
        if (root.disabled)
            return

        if (event.key === Qt.Key_Space || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            root.toggle()
            event.accepted = true
        }
    }
}
