pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int openIndex: -1
    property string selectedText: ""
    property bool showViewport: true
    signal itemTriggered(int menuIndex, int itemIndex, string text)

    function itemCount(menuIndex) {
        const item = root.model[menuIndex]
        return item && Array.isArray(item.items) ? item.items.length : 0
    }

    function itemAt(menuIndex, itemIndex) {
        const item = root.model[menuIndex]
        return item && Array.isArray(item.items) ? item.items[itemIndex] : undefined
    }

    function openMenu(index) {
        root.openIndex = index
        return true
    }

    function closeMenu() {
        root.openIndex = -1
        return true
    }

    function selectItem(index) {
        const item = root.itemAt(root.openIndex, index)
        if (!item)
            return false
        root.selectedText = item.title || item.text || item.label || ""
        root.itemTriggered(root.openIndex, index, root.selectedText)
        root.closeMenu()
        return true
    }

    implicitWidth: Math.max(navRow.implicitWidth, panel.implicitWidth)
    implicitHeight: navRow.implicitHeight + indicator.height + panel.implicitHeight + 8

    Column {
        anchors.fill: parent
        spacing: 4

        Row {
            id: navRow
            spacing: 6

            Repeater {
                id: triggerRepeater
                model: root.model

                delegate: AppButton {
                    required property int index
                    required property var modelData
                    theme: root.theme
                    text: modelData.text || modelData.label || modelData.title || ""
                    variant: root.openIndex === index ? "outline" : "ghost"
                    onClicked: root.openIndex === index ? root.closeMenu() : root.openMenu(index)
                }
            }
        }

        Rectangle {
            id: indicator
            visible: root.openIndex >= 0
            width: triggerRepeater.itemAt(root.openIndex) ? triggerRepeater.itemAt(root.openIndex).width : 0
            height: 3
            radius: 999
            color: root.theme.accent
            x: triggerRepeater.itemAt(root.openIndex) ? triggerRepeater.itemAt(root.openIndex).x : 0

            Behavior on x {
                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
            }

            Behavior on width {
                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
            }
        }

        Rectangle {
            id: panel
            visible: root.openIndex >= 0
            width: Math.min(parent.width, 320)
            implicitHeight: listColumn.implicitHeight + 20
            radius: root.theme.radiusLarge
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.dividerColor
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: visible ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: 120 }
            }

            Column {
                id: listColumn
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Repeater {
                    model: root.openIndex >= 0 ? root.itemCount(root.openIndex) : 0

                    delegate: Rectangle {
                        id: navItemDelegate
                        required property int index
                        width: listColumn.width
                        height: 48
                        radius: root.theme.radiusSmall
                        color: itemMouse.containsMouse ? root.theme.accentWeak : "transparent"

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 2

                            Text {
                                text: root.itemAt(root.openIndex, navItemDelegate.index).title || root.itemAt(root.openIndex, navItemDelegate.index).text || ""
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                visible: (root.itemAt(root.openIndex, navItemDelegate.index).description || "").length > 0
                                text: root.itemAt(root.openIndex, navItemDelegate.index).description || ""
                                color: root.theme.textMuted
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                        }

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: root.selectItem(navItemDelegate.index)
                        }
                    }
                }
            }
        }
    }
}
