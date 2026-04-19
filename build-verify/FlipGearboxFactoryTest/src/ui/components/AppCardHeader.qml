pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// CardHeader — shadcn/ui parity
// RowLayout with title/description on the left, optional action on the right.
//
// Usage:
//   AppCardHeader {
//       theme: root.theme
//       AppCardTitle { text: "Title" }
//       AppCardDescription { text: "Subtitle" }
//       // Optional action slot:
//       action: AppCardAction { ... }
//   }

RowLayout {
    id: root

    required property AppTheme theme
    default property alias content: headerLeft.data
    property alias action: actionSlot.data

    spacing: 8  // gap-2

    ColumnLayout {
        id: headerLeft
        Layout.fillWidth: true
        spacing: 2
    }

    Item {
        id: actionSlot
        Layout.alignment: Qt.AlignTop
        visible: actionSlot.children.length > 0
        implicitWidth: actionSlot.children.length > 0 ? actionSlot.children[0].implicitWidth : 0
        implicitHeight: actionSlot.children.length > 0 ? actionSlot.children[0].implicitHeight : 0
    }
}
