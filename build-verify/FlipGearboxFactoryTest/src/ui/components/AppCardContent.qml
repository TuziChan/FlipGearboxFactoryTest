pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// CardContent — shadcn/ui parity
// px-6 padding area for card body content.

Item {
    id: root

    required property AppTheme theme
    default property alias content: container.data

    Layout.fillWidth: true
    implicitHeight: container.childrenRect.height

    Item {
        id: container
        width: root.width
        height: root.height > 0 ? root.height : container.childrenRect.height
    }
}
