pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// CardFooter — shadcn/ui parity
// flex items-center px-6

Item {
    id: root

    required property AppTheme theme
    default property alias content: rowLayout.data

    Layout.fillWidth: true
    implicitHeight: rowLayout.implicitHeight

    RowLayout {
        id: rowLayout
        width: root.width
        spacing: 8
    }
}
