pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    default property alias content: contentRow.data
    property int overlap: 8

    implicitWidth: contentRow.childrenRect.width
    implicitHeight: contentRow.childrenRect.height

    Row {
        id: contentRow
        spacing: -root.overlap
    }
}
