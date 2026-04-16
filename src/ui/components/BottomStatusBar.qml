import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property var items
    property string clockText: ""

    color: "#FFFFFF"
    border.color: theme.dividerColor
    implicitHeight: 34

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 16

        Repeater {
            model: root.items

            delegate: RowLayout {
                required property var modelData
                spacing: 6

                Rectangle {
                    Layout.preferredWidth: 7
                    Layout.preferredHeight: 7
                    radius: 3.5
                    color: root.theme.ok
                }

                Text {
                    text: modelData.name || modelData
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: root.clockText
            color: root.theme.textMuted
            font.pixelSize: 12
        }
    }
}
