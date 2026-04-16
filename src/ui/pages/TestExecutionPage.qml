pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../components" as Components

Item {
    id: root
    objectName: "testExecutionPage"

    required property Components.AppTheme theme

    property bool running: false
    property int currentPhaseIndex: -1
    property real elapsedSeconds: 0
    property real phaseElapsedSeconds: 0
    property real speedValue: 0
    property real torqueValue: 0
    property real powerValue: 0
    property real motorCurrentValue: 0
    property real brakeCurrentValue: 0
    property real angleValue: 0
    property real ai1Value: 0
    property real ai2Value: 0
    property string verdictState: "pending"
    property string verdictText: "待测试"
    property string infoText: ""
    property string infoType: ""
    property bool speedChannelOn: true
    property bool torqueChannelOn: true
    property bool currentChannelOn: false
    property bool angleChannelOn: true
    property var phaseDurations: [4.0, 12.0, 12.5, 10.0, 3.0]
    property var chartSpeed: []
    property var chartTorque: []
    property var chartCurrent: []
    property var chartAngle: []
    property string phaseTitle: currentPhaseIndex >= 0 ? stepModel.get(currentPhaseIndex).name : "等待开始"

    ListModel { id: stepModel }
    ListModel { id: angleModel }
    ListModel { id: loadModel }

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
        const angleRows = [
            ["①", "3.0°", "--", "--", "±3.0°", "待测"],
            ["②", "49.0°", "--", "--", "±3.0°", "待测"],
            ["①", "3.0°", "--", "--", "±3.0°", "待测"],
            ["③", "113.5°", "--", "--", "±3.0°", "待测"],
            ["零", "0.0°", "--", "--", "±3.0°", "待测"]
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
        loadModel.append({ direction: "正转", brakeCurrent: "--", torque: "--", limit: "≥ 1.20 N·m", result: "待测" })
        loadModel.append({ direction: "反转", brakeCurrent: "--", torque: "--", limit: "≥ 1.20 N·m", result: "待测" })
    }

    function stepStateForIndex(index) {
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

    function updateAngleTable() {
        const targetValues = [3.0, 49.0, 3.0, 113.5, 0.0]
        for (let i = 0; i < angleModel.count; ++i) {
            const actual = targetValues[i] + Math.sin(elapsedSeconds + i) * 0.8
            angleModel.setProperty(i, "currentAngle", Number(actual).toFixed(1) + "°")
            angleModel.setProperty(i, "deviation", (actual - targetValues[i]).toFixed(1) + "°")
            angleModel.setProperty(i, "result", Math.abs(actual - targetValues[i]) <= 3 ? "OK" : "NG")
        }
    }

    function updateLoadTable() {
        const forwardTorque = 1.38 + Math.sin(elapsedSeconds * 0.5) * 0.12
        const reverseTorque = 1.42 + Math.cos(elapsedSeconds * 0.5) * 0.10
        loadModel.setProperty(0, "brakeCurrent", (0.82 + Math.sin(elapsedSeconds) * 0.04).toFixed(2) + " A")
        loadModel.setProperty(0, "torque", forwardTorque.toFixed(2) + " N·m")
        loadModel.setProperty(0, "result", forwardTorque >= 1.2 ? "OK" : "NG")
        loadModel.setProperty(1, "brakeCurrent", (0.85 + Math.cos(elapsedSeconds) * 0.03).toFixed(2) + " A")
        loadModel.setProperty(1, "torque", reverseTorque.toFixed(2) + " N·m")
        loadModel.setProperty(1, "result", reverseTorque >= 1.2 ? "OK" : "NG")
    }

    function resetRun() {
        running = false
        currentPhaseIndex = -1
        elapsedSeconds = 0
        phaseElapsedSeconds = 0
        speedValue = 0
        torqueValue = 0
        powerValue = 0
        motorCurrentValue = 0
        brakeCurrentValue = 0
        angleValue = 0
        ai1Value = 0
        ai2Value = 0
        verdictState = "pending"
        verdictText = "待测试"
        chartSpeed = []
        chartTorque = []
        chartCurrent = []
        chartAngle = []
        initializeModels()
    }

    function startRun() {
        if (!commandBar.serialNumber || commandBar.serialNumber.length < 8) {
            infoText = "SN 长度至少 8 位"
            infoType = "error"
            return
        }

        resetRun()
        running = true
        currentPhaseIndex = 0
        infoText = "测试开始，进入 " + phaseTitle
        infoType = "success"
        applyStepStates()
        simulationTimer.start()
    }

    function stopRun() {
        running = false
        simulationTimer.stop()
        infoText = "急停触发，测试已中断"
        infoType = "error"
        applyStepStates()
    }

    function finishRun() {
        running = false
        simulationTimer.stop()
        currentPhaseIndex = stepModel.count - 1
        phaseElapsedSeconds = phaseDurations[currentPhaseIndex]
        applyStepStates()
        verdictState = "ok"
        verdictText = "OK"
        infoText = "测试完成，整机判定 OK"
        infoType = "success"
        speedValue = 0
        torqueValue = 0
        powerValue = 0
        motorCurrentValue = 0
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

    function simulateTick() {
        elapsedSeconds += 0.1
        phaseElapsedSeconds += 0.1

        if (currentPhaseIndex === 0) {
            speedValue = 180 + Math.sin(elapsedSeconds * 1.4) * 22
            motorCurrentValue = 0.42 + Math.sin(elapsedSeconds * 0.9) * 0.08
            torqueValue = 0.10 + Math.cos(elapsedSeconds) * 0.02
            brakeCurrentValue = 0
            angleValue = Math.max(0, 30 - phaseElapsedSeconds * 8.0)
        } else if (currentPhaseIndex === 1) {
            speedValue = 1240 + Math.sin(elapsedSeconds * 1.8) * 42
            motorCurrentValue = 2.14 + Math.cos(elapsedSeconds) * 0.14
            torqueValue = 0.31 + Math.sin(elapsedSeconds * 1.3) * 0.06
            brakeCurrentValue = 0
            angleValue = (angleValue + 2.6) % 360
        } else if (currentPhaseIndex === 2) {
            speedValue = 340 + Math.sin(elapsedSeconds * 2.1) * 60
            motorCurrentValue = 1.62 + Math.sin(elapsedSeconds * 1.5) * 0.22
            torqueValue = 0.48 + Math.cos(elapsedSeconds) * 0.08
            brakeCurrentValue = 0
            angleValue = 58 + Math.sin(elapsedSeconds * 0.6) * 55
            updateAngleTable()
        } else if (currentPhaseIndex === 3) {
            speedValue = 740 + Math.sin(elapsedSeconds * 1.6) * 85
            motorCurrentValue = 2.86 + Math.sin(elapsedSeconds * 1.4) * 0.18
            torqueValue = 1.38 + Math.cos(elapsedSeconds * 0.8) * 0.18
            brakeCurrentValue = 0.82 + Math.sin(elapsedSeconds) * 0.04
            angleValue = (angleValue + 1.1) % 360
            updateLoadTable()
        } else if (currentPhaseIndex === 4) {
            speedValue = Math.max(0, 160 - phaseElapsedSeconds * 48)
            motorCurrentValue = Math.max(0, 0.42 - phaseElapsedSeconds * 0.10)
            torqueValue = Math.max(0, 0.12 - phaseElapsedSeconds * 0.03)
            brakeCurrentValue = 0
            angleValue = Math.max(0, 12 - phaseElapsedSeconds * 4.0)
        }

        powerValue = Math.max(0, torqueValue * speedValue * Math.PI / 30.0)
        ai1Value = running && currentPhaseIndex >= 2 ? 1.0 : 0.0
        ai2Value = running && currentPhaseIndex >= 3 ? 1.0 : 0.0

        appendPoint(chartSpeed, speedValue)
        appendPoint(chartTorque, torqueValue)
        appendPoint(chartCurrent, motorCurrentValue)
        appendPoint(chartAngle, angleValue)
        applyStepStates()

        if (phaseElapsedSeconds >= phaseDurations[currentPhaseIndex]) {
            stepModel.setProperty(currentPhaseIndex, "state", "done")
            stepModel.setProperty(currentPhaseIndex, "elapsed", formatSeconds(phaseDurations[currentPhaseIndex]))
            currentPhaseIndex += 1
            phaseElapsedSeconds = 0

            if (currentPhaseIndex >= stepModel.count) {
                finishRun()
                return
            }

            stepModel.setProperty(currentPhaseIndex, "state", "run")
            infoText = "阶段切换到 " + stepModel.get(currentPhaseIndex).name
            infoType = "warning"
        }
    }

    Timer {
        id: simulationTimer
        interval: 100
        repeat: true
        running: false
        onTriggered: root.simulateTick()
    }

    Component.onCompleted: {
        commandBar.serialNumber = "GBX42A-20260415-001"
        commandBar.backlashValue = "1.2"
        resetRun()
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
