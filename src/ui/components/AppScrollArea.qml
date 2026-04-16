pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property AppTheme theme
    default property alias content: contentColumn.data
    readonly property bool atVerticalEnd: flick.contentY >= Math.max(0, flick.contentHeight - flick.height - 1)

    function scrollToEnd() {
        flick.contentY = Math.max(0, flick.contentHeight - flick.height)
        return true
    }

    implicitWidth: 260
    implicitHeight: 180

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 1
        clip: true
        contentWidth: width
        contentHeight: contentColumn.height
        boundsBehavior: Flickable.StopAtBounds
        acceptedButtons: Qt.NoButton

        Column {
            id: contentColumn
            width: flick.width
            spacing: root.theme.spacingSmall
            padding: root.theme.spacingMedium
        }

        ScrollBar.vertical: ScrollBar {
            policy: flick.contentHeight > flick.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }
    }
}
