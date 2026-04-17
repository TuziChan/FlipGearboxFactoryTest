pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import "../components" as Components

Item {
    id: root
    objectName: "diagnosticsPage"

    required property Components.AppTheme theme
    property var viewModel: typeof diagnosticsViewModel !== "undefined" ? diagnosticsViewModel : null
    readonly property real deviceCardMinWidth: 320
    readonly property real deviceCardSpacing: 12
    readonly property int deviceCardColumns: width >= (deviceCardMinWidth * 2 + deviceCardSpacing + 80) ? 2 : 1
    readonly property int deviceCardHeight: 108
    readonly property int deviceCount: viewModel ? viewModel.deviceStatuses.length : 0
    readonly property int deviceRows: Math.max(1, Math.ceil(deviceCount / deviceCardColumns))
    readonly property real deviceStatusPaneHeight: Math.min(380,
                                                            56 + deviceRows * deviceCardHeight + Math.max(0, deviceRows - 1) * deviceCardSpacing + 36)

    property bool autoRefresh: true
    property int refreshInterval: 2000

    Timer {
        id: refreshTimer
        interval: root.refreshInterval
        running: root.autoRefresh
        repeat: true
        onTriggered: {
            if (root.viewModel)
                root.viewModel.refreshIncremental()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                RowLayout {
                    width: parent.width
                    spacing: 12

                    Components.AppLabel {
                        text: "I/O 诊断"
                        fontSize: 16
                        fontWeight: 600
                        theme: root.theme
                    }

                    Item { Layout.fillWidth: true }

                    Components.AppLabel {
                        text: "自动刷新:"
                        theme: root.theme
                    }

                    Components.AppSwitch {
                        checked: root.autoRefresh
                        theme: root.theme
                        onCheckedChanged: root.autoRefresh = checked
                    }

                    Components.AppLabel {
                        text: "刷新间隔:"
                        theme: root.theme
                    }

                    Components.AppSelect {
                        Layout.preferredWidth: 100
                        currentValue: String(root.refreshInterval)
                        model: [
                            {value: "100", label: "100ms"},
                            {value: "500", label: "500ms"},
                            {value: "1000", label: "1s"},
                            {value: "2000", label: "2s"}
                        ]
                        theme: root.theme
                        onCurrentValueChanged: root.refreshInterval = parseInt(currentValue)
                    }

                    Components.AppButton {
                        text: "立即刷新"
                        variant: "outline"
                        size: "sm"
                        theme: root.theme
                        onClicked: if (root.viewModel) root.viewModel.refresh()
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16

                    Components.AppCard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.deviceStatusPaneHeight
                        theme: root.theme

                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            Components.AppLabel {
                                text: "设备状态"
                                fontSize: 14
                                fontWeight: 600
                                theme: root.theme
                            }

                            Components.AppSeparator {
                                Layout.fillWidth: true
                                theme: root.theme
                            }

                            QQC.ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true
                                Grid {
                                    id: deviceGrid
                                    width: parent.width
                                    columns: root.deviceCardColumns
                                    spacing: root.deviceCardSpacing

                                    Repeater {
                                        model: root.viewModel ? root.viewModel.deviceStatuses : []

                                        delegate: Components.AppCard {
                                            required property string name
                                            required property string status
                                            required property string lastUpdate
                                            required property string summary
                                            required property int errorCount
                                            width: deviceGrid.columns > 1
                                                   ? Math.max(root.deviceCardMinWidth, Math.floor((deviceGrid.width - root.deviceCardSpacing) / 2))
                                                   : deviceGrid.width
                                            height: root.deviceCardHeight
                                            theme: root.theme

                                            ColumnLayout {
                                                width: parent.width
                                                spacing: 10

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 10

                                                    Rectangle {
                                                        width: 8
                                                        height: 8
                                                        radius: 4
                                                        color: status === "online" ? root.theme.okColor : root.theme.ngColor
                                                        Layout.alignment: Qt.AlignVCenter
                                                    }

                                                    Components.AppLabel {
                                                        text: name
                                                        fontSize: 14
                                                        fontWeight: 600
                                                        theme: root.theme
                                                        Layout.alignment: Qt.AlignVCenter
                                                    }

                                                    Item { Layout.fillWidth: true }

                                                    Components.AppBadge {
                                                        text: status === "online" ? "在线" : "离线"
                                                        variant: status === "online" ? "success" : "destructive"
                                                        theme: root.theme
                                                        Layout.alignment: Qt.AlignVCenter
                                                    }
                                                }

                                                Components.AppLabel {
                                                    text: "最后更新: " + lastUpdate
                                                    fontSize: 11
                                                    color: root.theme.textSecondary
                                                    theme: root.theme
                                                }

                                                Components.AppLabel {
                                                    text: summary
                                                    fontSize: 12
                                                    color: root.theme.textPrimary
                                                    theme: root.theme
                                                    wrapMode: Text.Wrap
                                                    maximumLineCount: 2
                                                }

                                                Components.AppLabel {
                                                    visible: errorCount > 0
                                                    text: "通信错误: " + errorCount + " 次"
                                                    fontSize: 11
                                                    color: root.theme.warnColor
                                                    theme: root.theme
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Components.AppCard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 248
                        theme: root.theme

                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            Components.AppLabel {
                                text: "手动控制"
                                fontSize: 15
                                fontWeight: 600
                                theme: root.theme
                            }

                            Components.AppSeparator {
                                Layout.fillWidth: true
                                theme: root.theme
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 2
                                columnSpacing: 12
                                rowSpacing: 12

                                Components.AppLabel {
                                    text: "电机控制:"
                                    color: root.theme.textSecondary
                                    fontWeight: 500
                                    theme: root.theme
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Components.AppButton {
                                        text: "正转"
                                        variant: "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setMotorForward()
                                    }

                                    Components.AppButton {
                                        text: "反转"
                                        variant: "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setMotorReverse()
                                    }

                                    Components.AppButton {
                                        text: "停止"
                                        variant: "destructive"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.stopMotor()
                                    }
                                }

                                Components.AppLabel {
                                    text: "制动电源:"
                                    color: root.theme.textSecondary
                                    fontWeight: 500
                                    theme: root.theme
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Components.AppButton {
                                        text: "使能"
                                        variant: "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setBrakeOutput(true)
                                    }

                                    Components.AppButton {
                                        text: "禁用"
                                        variant: "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setBrakeOutput(false)
                                    }
                                }

                                Components.AppLabel {
                                    text: "制动电流:"
                                    color: root.theme.textSecondary
                                    fontWeight: 500
                                    theme: root.theme
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Components.AppInput {
                                        id: brakeCurrentInput
                                        Layout.preferredWidth: 100
                                        text: "0.50"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "A"
                                        theme: root.theme
                                    }

                                    Components.AppButton {
                                        text: "设置"
                                        variant: "default"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setBrakeCurrent(Number(brakeCurrentInput.text))
                                    }

                                    Components.AppButton {
                                        text: "编码器置零"
                                        variant: "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: if (root.viewModel) root.viewModel.setEncoderZero()
                                    }
                                }
                            }

                            Item { Layout.fillHeight: true }

                            Components.AppAlert {
                                Layout.fillWidth: true
                                variant: root.viewModel && root.viewModel.runtimeInitialized ? "default" : "warning"
                                title: root.viewModel && root.viewModel.runtimeInitialized ? "状态" : "警告"
                                description: root.viewModel ? root.viewModel.statusMessage : "诊断后端未连接"
                                theme: root.theme
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }

                Components.AppCard {
                    Layout.preferredWidth: 480
                    Layout.fillHeight: true
                    theme: root.theme

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Components.AppLabel {
                                text: "通信日志"
                                fontSize: 14
                                fontWeight: 600
                                theme: root.theme
                            }

                            Item { Layout.fillWidth: true }

                            Components.AppButton {
                                text: "清空"
                                variant: "outline"
                                size: "sm"
                                theme: root.theme
                                onClicked: if (root.viewModel) root.viewModel.clearLog()
                            }
                        }

                        Components.AppSeparator {
                            Layout.fillWidth: true
                            theme: root.theme
                        }

                        QQC.ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ListView {
                                id: logListView
                                model: root.viewModel ? root.viewModel.communicationLogs : []
                                spacing: 4

                                delegate: Rectangle {
                                    required property string time
                                    required property string direction
                                    required property string device
                                    required property string message
                                    required property bool success

                                    width: ListView.view.width
                                    height: 60
                                    radius: 4
                                    color: success ? root.theme.bgSecondary : Qt.tint(root.theme.ngColor, "#22ffffff")

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 4

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Components.AppLabel {
                                                text: time
                                                fontSize: 11
                                                color: root.theme.textMuted
                                                theme: root.theme
                                            }

                                            Components.AppBadge {
                                                text: direction
                                                variant: success ? (direction === "发送" ? "default" : "secondary") : "destructive"
                                                theme: root.theme
                                            }

                                            Components.AppLabel {
                                                text: device
                                                fontSize: 11
                                                fontWeight: 500
                                                theme: root.theme
                                            }
                                        }

                                        Components.AppLabel {
                                            text: message
                                            fontSize: 12
                                            theme: root.theme
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (root.viewModel)
            root.viewModel.refresh()
    }
}
