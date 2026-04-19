pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// Card — shadcn/ui parity
//
// Container: flex flex-col gap-6 rounded-xl border bg-card py-6 text-card-foreground shadow-sm
//
// Supports two sizing modes:
//   1. Intrinsic: Card sizes to content (implicitWidth/implicitHeight)
//   2. Fill: Card stretches when placed in a layout with Layout.fillWidth/Height
//
// Children go into a ColumnLayout with gap-6 spacing and py-6 padding.
// Use Layout.fillWidth/Layout.fillHeight on children to stretch them.
// Do NOT use anchors.fill: parent on children — use Layout attached props.

Rectangle {
    id: root

    required property AppTheme theme
    default property alias content: cardLayout.data

    readonly property real _gap: 24       // gap-6
    readonly property real _paddingX: 24   // px-6
    readonly property real _paddingY: 24   // py-6

    implicitWidth: 350
    implicitHeight: cardLayout.implicitHeight + _paddingY * 2

    radius: theme.radiusLarge
    color: theme.cardColor
    border.width: 1
    border.color: theme.dividerColor
    clip: true

    ColumnLayout {
        id: cardLayout
        x: root._paddingX
        y: root._paddingY
        width: root.width - root._paddingX * 2
        // When card has explicit height (fill mode), stretch content;
        // otherwise use implicitHeight for intrinsic sizing.
        height: root.height > _paddingY * 2
            ? root.height - root._paddingY * 2
            : implicitHeight
        spacing: root._gap
    }
}
