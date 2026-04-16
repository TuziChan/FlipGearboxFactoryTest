pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string size: "default"
    property alias header: headerContainer.data
    property alias footer: footerContainer.data
    default property alias content: contentContainer.data

    implicitWidth: 350
    implicitHeight: contentColumn.implicitHeight
    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.width: 1
    border.color: root.theme.borderColor

    Column {
        id: contentColumn
        anchors.fill: parent
        spacing: 0

        Item {
            id: headerContainer
            width: parent.width
            height: childrenRect.height
            visible: children.length > 0
        }

        Item {
            id: contentContainer
            width: parent.width
            height: childrenRect.height
        }

        Item {
            id: footerContainer
            width: parent.width
            height: childrenRect.height
            visible: children.length > 0
        }
    }
}
