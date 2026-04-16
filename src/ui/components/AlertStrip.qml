import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string variant: "danger"
    property string title: ""
    property string text: ""

    visible: root.text.length > 0
    color: variant === "danger" ? theme.dangerWeak : theme.warnWeak
    border.color: "transparent"
    implicitHeight: visible ? 44 : 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 14

        Text {
            text: root.title
            color: variant === "danger" ? theme.danger : theme.warn
            font.pixelSize: 13
            font.bold: true
        }

        Text {
            text: root.text
            color: variant === "danger" ? theme.danger : theme.warn
            font.pixelSize: 13
        }
    }
}
