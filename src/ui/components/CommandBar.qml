import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property bool running
    readonly property int fieldGroupWidth: 180
    property alias modelIndex: modelBox.currentIndex
    property alias serialNumber: serialField.text
    property alias backlashValue: backlashField.text
    readonly property string modelText: modelBox.currentText
    required property var onStartRequested
    required property var onStopRequested

    objectName: "commandBar"
    color: root.theme.panelColor
    border.color: "transparent"
    implicitHeight: 68

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 20

            Column {
                width: root.fieldGroupWidth
                spacing: 4

                Text {
                    text: "型号"
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }

                AppSelect {
                    id: modelBox
                    objectName: "modelSelectControl"
                    theme: root.theme
                    label: ""
                    model: ["GBX-42A", "GBX-42B", "GBX-56A"]
                    popupObjectName: "modelSelectPopup"
                    enabled: !root.running
                    width: parent.width
                }
            }


            Column {
                width: root.fieldGroupWidth
                spacing: 4

                Text {
                    text: "SN"
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }

                AppInput {
                    theme: root.theme
                    label: ""
                    id: serialField
                    width: parent.width
                    placeholderText: ""
                    readOnly: root.running
                }
            }

            Column {
                width: root.fieldGroupWidth
                spacing: 4

                Text {
                    text: "回差补偿"
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
                AppInput {
                    theme: root.theme
                    label: ""
                    id: backlashField
                    width: parent.width
                    suffixText: "°"
                    readOnly: root.running
                }
            }
        }

        Item { Layout.fillWidth: true }
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 12

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
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: root.theme.dividerColor
    }
}
