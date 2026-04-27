pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property var runtimeManager: null

    signal delayChanged(int delayMs)
    signal errorInjected(string errorType)
    signal scenarioChanged(string scenario)

    radius: theme.radiusLarge
    color: theme.cardColor
    border.color: theme.dividerColor
    border.width: 1
    implicitHeight: 280

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Mock 测试控制"
                color: theme.textPrimary
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.runtimeManager && root.runtimeManager.isMockMode ? theme.okColor : theme.textMuted
            }

            Text {
                text: root.runtimeManager && root.runtimeManager.isMockMode ? "Mock 模式" : "真实设备"
                color: root.runtimeManager && root.runtimeManager.isMockMode ? theme.okColor : theme.textMuted
                font.pixelSize: 11
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            AppButton {
                text: root.runtimeManager && root.runtimeManager.isMockMode ? "切换到真实设备" : "切换到 Mock"
                variant: "outline"
                size: "sm"
                theme: root.theme
                onClicked: {
                    if (root.runtimeManager) {
                        root.runtimeManager.switchMode(!root.runtimeManager.isMockMode)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.dividerColor
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 12

            Text {
                text: "通信延迟"
                color: theme.textPrimary
                font.pixelSize: 12
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                AppSlider {
                    id: delaySlider
                    Layout.fillWidth: true
                    theme: root.theme
                    from: 0
                    to: 500
                    value: 50
                    stepSize: 10
                    onValueChanged: root.delayChanged(Math.round(value))
                }

                Text {
                    text: Math.round(delaySlider.value) + " ms"
                    color: theme.textSecondary
                    font.pixelSize: 11
                    Layout.preferredWidth: 50
                }
            }

            Text {
                text: "测试场景"
                color: theme.textPrimary
                font.pixelSize: 12
            }

            AppSelect {
                id: scenarioSelect
                Layout.fillWidth: true
                theme: root.theme
                model: ["正常运行", "高速运行", "重载测试", "边界角度", "磁铁干扰"]
                currentIndex: 0
                onCurrentIndexChanged: {
                    const scenarios = ["normal", "highspeed", "heavyload", "boundary", "magnet"]
                    root.scenarioChanged(scenarios[currentIndex])
                }
            }

            Text {
                text: "错误注入"
                color: theme.textPrimary
                font.pixelSize: 12
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                AppButton {
                    text: "超时"
                    variant: "outline"
                    size: "sm"
                    theme: root.theme
                    onClicked: root.errorInjected("timeout")
                }

                AppButton {
                    text: "断线"
                    variant: "outline"
                    size: "sm"
                    theme: root.theme
                    onClicked: root.errorInjected("disconnect")
                }

                AppButton {
                    text: "数据错误"
                    variant: "outline"
                    size: "sm"
                    theme: root.theme
                    onClicked: root.errorInjected("data_error")
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.dividerColor
        }

        Text {
            text: "磁铁检测模拟"
            color: theme.textPrimary
            font.pixelSize: 12
            font.bold: true
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 8
            rowSpacing: 6

            Repeater {
                model: [
                    { angle: "3°", label: "磁铁①", active: false },
                    { angle: "49°", label: "磁铁②", active: false },
                    { angle: "113.5°", label: "磁铁③", active: false }
                ]

                delegate: Rectangle {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    height: 36
                    radius: theme.radiusMedium
                    color: modelData.active ? theme.accentLight : theme.sectionColor
                    border.color: modelData.active ? theme.accent : theme.dividerColor
                    border.width: 1

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData.label
                            color: modelData.active ? theme.accent : theme.textPrimary
                            font.pixelSize: 10
                            font.bold: modelData.active
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData.angle
                            color: modelData.active ? theme.accent : theme.textSecondary
                            font.pixelSize: 9
                        }
                    }
                }
            }
        }
    }
}
