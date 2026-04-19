pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string appTitle: "齿轮箱测试系统"
    property string pageTitle: ""
    property string stationText: "工位 #03"
    property string connectionText: ""
    property bool isConnected: false

    color: root.theme.cardColor
    border.color: "transparent"
    implicitHeight: 52

    RowLayout {
        width: parent.width - 36
        height: parent.height
        x: 18
        spacing: 12

        RowLayout {
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 14
                color: root.theme.accentWeak
                border.color: root.theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "G"
                    color: root.theme.accent
                    font.pixelSize: 14
                    font.bold: true
                }
            }

            ColumnLayout {
                spacing: 0

                Text {
                    text: root.appTitle
                    color: root.theme.textPrimary
                    font.pixelSize: 15
                    font.bold: true
                }

                Text {
                    text: root.pageTitle
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }
        }

        Item { Layout.fillWidth: true }

        AppBadge {
            theme: root.theme
            text: root.stationText
            variant: "default"
        }

        RowLayout {
            spacing: 6
            visible: root.connectionText.length > 0

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: root.isConnected ? root.theme.ok : root.theme.ngColor
            }

            Text {
                text: root.connectionText
                color: root.isConnected ? root.theme.ok : root.theme.ngColor
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: root.theme.dividerColor
    }
}
