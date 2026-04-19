pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property string iconName
    required property string text
    property bool active: false
    property bool expanded: true
    signal triggered

    implicitHeight: 34
    radius: theme.radiusSmall
    color: root.active ? theme.accentWeak : mouse.hovered ? theme.surface : "transparent"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.expanded ? 10 : 0
        anchors.rightMargin: 10
        spacing: 8

        Rectangle {
            Layout.preferredWidth: 3
            Layout.preferredHeight: 14
            radius: 2
            color: root.active ? root.theme.accent : "transparent"
        }

        Item {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20

            AppIcon {
                anchors.centerIn: parent
                name: root.iconName
                color: root.active ? root.theme.accent : "#5E636A"
                iconSize: 20
            }
        }

        Text {
            Layout.fillWidth: true
            visible: root.expanded
            text: root.text
            color: root.active ? root.theme.accent : "#3E434A"
            font.pixelSize: 11
            font.bold: root.active
            elide: Text.ElideRight
        }
    }

    TapHandler {
        onTapped: root.triggered()
    }

    HoverHandler {
        id: mouse
    }
}
