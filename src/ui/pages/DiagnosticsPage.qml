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

    property bool autoRefresh: true
    property int refreshInterval: 2000
    property string pendingMotorAction: ""
    property bool motorConfirmVisible: false
    property string brakeCurrentInput: ""
    property string brakeVoltageInput: ""
    property string brakeModeValue: root.brakeTel.mode || "CC"
    property string encoderResolutionInput: "32768"
    property string encoderCommModeInput: "0"
    property string encoderPollIntervalInput: "5"

    property var motorTel: viewModel ? viewModel.motorTelemetry : ({"currentA": 0, "ai1Level": false, "online": false})
    property var torqueTel: viewModel ? viewModel.torqueTelemetry : ({"torqueNm": 0, "speedRpm": 0, "powerW": 0, "online": false})
    property var encoderTel: viewModel ? viewModel.encoderTelemetry : ({"angleDeg": 0, "totalAngleDeg": 0, "velocityRpm": 0, "online": false})
    property var brakeTel: viewModel ? viewModel.brakeTelemetry : ({"currentA": 0, "voltageV": 0, "powerW": 0, "mode": "CC", "channel": 0, "online": false})

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

    Component.onCompleted: {
        if (root.viewModel) {
            root.encoderResolutionInput = root.viewModel.encoderResolution.toString()
            root.encoderCommModeInput = root.viewModel.encoderCommMode.toString()
            root.encoderPollIntervalInput = root.viewModel.encoderPollInterval.toString()
        }
    }

    Connections {
        target: root.viewModel
        function onEncoderParamsChanged() {
            root.encoderResolutionInput = root.viewModel.encoderResolution.toString()
            root.encoderCommModeInput = root.viewModel.encoderCommMode.toString()
            root.encoderPollIntervalInput = root.viewModel.encoderPollInterval.toString()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            // Header card
            Components.AppCard {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
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

            // Tab bar
            Components.AppTabs {
                id: mainTabs
                Layout.fillWidth: true
                theme: root.theme
                showContent: false
                model: [
                    {text: "设备总览"},
                    {text: "AQMD 电机驱动器"},
                    {text: "DYN200 扭矩传感器"},
                    {text: "单圈绝对值编码器"},
                    {text: "制动电源"}
                ]
            }

            // Tab content area
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: mainTabs.currentIndex

                Loader {
                    active: StackLayout.index === 0 || StackLayout.isCurrentItem
                    sourceComponent: tab0Overview
                }
                Loader {
                    active: StackLayout.index === 1 || StackLayout.isCurrentItem
                    sourceComponent: tab1Motor
                }
                Loader {
                    active: StackLayout.index === 2 || StackLayout.isCurrentItem
                    sourceComponent: tab2Torque
                }
                Loader {
                    active: StackLayout.index === 3 || StackLayout.isCurrentItem
                    sourceComponent: tab3Encoder
                }
                Loader {
                    active: StackLayout.index === 4 || StackLayout.isCurrentItem
                    sourceComponent: tab4Brake
                }
            }

            // Communication log collapsible
            Components.AppCollapsible {
                id: logCollapsible
                Layout.fillWidth: true
                Layout.preferredHeight: open ? 320 : implicitHeight
                theme: root.theme
                title: "通信日志"
                open: false

                triggerContent: Components.AppButton {
                    text: "清空"
                    variant: "outline"
                    size: "sm"
                    theme: root.theme
                    onClicked: if (root.viewModel) root.viewModel.clearLog()
                }

                ListView {
                    width: parent.width
                    height: 240
                    clip: true
                    spacing: 2
                    model: root.viewModel ? root.viewModel.communicationLogs : []

                    delegate: Rectangle {
                        required property string time
                        required property string direction
                        required property string device
                        required property string message
                        required property bool success
                        width: ListView.view.width
                        height: rowLayout.implicitHeight + 8
                        color: index % 2 === 0 ? "transparent" : Qt.rgba(root.theme.muted.r, root.theme.muted.g, root.theme.muted.b, 0.3)
                        radius: 2

                        RowLayout {
                            id: rowLayout
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            Components.AppBadge {
                                text: time
                                variant: "outline"
                                theme: root.theme
                            }

                            Components.AppBadge {
                                text: direction
                                variant: "outline"
                                theme: root.theme
                            }

                            Components.AppBadge {
                                text: device
                                variant: "outline"
                                theme: root.theme
                            }

                            Components.AppLabel {
                                text: message
                                fontSize: 11
                                theme: root.theme
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                maximumLineCount: 2
                            }

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                color: success ? root.theme.okColor : root.theme.ngColor
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Tab 0: Device overview ──
    Component {
        id: tab0Overview

        ColumnLayout {
            spacing: 12

            Components.AppTable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                theme: root.theme
                headers: ["设备名称", "状态", "关键遥测", "最后更新", "错误数"]
                columnKeys: ["name", "statusText", "summary", "lastUpdate", "errorCountText"]
                rowVariantKey: "variant"
                rows: {
                    var result = [];
                    var statuses = root.viewModel ? root.viewModel.deviceStatuses : [];
                    for (var i = 0; i < statuses.length; i++) {
                        var s = statuses[i];
                        result.push({
                            "name": s.name || "",
                            "statusText": s.status === "online" ? "在线" : "离线",
                            "summary": s.summary || "",
                            "lastUpdate": s.lastUpdate || "",
                            "errorCountText": (s.errorCount || 0) + " 次",
                            "variant": s.status === "online" ? "success" : "danger"
                        });
                    }
                    return result;
                }
            }

            Components.AppAlert {
                Layout.fillWidth: true
                theme: root.theme
                description: root.viewModel ? root.viewModel.statusMessage : ""
                visible: description.length > 0
            }
        }
    }

    // ── Tab 1: AQMD Motor Driver ──
    Component {
        id: tab1Motor

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // Device info card
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        spacing: 10

                        Components.AppLabel {
                            text: "AQMD 电机驱动器"
                            fontSize: 15
                            fontWeight: 600
                            theme: root.theme
                        }

                        Components.AppBadge {
                            text: root.motorTel.online ? "在线" : "离线"
                            variant: root.motorTel.online ? "success" : "destructive"
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppLabel {
                            visible: root.viewModel && root.viewModel.deviceStatuses.length > 0
                            text: root.viewModel ? root.viewModel.deviceStatuses[0].lastUpdate : ""
                            fontSize: 11
                            color: root.theme.textSecondary
                            theme: root.theme
                        }
                    }

                    Components.AppLabel {
                        visible: root.viewModel && root.viewModel.deviceStatuses.length > 0 && root.viewModel.deviceStatuses[0].errorCount > 0
                        text: "通信错误: " + (root.viewModel ? root.viewModel.deviceStatuses[0].errorCount : 0) + " 次"
                        fontSize: 11
                        color: root.theme.ngColor
                        theme: root.theme
                    }
                }
            }

            // Telemetry cards
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "电机电流"
                    value: root.motorTel.currentA ? root.motorTel.currentA.toFixed(2) : "0.00"
                    unit: "A"
                    subtext: root.motorTel.online ? "实时" : "离线"
                    accentColor: root.motorTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "AI1 电平"
                    value: root.motorTel.ai1Level ? "高" : "低"
                    unit: ""
                    subtext: "数字输入"
                    accentColor: root.motorTel.online ? root.theme.ok : root.theme.textMuted
                }
            }

            // Motor controls
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppLabel {
                        text: "电机控制"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    RowLayout {
                        spacing: 10

                        Components.AppButton {
                            text: "正转"
                            variant: "default"
                            theme: root.theme
                            onClicked: {
                                root.pendingMotorAction = "forward"
                                root.motorConfirmVisible = true
                            }
                        }

                        Components.AppButton {
                            text: "反转"
                            variant: "default"
                            theme: root.theme
                            onClicked: {
                                root.pendingMotorAction = "reverse"
                                root.motorConfirmVisible = true
                            }
                        }

                        Components.AppButton {
                            text: "停止"
                            variant: "destructive"
                            theme: root.theme
                            onClicked: if (root.viewModel) root.viewModel.stopMotor()
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // ── Tab 2: DYN200 Torque Sensor ──
    Component {
        id: tab2Torque

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        spacing: 10

                        Components.AppLabel {
                            text: "DYN200 扭矩传感器"
                            fontSize: 15
                            fontWeight: 600
                            theme: root.theme
                        }

                        Components.AppBadge {
                            text: root.torqueTel.online ? "在线" : "离线"
                            variant: root.torqueTel.online ? "success" : "destructive"
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppLabel {
                            visible: root.viewModel && root.viewModel.deviceStatuses.length > 1
                            text: root.viewModel && root.viewModel.deviceStatuses.length > 1 ? root.viewModel.deviceStatuses[1].lastUpdate : ""
                            fontSize: 11
                            color: root.theme.textSecondary
                            theme: root.theme
                        }
                    }

                    Components.AppLabel {
                        visible: root.viewModel && root.viewModel.deviceStatuses.length > 1 && root.viewModel.deviceStatuses[1].errorCount > 0
                        text: "通信错误: " + (root.viewModel && root.viewModel.deviceStatuses.length > 1 ? root.viewModel.deviceStatuses[1].errorCount : 0) + " 次"
                        fontSize: 11
                        color: root.theme.ngColor
                        theme: root.theme
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "扭矩"
                    value: root.torqueTel.torqueNm ? root.torqueTel.torqueNm.toFixed(2) : "0.00"
                    unit: "Nm"
                    subtext: root.torqueTel.online ? "实时" : "离线"
                    accentColor: root.torqueTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "转速"
                    value: root.torqueTel.speedRpm ? root.torqueTel.speedRpm.toFixed(0) : "0"
                    unit: "RPM"
                    subtext: root.torqueTel.online ? "实时" : "离线"
                    accentColor: root.torqueTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "功率"
                    value: root.torqueTel.powerW ? root.torqueTel.powerW.toFixed(1) : "0.0"
                    unit: "W"
                    subtext: root.torqueTel.online ? "实时" : "离线"
                    accentColor: root.torqueTel.online ? root.theme.ok : root.theme.textMuted
                }
            }

            Components.AppAlert {
                Layout.fillWidth: true
                theme: root.theme
                variant: "default"
                description: "此设备为只读传感器，无控制功能"
            }

            Item { Layout.fillHeight: true }
        }
    }

    // ── Tab 3: Single-turn Absolute Encoder ──
    Component {
        id: tab3Encoder

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        spacing: 10

                        Components.AppLabel {
                            text: "单圈绝对值编码器"
                            fontSize: 15
                            fontWeight: 600
                            theme: root.theme
                        }

                        Components.AppBadge {
                            text: root.encoderTel.online ? "在线" : "离线"
                            variant: root.encoderTel.online ? "success" : "destructive"
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppLabel {
                            visible: root.viewModel && root.viewModel.deviceStatuses.length > 2
                            text: root.viewModel && root.viewModel.deviceStatuses.length > 2 ? root.viewModel.deviceStatuses[2].lastUpdate : ""
                            fontSize: 11
                            color: root.theme.textSecondary
                            theme: root.theme
                        }
                    }

                    Components.AppLabel {
                        visible: root.viewModel && root.viewModel.deviceStatuses.length > 2 && root.viewModel.deviceStatuses[2].errorCount > 0
                        text: "通信错误: " + (root.viewModel && root.viewModel.deviceStatuses.length > 2 ? root.viewModel.deviceStatuses[2].errorCount : 0) + " 次"
                        fontSize: 11
                        color: root.theme.ngColor
                        theme: root.theme
                    }
                }
            }

            Components.MetricCard {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                theme: root.theme
                label: "当前角度"
                value: root.encoderTel.angleDeg !== undefined ? Number(root.encoderTel.angleDeg).toFixed(2) : "0.00"
                unit: "deg"
                subtext: root.encoderTel.online ? "实时" : "离线"
                accentColor: root.encoderTel.online ? root.theme.ok : root.theme.textMuted
            }

            // Encoder parameters (editable)
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    Components.AppLabel {
                        text: "编码器参数"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 12
                        columnSpacing: 16

                        Components.AppLabel {
                            text: "分辨率:"
                            fontSize: 12
                            theme: root.theme
                        }

                        Components.AppSelect {
                            Layout.preferredWidth: 200
                            currentValue: root.encoderResolutionInput
                            model: [
                                {value: "4096", label: "4096 脉冲/圈"},
                                {value: "8192", label: "8192 脉冲/圈"},
                                {value: "16384", label: "16384 脉冲/圈"},
                                {value: "32768", label: "32768 脉冲/圈"}
                            ]
                            theme: root.theme
                            onCurrentValueChanged: root.encoderResolutionInput = currentValue
                        }

                        Components.AppLabel {
                            text: "通信模式:"
                            fontSize: 12
                            theme: root.theme
                        }

                        Components.AppSelect {
                            Layout.preferredWidth: 200
                            currentValue: root.encoderCommModeInput
                            model: [
                                {value: "0", label: "查询模式"},
                                {value: "1", label: "自动上报模式"}
                            ]
                            theme: root.theme
                            onCurrentValueChanged: root.encoderCommModeInput = currentValue
                        }

                        Components.AppLabel {
                            text: "轮询间隔:"
                            fontSize: 12
                            theme: root.theme
                        }

                        Components.AppSelect {
                            Layout.preferredWidth: 200
                            currentValue: root.encoderPollIntervalInput
                            model: [
                                {value: "5", label: "5 ms"},
                                {value: "10", label: "10 ms"},
                                {value: "20", label: "20 ms"},
                                {value: "50", label: "50 ms"},
                                {value: "100", label: "100 ms"}
                            ]
                            theme: root.theme
                            onCurrentValueChanged: root.encoderPollIntervalInput = currentValue
                        }

                        Components.AppLabel {
                            text: "成功率:"
                            fontSize: 12
                            theme: root.theme
                        }

                        Components.AppLabel {
                            text: {
                                if (!root.viewModel || root.viewModel.deviceStatuses.length <= 2) return "N/A"
                                var status = root.viewModel.deviceStatuses[2]
                                var total = (status.successCount || 0) + (status.errorCount || 0)
                                if (total === 0) return "N/A"
                                var rate = ((status.successCount || 0) / total * 100).toFixed(1)
                                return rate + "%"
                            }
                            fontSize: 12
                            fontWeight: 600
                            color: {
                                if (!root.viewModel || root.viewModel.deviceStatuses.length <= 2) return root.theme.textPrimary
                                var status = root.viewModel.deviceStatuses[2]
                                var total = (status.successCount || 0) + (status.errorCount || 0)
                                if (total === 0) return root.theme.textPrimary
                                var rate = (status.successCount || 0) / total
                                return rate >= 0.95 ? root.theme.okColor : (rate >= 0.8 ? root.theme.warningColor : root.theme.ngColor)
                            }
                            theme: root.theme
                        }
                    }

                    Components.AppButton {
                        text: "写入编码器"
                        variant: "default"
                        theme: root.theme
                        onClicked: {
                            if (root.viewModel) {
                                root.viewModel.setEncoderResolution(parseInt(root.encoderResolutionInput))
                                root.viewModel.setEncoderCommMode(parseInt(root.encoderCommModeInput))
                                root.viewModel.setEncoderPollInterval(parseInt(root.encoderPollIntervalInput))
                            }
                        }
                    }
                }
            }

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppLabel {
                        text: "标定"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    Components.AppLabel {
                        text: "将当前位置设为零点参考："
                        fontSize: 12
                        color: root.theme.textSecondary
                        theme: root.theme
                    }

                    Components.AppButton {
                        text: "置零"
                        variant: "default"
                        theme: root.theme
                        onClicked: if (root.viewModel) root.viewModel.setEncoderZero()
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // ── Tab 4: Brake Power Supply ──
    Component {
        id: tab4Brake

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        spacing: 10

                        Components.AppLabel {
                            text: "制动电源"
                            fontSize: 15
                            fontWeight: 600
                            theme: root.theme
                        }

                        Components.AppBadge {
                            text: root.brakeTel.online ? "在线" : "离线"
                            variant: root.brakeTel.online ? "success" : "destructive"
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppLabel {
                            visible: root.viewModel && root.viewModel.deviceStatuses.length > 3
                            text: root.viewModel && root.viewModel.deviceStatuses.length > 3 ? root.viewModel.deviceStatuses[3].lastUpdate : ""
                            fontSize: 11
                            color: root.theme.textSecondary
                            theme: root.theme
                        }
                    }

                    Components.AppLabel {
                        visible: root.viewModel && root.viewModel.deviceStatuses.length > 3 && root.viewModel.deviceStatuses[3].errorCount > 0
                        text: "通信错误: " + (root.viewModel && root.viewModel.deviceStatuses.length > 3 ? root.viewModel.deviceStatuses[3].errorCount : 0) + " 次"
                        fontSize: 11
                        color: root.theme.ngColor
                        theme: root.theme
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "输出电流"
                    value: root.brakeTel.currentA !== undefined ? Number(root.brakeTel.currentA).toFixed(2) : "0.00"
                    unit: "A"
                    subtext: root.brakeTel.online ? "实时" : "离线"
                    accentColor: root.brakeTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "输出电压"
                    value: root.brakeTel.voltageV !== undefined ? Number(root.brakeTel.voltageV).toFixed(2) : "0.00"
                    unit: "V"
                    subtext: root.brakeTel.online ? "实时" : "离线"
                    accentColor: root.brakeTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "输出功率"
                    value: root.brakeTel.powerW !== undefined ? Number(root.brakeTel.powerW).toFixed(1) : "0.0"
                    unit: "W"
                    subtext: root.brakeTel.online ? "实时" : "离线"
                    accentColor: root.brakeTel.online ? root.theme.ok : root.theme.textMuted
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "工作模式"
                    value: root.brakeTel.mode || "CC"
                    unit: ""
                    subtext: "恒流/恒压"
                    accentColor: root.brakeTel.online ? root.theme.ok : root.theme.textMuted
                }

                Components.MetricCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    theme: root.theme
                    label: "输出通道"
                    value: root.brakeTel.channel !== undefined ? String(root.brakeTel.channel) : "0"
                    unit: "CH"
                    subtext: "当前通道"
                    accentColor: root.brakeTel.online ? root.theme.ok : root.theme.textMuted
                }
            }

            // Brake mode switching
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppLabel {
                        text: "制动模式"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    RowLayout {
                        spacing: 10

                        Components.AppButton {
                            text: "恒流 (CC)"
                            variant: root.brakeModeValue === "CC" ? "default" : "outline"
                            theme: root.theme
                            onClicked: {
                                root.brakeModeValue = "CC"
                                if (root.viewModel && typeof root.viewModel.setBrakeMode === 'function')
                                    root.viewModel.setBrakeMode("CC")
                            }
                        }

                        Components.AppButton {
                            text: "恒压 (CV)"
                            variant: root.brakeModeValue === "CV" ? "default" : "outline"
                            theme: root.theme
                            onClicked: {
                                root.brakeModeValue = "CV"
                                if (root.viewModel && typeof root.viewModel.setBrakeMode === 'function')
                                    root.viewModel.setBrakeMode("CV")
                            }
                        }

                        Components.AppLabel {
                            text: "当前: " + root.brakeModeValue
                            fontSize: 12
                            color: root.theme.textSecondary
                            theme: root.theme
                        }
                    }
                }
            }

            // Output controls
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppLabel {
                        text: "输出控制"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    RowLayout {
                        spacing: 10

                        Components.AppButton {
                            text: "使能输出"
                            variant: "default"
                            theme: root.theme
                            onClicked: if (root.viewModel) root.viewModel.setBrakeOutput(true)
                        }

                        Components.AppButton {
                            text: "禁用输出"
                            variant: "destructive"
                            theme: root.theme
                            onClicked: if (root.viewModel) root.viewModel.setBrakeOutput(false)
                        }
                    }
                }
            }

            // Parameter settings
            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppLabel {
                        text: "参数设置"
                        fontSize: 13
                        fontWeight: 600
                        theme: root.theme
                    }

                    Components.AppLabel {
                        text: "电流范围 0~5 A，电压范围 0~24 V（超出自动限幅）"
                        fontSize: 11
                        color: root.theme.textSecondary
                        theme: root.theme
                    }

                    RowLayout {
                        spacing: 12

                        Components.AppInput {
                            Layout.preferredWidth: 140
                            theme: root.theme
                            label: "电流 (A) 0~5:"
                            placeholderText: "0.00 ~ 5.00"
                            text: root.brakeCurrentInput
                            onTextChanged: root.brakeCurrentInput = text
                            tone: root.brakeCurrentInput.length > 0 && (parseFloat(root.brakeCurrentInput) < 0 || parseFloat(root.brakeCurrentInput) > 5) ? "danger" : "default"
                        }

                        Components.AppButton {
                            text: "设置电流"
                            variant: "default"
                            theme: root.theme
                            enabled: root.brakeCurrentInput.length > 0 && !isNaN(parseFloat(root.brakeCurrentInput))
                            onClicked: {
                                if (root.viewModel && root.brakeCurrentInput.length > 0) {
                                    var val = parseFloat(root.brakeCurrentInput)
                                    if (!isNaN(val))
                                        root.viewModel.setBrakeCurrent(val)
                                }
                            }
                        }

                        Components.AppInput {
                            Layout.preferredWidth: 140
                            theme: root.theme
                            label: "电压 (V) 0~24:"
                            placeholderText: "0.00 ~ 24.00"
                            text: root.brakeVoltageInput
                            onTextChanged: root.brakeVoltageInput = text
                            tone: root.brakeVoltageInput.length > 0 && (parseFloat(root.brakeVoltageInput) < 0 || parseFloat(root.brakeVoltageInput) > 24) ? "danger" : "default"
                        }

                        Components.AppButton {
                            text: "设置电压"
                            variant: "default"
                            theme: root.theme
                            enabled: root.brakeVoltageInput.length > 0 && !isNaN(parseFloat(root.brakeVoltageInput))
                            onClicked: {
                                if (root.viewModel && root.brakeVoltageInput.length > 0) {
                                    var val = parseFloat(root.brakeVoltageInput)
                                    if (!isNaN(val))
                                        root.viewModel.setBrakeVoltage(val)
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Motor confirmation dialog
    Components.AppAlertDialog {
        id: motorConfirmDialog
        anchors.fill: parent
        theme: root.theme
        open: root.motorConfirmVisible
        title: "确认操作"
        description: root.pendingMotorAction === "forward" ? "确认电机正转？" : "确认电机反转？"
        confirmText: "确认"
        cancelText: "取消"
        onConfirmed: {
            if (root.pendingMotorAction === "forward" && root.viewModel)
                root.viewModel.setMotorForward()
            else if (root.pendingMotorAction === "reverse" && root.viewModel)
                root.viewModel.setMotorReverse()
            root.motorConfirmVisible = false
            root.pendingMotorAction = ""
        }
        onCancelled: {
            root.motorConfirmVisible = false
            root.pendingMotorAction = ""
        }
    }
}
