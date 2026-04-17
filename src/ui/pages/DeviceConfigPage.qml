pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC
import QtQuick.Layouts
import "../components" as Components

Item {
    id: root
    objectName: "deviceConfigPage"

    required property Components.AppTheme theme

    property bool isModified: false
    property string connectionStatus: "disconnected"
    readonly property real pagePadding: 16
    readonly property real cardMinWidth: 360
    readonly property int gridColumns: Math.max(1, Math.floor((width - pagePadding * 2 + 16) / (cardMinWidth + 16)))
    readonly property real gridCellWidth: Math.max(cardMinWidth, Math.floor((width - pagePadding * 2 - 20 - Math.max(0, gridColumns - 1) * 16) / gridColumns))
    readonly property bool compactHeader: width < 1100

    property var deviceConfig: ({
        aqmd: {
            portName: "COM3",
            baudRate: 9600,
            slaveId: 1,
            timeout: 1000,
            parity: "None",
            stopBits: 1,
            enabled: true
        },
        dyn200: {
            portName: "COM4",
            baudRate: 9600,
            slaveId: 2,
            timeout: 1000,
            parity: "None",
            stopBits: 1,
            enabled: true
        },
        encoder: {
            portName: "COM5",
            baudRate: 9600,
            slaveId: 3,
            timeout: 1000,
            parity: "None",
            stopBits: 1,
            resolution: 4096,
            enabled: true
        },
        brake: {
            portName: "COM6",
            baudRate: 9600,
            slaveId: 4,
            timeout: 1000,
            parity: "None",
            stopBits: 1,
            channel: 1,
            enabled: true
        }
    })

    function saveConfig() {
        if (typeof deviceConfigService !== "undefined" && deviceConfigService.saveDeviceConfig(deviceConfig)) {
            isModified = false
            console.log("Config saved to", deviceConfigService.configPath())
        } else {
            console.warn("Config save failed", typeof deviceConfigService !== "undefined" ? deviceConfigService.lastError() : "deviceConfigService unavailable")
        }
    }

    function resetConfig() {
        if (typeof deviceConfigService !== "undefined") {
            deviceConfig = deviceConfigService.loadDeviceConfig()
        }
        isModified = false
        console.log("Config reset")
    }

    function testConnection() {
        connectionStatus = "untested"
    }

    function testDevice(deviceName) {
        connectionStatus = "untested"
    }

    Component.onCompleted: {
        if (typeof deviceConfigService !== "undefined") {
            deviceConfig = deviceConfigService.loadDeviceConfig()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.pagePadding
            spacing: 16

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true

                        Components.AppLabel {
                            text: "设备配置"
                            fontSize: 16
                            fontWeight: 600
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        RowLayout {
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: {
                                    switch (root.connectionStatus) {
                                        case "connected": return root.theme.okColor
                                        case "connecting": return root.theme.warnColor
                                        case "error": return root.theme.ngColor
                                        default: return root.theme.textMuted
                                    }
                                }
                            }

                            Components.AppLabel {
                                text: {
                                    switch (root.connectionStatus) {
                                        case "connected": return "已连接"
                                        case "connecting": return "连接中..."
                                        case "error": return "连接失败"
                                        default: return "未连接"
                                    }
                                }
                                fontSize: 13
                                theme: root.theme
                            }
                        }
                    }

                    Flow {
                        width: parent.width
                        spacing: 12

                        Components.AppButton {
                            text: "测试连接"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.testConnection()
                        }

                        Components.AppButton {
                            text: "保存配置"
                            variant: "default"
                            size: "sm"
                            theme: root.theme
                            enabled: root.isModified
                            onClicked: root.saveConfig()
                        }

                        Components.AppButton {
                            text: "重置"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.resetConfig()
                        }

                        Components.AppLabel {
                            visible: root.compactHeader
                            text: typeof deviceConfigService !== "undefined" ? "配置文件: " + deviceConfigService.configPath() : ""
                            fontSize: 11
                            theme: root.theme
                            color: root.theme.textSecondary
                        }
                    }
                }
            }

            QQC.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                GridLayout {
                    width: parent.width - 20
                    columns: root.gridColumns
                    columnSpacing: 16
                    rowSpacing: 16

                    Repeater {
                        model: [
                            {name: "AQMD 电机驱动器", key: "aqmd", hasResolution: false, hasChannel: false},
                            {name: "DYN200 扭矩传感器", key: "dyn200", hasResolution: false, hasChannel: false},
                            {name: "单圈绝对值编码器", key: "encoder", hasResolution: true, hasChannel: false},
                            {name: "制动电源", key: "brake", hasResolution: false, hasChannel: true}
                        ]

                        delegate: Components.AppCard {
                            required property var modelData

                            Layout.preferredWidth: root.gridCellWidth
                            Layout.minimumWidth: root.gridCellWidth
                            Layout.maximumWidth: root.gridCellWidth
                            Layout.fillWidth: root.gridColumns === 1
                            implicitHeight: contentColumn.implicitHeight + 28
                            theme: root.theme

                            ColumnLayout {
                                id: contentColumn
                                width: parent.width
                                spacing: 8

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Components.AppLabel {
                                        text: modelData.name
                                        fontSize: 14
                                        fontWeight: 600
                                        theme: root.theme
                                    }

                                    Item { Layout.fillWidth: true }

                                    Components.AppSwitch {
                                        checked: root.deviceConfig[modelData.key].enabled
                                        theme: root.theme
                                        onCheckedChanged: {
                                            root.deviceConfig[modelData.key].enabled = checked
                                            root.isModified = true
                                        }
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 10
                                    rowSpacing: 6

                                    Components.AppLabel {
                                        text: "串口:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        Layout.fillWidth: true
                                        currentValue: root.deviceConfig[modelData.key].portName
                                        model: ["COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8"]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            root.deviceConfig[modelData.key].portName = currentValue
                                            root.isModified = true
                                        }
                                    }

                                    Components.AppLabel {
                                        text: "波特率:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        Layout.fillWidth: true
                                        currentValue: String(root.deviceConfig[modelData.key].baudRate)
                                        model: ["4800", "9600", "19200", "38400", "57600", "115200"]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            root.deviceConfig[modelData.key].baudRate = parseInt(currentValue)
                                            root.isModified = true
                                        }
                                    }

                                    Components.AppLabel {
                                        text: "从站地址:"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: String(root.deviceConfig[modelData.key].slaveId)
                                        theme: root.theme
                                        onTextChanged: {
                                            var val = parseInt(text)
                                            if (isNaN(val) || val < 1) val = 1
                                            if (val > 247) val = 247
                                            root.deviceConfig[modelData.key].slaveId = val
                                            root.isModified = true
                                        }
                                    }

                                    Components.AppLabel {
                                        text: "奇偶校验:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        Layout.fillWidth: true
                                        currentValue: root.deviceConfig[modelData.key].parity
                                        model: [
                                            {value: "None", label: "无"},
                                            {value: "Even", label: "偶校验"},
                                            {value: "Odd", label: "奇校验"}
                                        ]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            root.deviceConfig[modelData.key].parity = currentValue
                                            root.isModified = true
                                        }
                                    }

                                    Components.AppLabel {
                                        text: "停止位:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        Layout.fillWidth: true
                                        currentValue: String(root.deviceConfig[modelData.key].stopBits)
                                        model: [
                                            {value: "1", label: "1 位"},
                                            {value: "2", label: "2 位"}
                                        ]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            root.deviceConfig[modelData.key].stopBits = parseInt(currentValue) || 1
                                            root.isModified = true
                                        }
                                    }

                                    Components.AppLabel {
                                        visible: modelData.hasResolution
                                        text: "分辨率:"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        visible: modelData.hasResolution
                                        Layout.fillWidth: true
                                        text: modelData.hasResolution ? String(root.deviceConfig[modelData.key].resolution) : ""
                                        theme: root.theme
                                        onTextChanged: {
                                            if (modelData.hasResolution) {
                                                root.deviceConfig[modelData.key].resolution = parseInt(text) || 4096
                                                root.isModified = true
                                            }
                                        }
                                    }

                                    Components.AppLabel {
                                        visible: modelData.hasChannel
                                        text: "使用通道:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        visible: modelData.hasChannel
                                        Layout.fillWidth: true
                                        currentValue: modelData.hasChannel ? String(root.deviceConfig[modelData.key].channel) : "1"
                                        model: ["1", "2", "3", "4"]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            if (modelData.hasChannel) {
                                                root.deviceConfig[modelData.key].channel = parseInt(currentValue)
                                                root.isModified = true
                                            }
                                        }
                                    }

                                    Components.AppLabel {
                                        text: "超时 (ms):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: String(root.deviceConfig[modelData.key].timeout)
                                        theme: root.theme
                                        onTextChanged: {
                                            root.deviceConfig[modelData.key].timeout = parseInt(text) || 1000
                                            root.isModified = true
                                        }
                                    }
                                }


                                Components.AppButton {
                                    Layout.fillWidth: true
                                    text: "测试设备"
                                    variant: "outline"
                                    size: "sm"
                                    theme: root.theme
                                    onClicked: root.testDevice(modelData.key)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
