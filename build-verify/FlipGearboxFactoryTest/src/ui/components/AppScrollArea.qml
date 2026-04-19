pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property AppTheme theme
    default property alias content: contentColumn.data
    readonly property bool atVerticalEnd: scrollView.contentItem.contentY >= Math.max(0, scrollView.contentItem.contentHeight - scrollView.contentItem.height - 1)

    function scrollToEnd() {
        scrollView.contentItem.contentY = Math.max(0, scrollView.contentItem.contentHeight - scrollView.contentItem.height)
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

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: 1
        clip: true

        Column {
            id: contentColumn
            width: scrollView.width - (scrollView.ScrollBar.vertical.visible ? scrollView.ScrollBar.vertical.width : 0)
            spacing: root.theme.spacingSmall
            padding: root.theme.spacingMedium
        }
    }
}
