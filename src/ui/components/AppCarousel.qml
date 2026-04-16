pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int currentIndex: 0
    property string orientation: "horizontal"
    readonly property bool canScrollPrev: root.currentIndex > 0
    readonly property bool canScrollNext: root.currentIndex < root.itemCount() - 1

    function itemCount() {
        return Array.isArray(root.model) ? root.model.length : 0
    }

    function itemAt(index) {
        return index >= 0 && index < root.itemCount() ? root.model[index] : undefined
    }

    function itemTitleAt(index) {
        const item = root.itemAt(index)
        if (typeof item === "string")
            return item
        return item && (item.title || item.text || item.label) || ""
    }

    function itemDescriptionAt(index) {
        const item = root.itemAt(index)
        return item && item.description ? item.description : ""
    }

    function goTo(index) {
        root.currentIndex = Math.max(0, Math.min(root.itemCount() - 1, index))
        return true
    }

    function previous() {
        return root.goTo(root.currentIndex - 1)
    }

    function next() {
        return root.goTo(root.currentIndex + 1)
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left) {
            root.previous()
            event.accepted = true
        } else if (event.key === Qt.Key_Right) {
            root.next()
            event.accepted = true
        }
    }

    implicitWidth: 320
    implicitHeight: 210

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Item {
            id: viewport
            width: parent.width
            height: 140
            clip: true

            Row {
                id: slideRow
                width: viewport.width * Math.max(1, root.itemCount())
                height: viewport.height
                x: -(root.currentIndex * viewport.width)
                spacing: 0

                Behavior on x {
                    NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
                }

                Repeater {
                    model: root.itemCount()

                    delegate: Rectangle {
                        required property int index
                        width: viewport.width
                        height: viewport.height
                        radius: root.theme.radiusMedium
                        color: root.theme.surface

                        Column {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            Text {
                                text: root.itemTitleAt(index)
                                color: root.theme.textPrimary
                                font.pixelSize: 14
                                font.bold: true
                            }

                            Text {
                                text: root.itemDescriptionAt(index)
                                color: root.theme.textSecondary
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            width: parent.width

            AppButton {
                theme: root.theme
                size: "icon-sm"
                variant: "outline"
                iconName: "chevron-left"
                text: ""
                disabled: !root.canScrollPrev
                onClicked: root.previous()
            }

            Item { Layout.fillWidth: true }

            Row {
                spacing: 6

                Repeater {
                    model: root.itemCount()

                    delegate: Rectangle {
                        required property int index
                        width: 6
                        height: 6
                        radius: 3
                        color: root.currentIndex === index ? root.theme.accent : root.theme.muted
                    }
                }
            }

            Item { Layout.fillWidth: true }

            AppButton {
                theme: root.theme
                size: "icon-sm"
                variant: "outline"
                iconName: "chevron-right"
                text: ""
                disabled: !root.canScrollNext
                onClicked: root.next()
            }
        }
    }
}
