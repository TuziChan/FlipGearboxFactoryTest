import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property int order: 1
    property string title: ""
    property string detail: ""
    property string elapsedText: "00:00.0"
    property string state: "pending"

    readonly property bool current: state === "current" || state === "run"
    readonly property bool done: state === "done"

    radius: theme.radiusMedium
    color: current ? theme.accentWeak : done ? theme.okWeak : theme.cardColor
    border.width: !current && !done ? 1 : 0
    border.color: theme.dividerColor
    implicitHeight: 58

    Column {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 3

        RowLayout {
            width: parent.width
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                radius: 10
                color: root.done ? root.theme.ok : root.current ? "#DCEBFF" : "#F0F1F4"

                Text {
                    anchors.centerIn: parent
                    text: root.done ? "✓" : root.order
                    color: root.done ? "#FFFFFF" : root.current ? root.theme.accent : root.theme.textMuted
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Text {
                Layout.fillWidth: true
                text: root.title
                color: root.theme.textPrimary
                font.pixelSize: 12
                font.bold: true
            }

            Text {
                text: root.elapsedText
                color: root.theme.textMuted
                font.pixelSize: 10
            }
        }

        Text {
            text: root.detail
            color: root.theme.textSecondary
            font.pixelSize: 10
            elide: Text.ElideRight
        }
    }
}
