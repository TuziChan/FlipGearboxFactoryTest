pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string title: ""
    property string description: ""
    property real panelWidth: 260
    property int openDelay: 120
    property int closeDelay: 80
    property real sideOffset: 4
    default property alias content: contentColumn.data

    function showCard() {
        closeTimer.stop()
        if (root.openDelay > 0)
            openTimer.restart()
        else
            root.open = true
        return true
    }

    function hideCard() {
        openTimer.stop()
        if (root.closeDelay > 0)
            closeTimer.restart()
        else
            root.open = false
        return true
    }

    Timer {
        id: openTimer
        interval: root.openDelay
        onTriggered: root.open = true
    }

    Timer {
        id: closeTimer
        interval: root.closeDelay
        onTriggered: root.open = false
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open
        y: root.sideOffset
        width: root.panelWidth
        implicitHeight: panelColumn.implicitHeight + 24
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor

        Column {
            id: panelColumn
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

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

            Column {
                id: contentColumn
                width: parent.width
                spacing: 8
            }
        }
    }
}
