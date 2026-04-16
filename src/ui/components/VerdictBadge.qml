import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string state: "pending"
    property string text: "待测试"

    function bgColor() {
        if (root.state === "ok")
            return root.theme.okWeak
        if (root.state === "ng" || root.state === "interrupted")
            return root.theme.dangerWeak
        return "#F0F2F5"
    }

    function fgColor() {
        if (root.state === "ok")
            return root.theme.ok
        if (root.state === "ng" || root.state === "interrupted")
            return root.theme.danger
        return root.theme.textMuted
    }

    radius: root.theme.radiusLarge
    color: bgColor()
    implicitHeight: 56
    implicitWidth: 140

    Text {
        anchors.centerIn: parent
        text: root.text
        color: root.fgColor()
        font.pixelSize: 22
        font.bold: true
    }
}
