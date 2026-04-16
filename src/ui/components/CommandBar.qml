import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property bool running
    property alias modelIndex: modelBox.currentIndex
    property alias serialNumber: serialField.text
    property alias backlashValue: backlashField.text
    required property var onStartRequested
    required property var onStopRequested

    objectName: "commandBar"
    color: root.theme.panelColor
    border.color: "transparent"
    implicitHeight: 50

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        spacing: 10

        RowLayout {
            spacing: 6

            AppSelect {
                id: modelBox
                objectName: "modelSelectControl"
                theme: root.theme
                label: "型号"
                model: ["GBX-42A", "GBX-42B", "GBX-56A"]
                popupObjectName: "modelSelectPopup"
                enabled: !root.running
                Layout.preferredWidth: 142
            }
        }

        RowLayout {
            spacing: 6

            AppInput {
                theme: root.theme
                label: "SN"
                id: serialField
                Layout.preferredWidth: 192
                placeholderText: ""
                readOnly: root.running
            }
        }

        RowLayout {
            spacing: 6

            AppInput {
                theme: root.theme
                label: "回差补偿"
                id: backlashField
                Layout.preferredWidth: 62
                readOnly: root.running
            }

            Text { text: "°"; color: root.theme.textSecondary; font.pixelSize: 12 }
        }

        Item { Layout.fillWidth: true }

        AppButton {
            text: "开始测试"
            disabled: root.running
            variant: "primary"
            onClicked: root.onStartRequested()
            theme: root.theme
        }

        AppButton {
            text: "急停"
            disabled: !root.running
            variant: "danger"
            onClicked: root.onStopRequested()
            theme: root.theme
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
