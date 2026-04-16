pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property alias content: container.data

    Layout.column: 1
    Layout.row: 0
    Layout.rowSpan: 2
    Layout.alignment: Qt.AlignTop | Qt.AlignRight
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    Item {
        id: container
        width: childrenRect.width
        height: childrenRect.height
    }
}
