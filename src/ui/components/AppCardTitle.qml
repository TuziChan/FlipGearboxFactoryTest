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
    wrapMode: Text.WordWrap
    Layout.fillWidth: true
    Layout.columnSpan: parent && parent.columns > 1 ? 1 : 1
}
