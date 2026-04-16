pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    objectName: "navRail"

    required property AppTheme theme
    required property var model
    required property int activeIndex
    property bool expanded: false
    signal itemSelected(int index)

    color: root.theme.panelColor
    border.color: root.theme.dividerColor
    implicitWidth: root.expanded ? 184 : 76

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            radius: root.theme.radiusMedium
            color: root.theme.surface
            border.color: root.theme.stroke

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 18
                    Layout.preferredHeight: 18
                    radius: 9
                    color: root.theme.cardColor
                    border.color: root.theme.stroke

                    Text {
                        anchors.centerIn: parent
                        text: root.expanded ? "‹" : "›"
                        color: root.theme.textSecondary
                        font.pixelSize: 12
                        font.bold: true
                    }
                }

                Text {
                    visible: root.expanded
                    text: "导航"
                    color: root.theme.textPrimary
                    font.pixelSize: 11
                    font.bold: true
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.expanded = !root.expanded
            }
        }

        Repeater {
            model: root.model

            delegate: SidebarNavItem {
                required property int index
                required property var modelData
                Layout.fillWidth: true
                theme: root.theme
                expanded: root.expanded
                iconName: modelData.iconName
                text: modelData.title
                active: index === root.activeIndex
                onTriggered: root.itemSelected(index)
            }
        }

        Item { Layout.fillHeight: true }
    }
}
