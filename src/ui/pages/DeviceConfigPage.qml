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
    readonly property string connectionStatus: {
        if (typeof diagnosticsViewModel === "undefined" || !diagnosticsViewModel)
            return "disconnected"
        var statuses = diagnosticsViewModel.deviceStatuses
        if (!statuses || statuses.length === 0)
            return "disconnected"
        var anyOnline = false
        var anyEnabled = false
        var keys = ["aqmd", "dyn200", "encoder", "brake"]
        for (var i = 0; i < keys.length; i++) {
            if (!root.deviceConfig[keys[i]].enabled) continue
            anyEnabled = true
            if (root.isDeviceOnline(keys[i])) anyOnline = true
        }
        if (!anyEnabled) return "disconnected"
        return anyOnline ? "connected" : "error"
    }

    // DYN200 mode switch confirmation state
    property bool dyn200ModeConfirmVisible: false
    property string dyn200PendingModeValue: ""
    property string dyn200PendingModeLabel: ""
    property int dyn200SavedCurrentIndex: -1
    property bool dyn200ConfirmAccepted: false
    property bool testDialogOpen: false
    property string testResultTitle: ""
    property string testResultText: ""
    property string testResultType: "info"
    readonly property real pagePadding: 16
    readonly property real cardMinWidth: 360
    readonly property int gridColumns: Math.max(1, Math.floor((width - pagePadding * 2 + 16) / (cardMinWidth + 16)))
    readonly property real gridCellWidth: Math.max(cardMinWidth, Math.floor((width - pagePadding * 2 - 20 - Math.max(0, gridColumns - 1) * 16) / gridColumns))
    readonly property bool compactHeader: width < 1100

    readonly property var availablePorts: {
        if (typeof deviceConfigService !== "undefined" && deviceConfigService) {
            var ports = deviceConfigService.availableSerialPorts()
            if (ports && ports.length > 0)
                return ports
        }
        return ["COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8"]
    }

    property var deviceConfig: ({
        aqmd: {
            portName: "COM3",
            baudRate: 9600,
            slaveId: 1,
            timeout: 500,
            parity: "Even",
            stopBits: 1,
            communicationMode: 0,
            enabled: true
        },
        dyn200: {
            portName: "COM4",
            baudRate: 19200,
            slaveId: 2,
            timeout: 500,
            parity: "None",
            stopBits: 2,
            communicationMode: 0,
            enabled: true
        },
        encoder: {
            portName: "COM5",
            baudRate: 9600,
            slaveId: 3,
            timeout: 500,
            parity: "None",
            stopBits: 1,
            communicationMode: 0,
            resolution: 4096,
            enabled: true
        },
        brake: {
            portName: "COM6",
            baudRate: 9600,
            slaveId: 4,
            timeout: 500,
            parity: "None",
            stopBits: 1,
            communicationMode: 0,
            channel: 1,
            enabled: true
        }
    })

    property var deviceNames: ({
        "aqmd": "AQMD 电机驱动器",
        "dyn200": "DYN200 扭矩传感器",
        "encoder": "单圈绝对值编码器",
        "brake": "制动电源"
    })

    function isDeviceOnline(deviceKey) {
        if (typeof diagnosticsViewModel === "undefined" || !diagnosticsViewModel)
            return false
        var statuses = diagnosticsViewModel.deviceStatuses
        if (!statuses || statuses.length === 0)
            return false
        for (var i = 0; i < statuses.length; i++) {
            var s = statuses[i]
            if (s.name === root.deviceNames[deviceKey])
                return s.status === "online"
        }
        return false
    }

    function saveConfig() {
        if (typeof deviceConfigService !== "undefined" && deviceConfigService.saveDeviceConfig(deviceConfig)) {
            isModified = false
            console.log("Config saved to", deviceConfigService.configPath())
            return true
        } else {
            console.warn("Config save failed", typeof deviceConfigService !== "undefined" ? deviceConfigService.lastError() : "deviceConfigService unavailable")
            return false
        }
    }

    function resetConfig() {
        if (typeof deviceConfigService !== "undefined") {
            deviceConfig = deviceConfigService.loadDeviceConfig()
        }
        isModified = false
        console.log("Config reset")
    }

    function connectDevice(deviceKey) {
        var label = root.deviceNames[deviceKey] || deviceKey
        var cfg = root.deviceConfig[deviceKey]

        if (!cfg || !cfg.enabled) {
            root.testResultTitle = label
            root.testResultText = "设备未启用，无法连接"
            root.testResultType = "warning"
            root.testDialogOpen = true
            return
        }

        if (!saveConfig()) {
            root.testResultTitle = label
            root.testResultText = "配置保存失败"
            root.testResultType = "error"
            root.testDialogOpen = true
            return
        }

        root.testResultTitle = label
        root.testResultText = "正在连接..."
        root.testResultType = "info"
        root.testDialogOpen = true

        if (typeof runtimeManager !== "undefined" && runtimeManager) {
            runtimeManager.reloadRuntime()
        }

        deviceConnectTimer.deviceKey = deviceKey
        deviceConnectTimer.restart()
    }

    function disconnectDevice(deviceKey) {
        var label = root.deviceNames[deviceKey] || deviceKey

        if (typeof runtimeManager !== "undefined" && runtimeManager) {
            runtimeManager.disconnectDeviceBus(deviceKey)
        }
        if (typeof diagnosticsViewModel !== "undefined" && diagnosticsViewModel) {
            diagnosticsViewModel.refresh()
        }

        root.testResultTitle = label
        root.testResultText = "已断开连接"
        root.testResultType = "info"
        root.testDialogOpen = true
    }

    Timer {
        id: deviceConnectTimer
        property string deviceKey: ""
        interval: 600
        onTriggered: {
            var label = root.deviceNames[deviceKey] || deviceKey
            if (root.isDeviceOnline(deviceKey)) {
                root.testResultTitle = label
                root.testResultText = "连接成功"
                root.testResultType = "success"
            } else {
                root.testResultTitle = label
                root.testResultText = "连接失败，请检查串口配置"
                root.testResultType = "error"
            }
            root.testDialogOpen = true
            deviceKey = ""
        }
    }

    function testDevice(deviceKey) {
        var label = root.deviceNames[deviceKey] || deviceKey

        if (!root.isDeviceOnline(deviceKey)) {
            root.testResultTitle = label
            root.testResultText = "设备未连接，请先连接设备"
            root.testResultType = "warning"
            root.testDialogOpen = true
            return
        }

        if (typeof diagnosticsViewModel === "undefined" || !diagnosticsViewModel) {
            root.testResultTitle = label
            root.testResultText = "diagnosticsViewModel 不可用"
            root.testResultType = "error"
            root.testDialogOpen = true
            return
        }

        root.testResultTitle = label
        root.testResultText = "正在测试..."
        root.testResultType = "info"
        root.testDialogOpen = true

        diagnosticsViewModel.refresh()
        testDeviceTimer.deviceKey = deviceKey
        testDeviceTimer.restart()
    }

    Timer {
        id: testDeviceTimer
        property string deviceKey: ""
        interval: 300
        onTriggered: {
            var label = root.deviceNames[deviceKey] || deviceKey
            if (typeof diagnosticsViewModel === "undefined" || !diagnosticsViewModel) {
                root.testResultTitle = label
                root.testResultText = "diagnosticsViewModel 不可用"
                root.testResultType = "error"
                root.testDialogOpen = true
                deviceKey = ""
                return
            }

            var statuses = diagnosticsViewModel.deviceStatuses
            var found = null
            for (var i = 0; i < statuses.length; i++) {
                if (statuses[i].name === label) {
                    found = statuses[i]
                    break
                }
            }

            root.testResultTitle = label + " 测试"
            if (found && found.status === "online") {
                root.testResultText = "✅ 设备响应正常\n" + (found.summary || "")
                root.testResultType = "success"
            } else if (found) {
                root.testResultText = "❌ 设备无响应\n" + (found.summary || "")
                root.testResultType = "error"
            } else {
                root.testResultText = "❌ 未找到设备状态"
                root.testResultType = "error"
            }
            root.testDialogOpen = true
            deviceKey = ""
        }
    }

    Component.onCompleted: {
        if (typeof deviceConfigService !== "undefined") {
            deviceConfig = deviceConfigService.loadDeviceConfig()
            console.log("DeviceConfig loaded:",
                        "aqmd=", deviceConfig.aqmd.baudRate,
                        "dyn200=", deviceConfig.dyn200.baudRate,
                        "encoder=", deviceConfig.encoder.baudRate,
                        "brake=", deviceConfig.brake.baudRate)
        }
    }

    Component.onDestruction: {
        if (isModified) {
            saveConfig()
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
                            text: "全部连接"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: {
                                if (!root.saveConfig()) return
                                if (typeof runtimeManager !== "undefined" && runtimeManager) {
                                    runtimeManager.reloadRuntime()
                                }
                            }
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
                            {
                                name: "AQMD 电机驱动器",
                                key: "aqmd",
                                hasResolution: false,
                                hasChannel: false,
                                protocolOptions: [
                                    {value: 0, label: "Modbus RTU"}
                                ]
                            },
                            {
                                name: "DYN200 扭矩传感器",
                                key: "dyn200",
                                hasResolution: false,
                                hasChannel: false,
                                protocolOptions: [
                                    {value: 0, label: "Modbus RTU"},
                                    {value: 1, label: "HEX 6 字节主动上传"},
                                    {value: 2, label: "HEX 8 字节主动上传"},
                                    {value: 3, label: "ASCII 主动上传"}
                                ]
                            },
                            {
                                name: "单圈绝对值编码器",
                                key: "encoder",
                                hasResolution: true,
                                hasChannel: false,
                                protocolOptions: [
                                    {value: 0, label: "查询模式"},
                                    {value: 1, label: "预留模式 1"},
                                    {value: 2, label: "自动回传单圈"},
                                    {value: 3, label: "自动回传虚拟多圈"},
                                    {value: 4, label: "自动回传角速度"}
                                ]
                            },
                            {
                                name: "制动电源",
                                key: "brake",
                                hasResolution: false,
                                hasChannel: true,
                                protocolOptions: [
                                    {value: 0, label: "Modbus RTU"}
                                ]
                            }
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
                                        model: root.availablePorts
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
                                        text: "通信协议:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        id: dyn200CommModeSelect
                                        Layout.fillWidth: true
                                        currentValue: root.deviceConfig[modelData.key].communicationMode
                                        model: modelData.protocolOptions
                                        enabled: modelData.protocolOptions.length > 1
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            // DYN200 P0 safety: warn when switching from proactive mode back to Modbus RTU
                                            if (modelData.key === "dyn200") {
                                                var oldMode = root.deviceConfig[modelData.key].communicationMode
                                                var newMode = currentValue
                                                if (oldMode !== 0 && newMode === 0) {
                                                    // Save the old currentIndex so we can restore on cancel
                                                    var oldIdx = 0
                                                    for (var j = 0; j < modelData.protocolOptions.length; j++) {
                                                        if (modelData.protocolOptions[j].value === oldMode) {
                                                            oldIdx = j
                                                            break
                                                        }
                                                    }
                                                    root.dyn200SavedCurrentIndex = oldIdx
                                                    root.dyn200ConfirmAccepted = false

                                                    // User is switching from proactive mode to Modbus RTU - show confirmation
                                                    root.dyn200PendingModeValue = newMode
                                                    var label = "Modbus RTU"
                                                    for (var i = 0; i < modelData.protocolOptions.length; i++) {
                                                        if (modelData.protocolOptions[i].value === newMode) {
                                                            label = modelData.protocolOptions[i].label
                                                            break
                                                        }
                                                    }
                                                    root.dyn200PendingModeLabel = label
                                                    root.dyn200ModeConfirmVisible = true
                                                    return  // Don't apply yet, wait for confirmation
                                                }
                                            }
                                            root.deviceConfig[modelData.key].communicationMode = currentValue
                                            root.isModified = true
                                        }
                                    }
                                    Connections {
                                        target: root
                                        function onDyn200ModeConfirmVisibleChanged() {
                                            if (modelData.key === "dyn200" && !root.dyn200ModeConfirmVisible) {
                                                if (root.dyn200ConfirmAccepted) {
                                                    // Confirmed: sync currentIndex to the newly applied mode
                                                    for (var i = 0; i < modelData.protocolOptions.length; i++) {
                                                        if (modelData.protocolOptions[i].value === root.deviceConfig.dyn200.communicationMode) {
                                                            dyn200CommModeSelect.currentIndex = i
                                                            break
                                                        }
                                                    }
                                                } else if (root.dyn200SavedCurrentIndex >= 0) {
                                                    // Cancelled: restore the original currentIndex
                                                    dyn200CommModeSelect.currentIndex = root.dyn200SavedCurrentIndex
                                                }
                                                root.dyn200SavedCurrentIndex = -1
                                            }
                                        }
                                    }

                                    Components.AppLabel {
                                        visible: modelData.hasResolution
                                        text: "分辨率:"
                                        theme: root.theme
                                    }
                                    Components.AppSelect {
                                        visible: modelData.hasResolution
                                        Layout.fillWidth: true
                                        currentValue: modelData.hasResolution ? String(root.deviceConfig[modelData.key].resolution) : "4096"
                                        model: [
                                            {value: "1024", label: "1024 (10bit)"},
                                            {value: "4096", label: "4096 (12bit)"},
                                            {value: "16384", label: "16384 (14bit)"},
                                            {value: "32768", label: "32768 (15bit)"},
                                            {value: "65536", label: "65536 (16bit)"},
                                            {value: "131072", label: "131072 (17bit)"}
                                        ]
                                        theme: root.theme
                                        onCurrentValueChanged: {
                                            if (modelData.hasResolution) {
                                                root.deviceConfig[modelData.key].resolution = parseInt(currentValue)
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

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Components.AppButton {
                                        Layout.fillWidth: true
                                        text: root.isDeviceOnline(modelData.key) ? "断开设备" : "连接设备"
                                        variant: root.isDeviceOnline(modelData.key) ? "destructive" : "outline"
                                        size: "sm"
                                        theme: root.theme
                                        onClicked: {
                                            if (root.isDeviceOnline(modelData.key)) {
                                                root.disconnectDevice(modelData.key)
                                            } else {
                                                root.connectDevice(modelData.key)
                                            }
                                        }
                                    }

                                    Components.AppButton {
                                        Layout.fillWidth: true
                                        text: "测试"
                                        variant: "outline"
                                        size: "sm"
                                        enabled: root.isDeviceOnline(modelData.key)
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

    // DYN200 mode switch confirmation dialog (P0 safety)
    Components.AppAlertDialog {
        id: dyn200ModeConfirmDialog
        anchors.fill: parent
        theme: root.theme
        open: root.dyn200ModeConfirmVisible
        title: "⚠️ 危险操作确认"
        description: "您正在将 DYN200 扭矩传感器从主动上传模式切回 Modbus RTU 模式。\n\n" +
                     "⚠️ 此操作可能导致设备通信完全中断！\n\n" +
                     "如果通信中断，需要通过传感器物理按键恢复出厂设置才能恢复。\n\n" +
                     "目标模式：" + root.dyn200PendingModeLabel + "\n\n" +
                     "确认继续吗？"
        confirmText: "确认切换"
        cancelText: "取消"
        onConfirmed: {
            root.dyn200ConfirmAccepted = true
            root.deviceConfig.dyn200.communicationMode = parseInt(root.dyn200PendingModeValue)
            root.isModified = true
            root.dyn200ModeConfirmVisible = false
            root.dyn200PendingModeValue = ""
            root.dyn200PendingModeLabel = ""
        }
        onCancelled: {
            root.dyn200ModeConfirmVisible = false
            root.dyn200PendingModeValue = ""
            root.dyn200PendingModeLabel = ""
        }
    }

    Components.AppAlertDialog {
        id: testResultDialog
        anchors.fill: parent
        theme: root.theme
        open: root.testDialogOpen
        title: root.testResultTitle
        description: root.testResultText
        variant: root.testResultType === "error" ? "destructive" : "default"
        confirmText: "确定"
        cancelText: "关闭"
        onConfirmed: root.testDialogOpen = false
        onCancelled: root.testDialogOpen = false
    }
}
