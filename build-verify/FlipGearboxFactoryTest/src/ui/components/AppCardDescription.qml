pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Text {
    id: root

    required property AppTheme theme
    property string description: ""

    text: root.description
    color: root.theme.textMuted
    font.pixelSize: 14
    font.family: "Inter, system-ui, -apple-system, sans-serif"
    lineHeight: 1.5
    wrapMode: Text.WordWrap
    Layout.fillWidth: true
    Layout.columnSpan: parent && parent.columns > 1 ? 2 : 1

    // shadcn/ui 风格的细微动画效果
    Behavior on color {
        ColorAnimation { duration: 150 }
    }

    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }
}
