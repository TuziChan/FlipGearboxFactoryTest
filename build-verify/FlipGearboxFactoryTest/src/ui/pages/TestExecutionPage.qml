pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../components" as Components

Item {
    id: root
    objectName: "testExecutionPage"

    required property Components.AppTheme theme

    // ViewModel connection - exposed from main.cpp as "testViewModel"
    property var viewModel: typeof testViewModel !== "undefined" ? testViewModel : null

    // Local UI state derived from ViewModel
    property bool running: viewModel ? viewModel.running : false
    property int currentPhaseIndex: -1
    property real elapsedSeconds: viewModel ? viewModel.elapsedMs / 1000.0 : 0
    property real phaseElapsedSeconds: 0
    property real speedValue: viewModel ? viewModel.speed : 0
    property real torqueValue: viewModel ? viewModel.torque : 0
    property real powerValue: viewModel ? viewModel.power : 0
    property real motorCurrentValue: viewModel ? viewModel.motorCurrent : 0
    property real brakeCurrentValue: viewModel ? viewModel.brakeCurrent : 0
    property real angleValue: viewModel ? viewModel.angle : 0
    property real ai1Value: viewModel ? (viewModel.ai1Level ? 1.0 : 0.0) : 0
    property real ai2Value: 0
    property string verdictState: viewModel ? (viewModel.testPassed ? "ok" : "pending") : "pending"
    property string verdictText: viewModel ? viewModel.overallVerdict : "待测试"
    property string infoText: viewModel ? viewModel.statusMessage : ""
    property string infoType: ""
    property bool speedChannelOn: true
    property bool torqueChannelOn: true
    property bool currentChannelOn: false
    property bool angleChannelOn: true
    property var phaseDurations: []
    property var chartSpeed: []
    property var chartTorque: []
    property var chartCurrent: []
    property var chartAngle: []
    property string phaseTitle: viewModel ? viewModel.currentPhase : "等待开始"

    ListModel { id: stepModel }
    ListModel { id: angleModel }
    ListModel { id: loadModel }

    // Connect to ViewModel signals
    Connections {
        target: root.viewModel
        enabled: root.viewModel !== null

        function onTelemetryChanged() {
            // Update chart data with new telemetry
            appendPoint(root.chartSpeed, root.speedValue)
            appendPoint(root.chartTorque, root.torqueValue)
            appendPoint(root.chartCurrent, root.motorCurrentValue)
            appendPoint(root.chartAngle, root.angleValue)
        }

        function onCurrentPhaseChanged() {
            updateCurrentPhaseIndex()
            root.infoText = "阶段切换到 " + root.phaseTitle
            root.infoType = "warning"
            applyStepStates()
        }

        function onRunningChanged() {
            if (root.running) {
                root.infoText = "测试开始，进入 " + root.phaseTitle
                root.infoType = "success"
            } else {
                root.infoText = "测试已停止"
                root.infoType = "warning"
            }
            applyStepStates()
        }

        function onResultsChanged() {
            applyStepStates()
            if (root.viewModel.testPassed) {
                root.infoText = "测试完成，整机判定 OK"
                root.infoType = "success"
            } else {
                root.infoText = "测试完成，整机判定 NG"
                root.infoType = "error"
            }
        }

        function onErrorOccurred(message) {
            root.infoText = message
            root.infoType = "error"
        }
    }

    function formatSeconds(seconds) {
        const mins = Math.floor(seconds / 60)
        const secs = seconds - mins * 60
        const secText = secs < 10 ? "0" + secs.toFixed(1) : secs.toFixed(1)
        return (mins < 10 ? "0" + mins : mins) + ":" + secText
    }

    function metricColor(label) {
        if (label === "角度")
            return theme.accent
        if (label === "制动电流" && brakeCurrentValue > 0)
            return theme.warnColor
        if (label === "扭矩" && torqueValue > 1.2)
            return theme.okColor
        return theme.textPrimary
    }

    function metricValue(label) {
        if (label === "转速")
            return speedValue.toFixed(0)
        if (label === "扭矩")
            return torqueValue.toFixed(2)
        if (label === "功率")
            return powerValue.toFixed(0)
        if (label === "电机电流")
            return motorCurrentValue.toFixed(2)
        if (label === "制动电流")
            return brakeCurrentValue.toFixed(2)
        return angleValue.toFixed(1)
    }

    function metricUnit(label) {
        if (label === "转速")
            return "RPM"
        if (label === "扭矩")
            return "N·m"
        if (label === "功率")
            return "W"
        if (label === "角度")
            return "°"
        return "A"
    }

    function initializeModels() {
        stepModel.clear()
        const steps = [
            { name: "准备/找零", detail: "编码器归零并建立补偿", state: "wait", elapsed: "00:00.0" },
            { name: "空载正反转", detail: "采集正反转电流与转速", state: "wait", elapsed: "00:00.0" },
            { name: "角度定位", detail: "执行五个目标位定位判定", state: "wait", elapsed: "00:00.0" },
            { name: "负载上升", detail: "锁止并采集制动扭矩", state: "wait", elapsed: "00:00.0" },
            { name: "回零结束", detail: "回到零位并汇总结果", state: "wait", elapsed: "00:00.0" }
        ]
        for (let i = 0; i < steps.length; ++i)
            stepModel.append(steps[i])

        angleModel.clear()
        var angleRows = [
            ["①", "3.0°", "--", "--", "±3.0°", "待测"],
            ["②", "49.0°", "--", "--", "±3.0°", "待测"],
            ["①", "3.0°", "--", "--", "±3.0°", "待测"],
            ["③", "113.5°", "--", "--", "±3.0°", "待测"],
            ["零", "0.0°", "--", "--", "±1.0°", "待测"]
        ]
        for (let j = 0; j < angleRows.length; ++j) {
            angleModel.append({
                target: angleRows[j][0],
                targetAngle: angleRows[j][1],
                currentAngle: angleRows[j][2],
                deviation: angleRows[j][3],
                tolerance: angleRows[j][4],
                result: angleRows[j][5]
            })
        }

        loadModel.clear()
        loadModel.append({ direction: "正转", brakeCurrent: "--", torque: "--", limit: "-- N·m", result: "待测" })
        loadModel.append({ direction: "反转", brakeCurrent: "--", torque: "--", limit: "-- N·m", result: "待测" })
    }

    function updateCurrentPhaseIndex() {
        const phaseName = root.phaseTitle
        if (phaseName.includes("准备") || phaseName.includes("找零") || phaseName.includes("Homing"))
            currentPhaseIndex = 0
        else if (phaseName.includes("空载") || phaseName.includes("Idle"))
            currentPhaseIndex = 1
        else if (phaseName.includes("角度") || phaseName.includes("Angle"))
            currentPhaseIndex = 2
        else if (phaseName.includes("负载") || phaseName.includes("Load"))
            currentPhaseIndex = 3
        else if (phaseName.includes("回零") || phaseName.includes("Return"))
            currentPhaseIndex = 4
        else
            currentPhaseIndex = -1
    }

    function stepStateForIndex(index) {
        if (currentPhaseIndex < 0)
            return "wait"
        if (index < currentPhaseIndex)
            return "done"
        if (index === currentPhaseIndex)
            return running ? "run" : "done"
        return "wait"
    }

    function applyStepStates() {
        for (let i = 0; i < stepModel.count; ++i) {
            stepModel.setProperty(i, "state", stepStateForIndex(i))
            if (i === currentPhaseIndex)
                stepModel.setProperty(i, "elapsed", formatSeconds(phaseElapsedSeconds))
        }
    }

    function appendPoint(array, value) {
        array.push(value)
        if (array.length > 120)
            array.shift()
    }

    function resetRun() {
        currentPhaseIndex = -1
        phaseElapsedSeconds = 0
        chartSpeed = []
        chartTorque = []
        chartCurrent = []
        chartAngle = []
        initializeModels()
        
        if (viewModel) {
            viewModel.resetTest()
        }
    }

    function startRun() {
        if (!commandBar.serialNumber || commandBar.serialNumber.length < 8) {
            infoText = "SN 长度至少 8 位"
            infoType = "error"
            return
        }

        if (!viewModel) {
            infoText = "ViewModel 未连接，无法启动测试"
            infoType = "error"
            return
        }

        const backlashDeg = Number(commandBar.backlashValue)
        if (commandBar.backlashValue !== "" && Number.isNaN(backlashDeg)) {
            infoText = "回差补偿必须是数字"
            infoType = "error"
            return
        }

        resetRun()
        viewModel.loadRecipe(commandBar.modelText)
        viewModel.selectedModel = commandBar.modelText
        viewModel.setSerialNumber(commandBar.serialNumber)
        viewModel.backlashCompensationDeg = Number.isNaN(backlashDeg) ? 0 : backlashDeg
        viewModel.startTest()
    }

    function stopRun() {
        if (viewModel) {
            viewModel.stopTest()
        }
        infoText = "急停触发，测试已中断"
        infoType = "error"
        applyStepStates()
    }

    function toggleChannel(channelName) {
        if (channelName === "speed")
            speedChannelOn = !speedChannelOn
        else if (channelName === "torque")
            torqueChannelOn = !torqueChannelOn
        else if (channelName === "current")
            currentChannelOn = !currentChannelOn
        else if (channelName === "angle")
            angleChannelOn = !angleChannelOn
    }

    Component.onCompleted: {
        commandBar.serialNumber = ""
        commandBar.backlashValue = ""
        initializeModels()
        
        if (!viewModel) {
            console.warn("TestExecutionPage: testViewModel not found in context")
            infoText = "警告：ViewModel 未连接，UI 将无法控制测试"
            infoType = "warning"
        } else {
            console.log("TestExecutionPage: Connected to testViewModel")
            commandBar.modelIndex = 0
            viewModel.loadRecipe(commandBar.modelText)
            updateCurrentPhaseIndex()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Components.CommandBar {
                id: commandBar
                Layout.fillWidth: true
                theme: root.theme
                running: root.running
                onStartRequested: root.startRun
                onStopRequested: root.stopRun
            }

            Components.TestExecutionWorkspace {
                Layout.fillWidth: true
                Layout.fillHeight: true
                theme: root.theme
                hostWidth: root.width
                currentPhaseIndex: root.currentPhaseIndex
                phaseTitle: root.phaseTitle
                totalTimeText: root.formatSeconds(root.elapsedSeconds)
                stepModel: stepModel
                angleModel: angleModel
                loadModel: loadModel
                chartSpeed: root.chartSpeed
                chartTorque: root.chartTorque
                chartCurrent: root.chartCurrent
                chartAngle: root.chartAngle
                running: root.running
                speedChannelOn: root.speedChannelOn
                torqueChannelOn: root.torqueChannelOn
                currentChannelOn: root.currentChannelOn
                angleChannelOn: root.angleChannelOn
                speedValue: root.speedValue
                torqueValue: root.torqueValue
                ai1Value: root.ai1Value
                ai2Value: root.ai2Value
                verdictState: root.verdictState
                verdictText: root.verdictText
                metricColorProvider: root.metricColor
                metricValueProvider: root.metricValue
                metricUnitProvider: root.metricUnit
                onToggleChannel: root.toggleChannel
                onCopyReport: function() {
                    root.infoText = "报告已生成，可在后续版本接入系统剪贴板"
                    root.infoType = "warning"
                }
                onResetRequested: root.resetRun
            }

            Components.AlertStrip {
                Layout.fillWidth: true
                theme: root.theme
                variant: root.infoType === "error" ? "danger" : "warning"
                title: root.infoType === "success" ? "OK" : root.infoType === "error" ? "NG" : "提示"
                text: root.infoText
            }
        }
    }
}
