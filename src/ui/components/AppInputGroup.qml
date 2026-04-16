pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    default property alias content: contentRow.data
    property string leadingText: ""
    property string trailingText: ""
    property string leadingIcon: ""
    property string trailingIcon: ""

    radius: root.theme.radiusMedium
    color: root.theme.cardColor
    border.width: 1
    border.color: root.theme.stroke
    implicitWidth: Math.max(220, contentRow.implicitWidth + 20)
    implicitHeight: Math.max(32, contentRow.implicitHeight + 8)

    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 8

        AppIcon {
            visible: root.leadingIcon.length > 0
            name: root.leadingIcon
            color: root.theme.textMuted
            iconSize: 12
        }

        Text {
            visible: root.leadingText.length > 0
            text: root.leadingText
            color: root.theme.textMuted
            font.pixelSize: 12
        }

        Item { Layout.fillWidth: true; visible: false }

        Text {
            visible: root.trailingText.length > 0
            text: root.trailingText
            color: root.theme.textMuted
            font.pixelSize: 12
        }

        AppIcon {
            visible: root.trailingIcon.length > 0
            name: root.trailingIcon
            color: root.theme.textMuted
            iconSize: 12
        }
    }
}
