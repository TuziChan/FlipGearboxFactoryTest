pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int currentIndex: 0
    property string variant: "default"
    property string orientation: "horizontal"
    property bool showContent: true
    readonly property string currentText: root.tabContentAt(root.currentIndex)

    function itemCount() {
        return Array.isArray(root.model) ? root.model.length : 0
    }

    function itemAt(index) {
        return index >= 0 && index < root.itemCount() ? root.model[index] : undefined
    }

    function tabTextAt(index) {
        const item = root.itemAt(index)
        if (typeof item === "string")
            return item
        return item && (item.text || item.label || item.title) || ""
    }

    function tabContentAt(index) {
        const item = root.itemAt(index)
        if (!item || typeof item === "string")
            return ""
        return item.content || item.description || item.body || ""
    }

    function setCurrentIndex(index) {
        if (index < 0 || index >= root.itemCount())
            return false
        root.currentIndex = index
        return true
    }

    function moveTab(delta) {
        return root.setCurrentIndex(Math.max(0, Math.min(root.itemCount() - 1, root.currentIndex + delta)))
    }

    Keys.onPressed: function(event) {
        if (root.orientation === "horizontal") {
            if (event.key === Qt.Key_Right) {
                root.moveTab(1)
                event.accepted = true
            } else if (event.key === Qt.Key_Left) {
                root.moveTab(-1)
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Down) {
            root.moveTab(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Up) {
            root.moveTab(-1)
            event.accepted = true
        }
    }

    implicitWidth: root.orientation === "horizontal" ? 280 : 220
    implicitHeight: root.orientation === "horizontal"
                    ? (root.showContent ? tabsColumn.implicitHeight : listFrame.implicitHeight)
                    : tabsRow.implicitHeight

    ColumnLayout {
        id: tabsColumn
        anchors.fill: parent
        spacing: 10
        visible: root.orientation === "horizontal"

        Rectangle {
            id: listFrame
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: false
            radius: root.variant === "line" ? 0 : root.theme.radiusLarge
            color: root.variant === "default" ? root.theme.muted : "transparent"
            border.width: root.variant === "line" ? 0 : 1
            border.color: root.theme.dividerColor
            implicitHeight: 34
            implicitWidth: listRow.implicitWidth + 6

            Row {
                id: listRow
                anchors.fill: parent
                anchors.margins: 3
                spacing: root.variant === "line" ? 6 : 3

                Repeater {
                    model: root.itemCount()

                    delegate: Rectangle {
                        id: tabTrigger
                        required property int index
                        width: Math.max(56, triggerLabel.implicitWidth + 18)
                        height: parent.height
                        radius: root.theme.radiusMedium
                        color: root.currentIndex === index && root.variant === "default" ? root.theme.cardColor : "transparent"

                        Rectangle {
                            visible: root.variant === "line" && root.currentIndex === tabTrigger.index
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 2
                            color: root.theme.textPrimary
                        }

                        Text {
                            id: triggerLabel
                            anchors.centerIn: parent
                            text: root.tabTextAt(tabTrigger.index)
                            color: root.currentIndex === tabTrigger.index ? root.theme.textPrimary : root.theme.textSecondary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        TapHandler {
                            onTapped: root.setCurrentIndex(tabTrigger.index)
                        }
                    }
                }
            }
        }

        Rectangle {
            visible: root.showContent
            Layout.fillWidth: true
            radius: root.theme.radiusLarge
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.dividerColor
            implicitHeight: Math.max(56, contentText.implicitHeight + 24)

            Text {
                id: contentText
                anchors.fill: parent
                anchors.margins: 12
                text: root.currentText
                color: root.theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                visible: root.currentText.length > 0
            }
        }
    }

    RowLayout {
        id: tabsRow
        anchors.fill: parent
        spacing: 10
        visible: root.orientation === "vertical"

        Rectangle {
            Layout.fillHeight: true
            radius: root.variant === "line" ? 0 : root.theme.radiusLarge
            color: root.variant === "default" ? root.theme.muted : "transparent"
            border.width: root.variant === "line" ? 0 : 1
            border.color: root.theme.dividerColor
            implicitWidth: 120

            Column {
                anchors.fill: parent
                anchors.margins: 3
                spacing: root.variant === "line" ? 6 : 3

                Repeater {
                    model: root.itemCount()

                    delegate: Rectangle {
                        id: verticalTrigger
                        required property int index
                        width: parent.width
                        height: 28
                        radius: root.theme.radiusMedium
                        color: root.currentIndex === index && root.variant === "default" ? root.theme.cardColor : "transparent"

                        Rectangle {
                            visible: root.variant === "line" && root.currentIndex === verticalTrigger.index
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            width: 2
                            color: root.theme.textPrimary
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            text: root.tabTextAt(verticalTrigger.index)
                            color: root.currentIndex === verticalTrigger.index ? root.theme.textPrimary : root.theme.textSecondary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        TapHandler {
                            onTapped: root.setCurrentIndex(verticalTrigger.index)
                        }
                    }
                }
            }
        }

        Rectangle {
            visible: root.showContent
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radiusLarge
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.dividerColor

            Text {
                anchors.fill: parent
                anchors.margins: 12
                text: root.currentText
                color: root.theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                visible: root.currentText.length > 0
            }
        }
    }
}
