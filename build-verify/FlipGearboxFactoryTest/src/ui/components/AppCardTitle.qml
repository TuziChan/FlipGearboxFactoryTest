pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Text {
    id: root

    required property AppTheme theme
    property string title: ""

    text: root.title
    color: root.theme.textPrimary
    font.pixelSize: 18
    font.weight: Font.DemiBold
    font.family: "Inter, system-ui, -apple-system, sans-serif"
    wrapMode: Text.WordWrap
    Layout.fillWidth: true
    Layout.columnSpan: parent && parent.columns > 1 ? 1 : 1

    // shadcn/ui 风格的细微动画效果
    Behavior on color {
        ColorAnimation { duration: 150 }
    }

    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }
}