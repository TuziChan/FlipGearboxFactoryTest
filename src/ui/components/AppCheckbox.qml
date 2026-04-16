pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property bool checked: false
    property bool disabled: false
    property bool invalid: false
    signal toggled(bool checked)

    implicitWidth: 16
    implicitHeight: 16
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
        radius: 4
        color: root.checked ? root.theme.accent : root.theme.cardColor
        border.width: 1
        border.color: root.invalid
                      ? root.theme.danger
                      : root.checked || root.activeFocus
                        ? root.theme.accent
                        : root.theme.stroke
        opacity: root.disabled ? 0.5 : 1

        AppIcon {
            anchors.centerIn: parent
            visible: root.checked
            name: "check"
            color: root.theme.primaryForeground
            iconSize: 12
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: root.activeFocus ? 2 : 0
            border.color: Qt.rgba(root.theme.accent.r, root.theme.accent.g, root.theme.accent.b, 0.25)
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: !root.disabled
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
