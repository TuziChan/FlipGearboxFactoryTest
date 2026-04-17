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
    readonly property real cardPadding: 16
    readonly property real headerImplicitHeight: root.sectionImplicitHeight(headerContainer)
    readonly property real contentImplicitHeight: root.sectionImplicitHeight(contentContainer)
    readonly property real footerImplicitHeight: root.sectionImplicitHeight(footerContainer)

    implicitWidth: 350
    implicitHeight: headerImplicitHeight + contentImplicitHeight + footerImplicitHeight + cardPadding * 2
    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.width: 1
    border.color: root.theme.borderColor
    clip: true

    function sectionImplicitHeight(container) {
        let maxBottom = 0
        for (let i = 0; i < container.children.length; ++i) {
            const child = container.children[i]
            if (!child || !child.visible)
                continue

            const childHeight = child.implicitHeight > 0 ? child.implicitHeight : child.height
            maxBottom = Math.max(maxBottom, child.y + childHeight)
        }
        return maxBottom
    }

    Column {
        id: cardLayout
        anchors.fill: parent
        anchors.margins: root.cardPadding
        spacing: 0

        Item {
            id: headerContainer
            width: parent.width
            height: implicitHeight
            implicitHeight: root.headerImplicitHeight
            visible: children.length > 0
        }

        Item {
            id: contentContainer
            width: parent.width
            height: root.height > 0
                    ? Math.max(root.contentImplicitHeight,
                               root.height - root.cardPadding * 2 - headerContainer.height - footerContainer.height)
                    : implicitHeight
            implicitHeight: root.contentImplicitHeight
        }

        Item {
            id: footerContainer
            width: parent.width
            height: implicitHeight
            implicitHeight: root.footerImplicitHeight
            visible: children.length > 0
        }
    }
}
