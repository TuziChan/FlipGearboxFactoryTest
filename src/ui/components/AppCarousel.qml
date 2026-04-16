pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int currentIndex: 0
    property string orientation: "horizontal"
    property bool showControls: true
    property bool showIndicators: true
    property string buttonVariant: "outline"
    property string buttonSize: "icon-sm"
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
    implicitHeight: root.showControls ? 210 : 180

    Item {
        id: viewport
        anchors.fill: parent
        clip: true

        Row {
            id: slideRow
            width: viewport.width * Math.max(1, root.itemCount())
            height: viewport.height
            x: -(root.currentIndex * viewport.width)
            spacing: 0

            Behavior on x {
                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
            }

            Repeater {
                model: root.itemCount()

                delegate: Item {
                    required property int index
                    width: viewport.width
                    height: viewport.height

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 16
                        radius: root.theme.radiusMedium
                        color: root.theme.mutedColor
                        border.width: 1
                        border.color: root.theme.borderColor

                        Column {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            Text {
                                width: parent.width
                                text: root.itemTitleAt(index)
                                color: root.theme.textPrimary
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }

                            Text {
                                width: parent.width
                                text: root.itemDescriptionAt(index)
                                color: root.theme.textSecondary
                                font.pixelSize: 14
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }
    }

    AppButton {
        visible: root.showControls
        theme: root.theme
        size: root.buttonSize
        variant: root.buttonVariant
        iconName: "chevron-left"
        text: ""
        disabled: !root.canScrollPrev
        anchors.left: parent.left
        anchors.leftMargin: -48
        anchors.verticalCenter: parent.verticalCenter
        onClicked: root.previous()
    }

    AppButton {
        visible: root.showControls
        theme: root.theme
        size: root.buttonSize
        variant: root.buttonVariant
        iconName: "chevron-right"
        text: ""
        disabled: !root.canScrollNext
        anchors.right: parent.right
        anchors.rightMargin: -48
        anchors.verticalCenter: parent.verticalCenter
        onClicked: root.next()
    }

    Row {
        visible: root.showIndicators
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        Repeater {
            model: root.itemCount()

            delegate: Rectangle {
                required property int index
                width: 8
                height: 8
                radius: 4
                color: root.currentIndex === index ? root.theme.accent : root.theme.mutedColor
                border.width: 1
                border.color: root.currentIndex === index ? "transparent" : root.theme.borderColor

                Behavior on color {
                    ColorAnimation { duration: 200 }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.goTo(index)
                }
            }
        }
    }
}
