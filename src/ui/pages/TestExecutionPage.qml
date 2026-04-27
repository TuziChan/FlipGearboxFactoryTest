pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components" as Components

Item {
    id: root
    objectName: "testExecutionPage"

    required property Components.AppTheme theme

    // ViewModel connection - exposed from main.cpp as "testViewModel"
    property var viewModel: typeof testViewModel !== "undefined" ? testViewModel : null
    property var recipeVM: typeof recipeViewModel !== "undefined" ? recipeViewModel : null
    property var recipeOptions: []

    // Local UI state derived from ViewModel
    property bool running: viewModel ? viewModel.running : false
    property real elapsedSeconds: viewModel ? viewModel.elapsedMs / 1000.0 : 0
    property real phaseElapsedSeconds: 0
    property real speedValue: viewModel ? viewModel.speed : 0
    property real torqueValue: viewModel ? viewModel.torque : 0
    property real powerValue: viewModel ? viewModel.power : 0
    property real motorCurrentValue: viewModel ? viewModel.motorCurrent : 0
    property bool motorOnlineValue: viewModel ? viewModel.motorOnline : false
    property real brakeCurrentValue: viewModel ? viewModel.brakeCurrent : 0
    property real brakeVoltageValue: viewModel ? viewModel.brakeVoltage : 0
    property real brakePowerValue: viewModel ? viewModel.brakePower : 0
    property bool brakeOnlineValue: viewModel ? viewModel.brakeOnline : false
    property real angleValue: viewModel ? viewModel.angle : 0
    property real encoderTotalAngleValue: viewModel ? viewModel.encoderTotalAngle : 0
    property real encoderVelocityValue: viewModel ? viewModel.encoderVelocity : 0
    property bool encoderOnlineValue: viewModel ? viewModel.encoderOnline : false
    property bool torqueOnlineValue: viewModel ? viewModel.torqueOnline : false
    property real ai1Value: viewModel ? (viewModel.ai1Level ? 1.0 : 0.0) : 0
    property real ai2Value: 0
    property string verdictState: viewModel ? (viewModel.testPassed ? "ok" : "pending") : "pending"
    property string verdictText: viewModel ? viewModel.overallVerdict : "待测试"
    property string phaseTitle: viewModel ? viewModel.currentPhase : "等待开始"

    // Computed properties for UI state (replaces manual Connections assignments)
    property bool hasImpactTest: viewModel ? viewModel.impactTestEnabled : false

    property int currentPhaseIndex: {
        const phaseName = phaseTitle
        if (hasImpactTest) {
            if (phaseName.includes("冲击") || phaseName.includes("Impact"))
                return 0
            else if (phaseName.includes("准备") || phaseName.includes("找零") || phaseName.includes("Homing"))
                return 1
            else if (phaseName.includes("空载") || phaseName.includes("Idle"))
                return 2
            else if (phaseName.includes("角度") || phaseName.includes("Angle"))
                return 3
            else if (phaseName.includes("负载") || phaseName.includes("Load"))
                return 4
            else if (phaseName.includes("回零") || phaseName.includes("Return"))
                return 5
        } else {
            if (phaseName.includes("准备") || phaseName.includes("找零") || phaseName.includes("Homing"))
                return 0
            else if (phaseName.includes("空载") || phaseName.includes("Idle"))
                return 1
            else if (phaseName.includes("角度") || phaseName.includes("Angle"))
                return 2
            else if (phaseName.includes("负载") || phaseName.includes("Load"))
                return 3
            else if (phaseName.includes("回零") || phaseName.includes("Return"))
                return 4
        }
        return -1
    }

    property string infoText: {
        if (errorMessage !== "")
            return errorMessage
        if (!viewModel)
            return "警告：ViewModel 未连接，UI 将无法控制测试"
        if (viewModel.testPassed !== undefined && verdictState !== "pending") {
            return viewModel.testPassed ? "测试完成，整机判定 OK" : "测试完成，整机判定 NG"
        }
        if (running)
            return "测试进行中，当前阶段：" + phaseTitle
        return viewModel.statusMessage || "待机中"
    }

    property string infoType: {
        if (errorMessage !== "")
            return "error"
        if (!viewModel)
            return "warning"
        if (verdictState === "ok")
            return "success"
        if (verdictState === "ng")
            return "error"
        if (running)
            return "success"
        return "warning"
    }

    property string errorMessage: ""
    property bool speedChannelOn: true
    property bool torqueChannelOn: true
    property bool currentChannelOn: false
    property bool angleChannelOn: true
    property var phaseDurations: []
    property var chartSpeed: []
    property var chartTorque: []
    property var chartCurrent: []
    property var chartAngle: []
    property var magnetMarkers: []
    property string anomalyMessage: ""
    property string anomalyType: ""

    // Physics validation data (from ViewModel if available, otherwise mock)
    property var physicsViolations: viewModel && viewModel.physicsViolations !== undefined
                                    ? viewModel.physicsViolations
                                    : mockPhysicsViolations
    property var physicsViolationStats: viewModel && viewModel.physicsViolationStats !== undefined
                                        ? viewModel.physicsViolationStats
                                        : mockPhysicsViolationStats
    property var mockPhysicsViolations: []
    property var mockPhysicsViolationStats: ({})

    ListModel { id: stepModel }
    ListModel { id: angleModel }
    ListModel { id: loadModel }
    ListModel { id: idleModel }

    // Mock physics validation generator for UI demonstration
    Timer {
        id: physicsMockTimer
        interval: 2000
        running: root.running
        repeat: true
        onTriggered: root.generateMockPhysicsViolations()
    }

    function generateMockPhysicsViolations() {
        const rules = [
            { name: "功率守恒定律", range: "P = T×ω", prob: 0.15 },
            { name: "扭矩组成一致性", range: "T_total = T_motor + T_brake + 0.3", prob: 0.1 },
            { name: "转速一致性", range: "|speed_torque - speed_encoder| < 5%", prob: 0.08 },
            { name: "制动扭矩单调性", range: "dT_brake/dI_brake > 0", prob: 0.05 },
            { name: "角度积分连续性", range: "|Δθ - ω×Δt| < 1°", prob: 0.12 },
            { name: "电机电流负载关系", range: "I = 0.5 + 1.5×duty + 0.3×brake", prob: 0.1 }
        ]

        const now = new Date()
        const timeStr = now.getHours().toString().padStart(2, '0') + ":" +
                        now.getMinutes().toString().padStart(2, '0') + ":" +
                        now.getSeconds().toString().padStart(2, '0')

        let violations = root.mockPhysicsViolations.slice()
        let newViolation = null

        for (let i = 0; i < rules.length; i++) {
            if (Math.random() < rules[i].prob) {
                const severities = ["warning", "warning", "critical", "info"]
                const sev = severities[Math.floor(Math.random() * severities.length)]
                newViolation = {
                    ruleName: rules[i].name,
                    severity: sev,
                    actualValue: (Math.random() * 100).toFixed(2),
                    expectedRange: rules[i].range,
                    timestamp: timeStr,
                    details: "物理量偏离期望值，建议检查传感器校准"
                }
                violations.unshift(newViolation)
                break
            }
        }

        if (violations.length > 50) {
            violations = violations.slice(0, 50)
        }

        root.mockPhysicsViolations = violations

        let criticalCount = 0, warningCount = 0, infoCount = 0
        for (let i = 0; i < violations.length; i++) {
            if (violations[i].severity === "critical") criticalCount++
            else if (violations[i].severity === "warning") warningCount++
            else if (violations[i].severity === "info") infoCount++
        }

        let trend = root.mockPhysicsViolationStats.trendData || [0,0,0,0,0,0,0,0,0,0]
        trend = trend.slice(1)
        trend.push(newViolation ? 1 : 0)

        root.mockPhysicsViolationStats = {
            totalCount: violations.length,
            criticalCount: criticalCount,
            warningCount: warningCount,
            infoCount: infoCount,
            trendData: trend
        }
    }

    function resetPhysicsViolations() {
        root.mockPhysicsViolations = []
        root.mockPhysicsViolationStats = {
            totalCount: 0,
            criticalCount: 0,
            warningCount: 0,
            infoCount: 0,
            trendData: [0,0,0,0,0,0,0,0,0,0]
        }
    }

    // Connect to ViewModel signals (optimized - reduced from 5 handlers to 2)
    Connections {
        target: root.viewModel
        enabled: root.viewModel !== null

        function onMotorTelemetryChanged() {
            root.chartSpeed = appendPoint(root.chartSpeed, root.speedValue)
            root.chartCurrent = appendPoint(root.chartCurrent, root.motorCurrentValue)
        }

        function onTorqueTelemetryChanged() {
            root.chartTorque = appendPoint(root.chartTorque, root.torqueValue)
            root.chartSpeed = appendPoint(root.chartSpeed, root.speedValue)
        }

        function onEncoderTelemetryChanged() {
            root.chartAngle = appendPoint(root.chartAngle, root.angleValue)
        }

        function onBrakeTelemetryChanged() {
            root.chartCurrent = appendPoint(root.chartCurrent, root.motorCurrentValue)
        }

        function onErrorOccurred(message) {
            root.errorMessage = message
            errorMessageTimer.restart()
        }
    }

    Connections {
        target: root.recipeVM
        enabled: root.recipeVM !== null

        function onRecipesChanged() {
            refreshRecipeOptions(commandBar.serialNumber === "")
        }
    }

    // Auto-clear error message after 5 seconds
    Timer {
        id: errorMessageTimer
        interval: 5000
        repeat: false
        onTriggered: root.errorMessage = ""
    }

    // Property change handlers (replaces Connections logic)
    onCurrentPhaseIndexChanged: applyStepStates()
    onRunningChanged: applyStepStates()
    onVerdictStateChanged: {
        if (verdictState !== "pending") {
            populateResultModels()
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
        if (label === "累计角度")
            return theme.accent
        if (label === "编码器转速")
            return theme.accent
        if (label === "电机电流" && motorCurrentValue > 0)
            return theme.warnColor
        if (label === "制动电流" && brakeCurrentValue > 0)
            return theme.warnColor
        if (label === "制动电压" && brakeVoltageValue > 0)
            return theme.warnColor
        if (label === "制动功率" && brakePowerValue > 0)
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
        if (label === "制动电压")
            return brakeVoltageValue.toFixed(2)
        if (label === "制动功率")
            return brakePowerValue.toFixed(2)
        if (label === "累计角度")
            return encoderTotalAngleValue.toFixed(1)
        if (label === "编码器转速")
            return encoderVelocityValue.toFixed(1)
        return angleValue.toFixed(1)
    }

    function metricUnit(label) {
        if (label === "转速")
            return "RPM"
        if (label === "扭矩")
            return "N·m"
        if (label === "功率")
            return "W"
        if (label === "角度" || label === "累计角度")
            return "°"
        if (label === "编码器转速")
            return "RPM"
        if (label === "制动电压")
            return "V"
        if (label === "制动功率")
            return "W"
        if (label === "电机电流" || label === "制动电流")
            return "A"
        return ""
    }

    function formatDateToken(value, width) {
        return String(value).padStart(width, "0")
    }

    function applySerialRuleTokens(template) {
        if (!template)
            return ""

        const now = new Date()
        let result = String(template)
        result = result.replace(/yyyy/g, String(now.getFullYear()))
        result = result.replace(/yy/g, formatDateToken(now.getFullYear() % 100, 2))
        result = result.replace(/mm/g, formatDateToken(now.getMonth() + 1, 2))
        result = result.replace(/dd/g, formatDateToken(now.getDate(), 2))
        return result
    }

    function generateSerialNumber(rule) {
        const normalizedRule = (rule || "").trim()
        if (!normalizedRule)
            return ""

        const match = normalizedRule.match(/^(.*?)(\d+)$/)
        if (!match)
            return applySerialRuleTokens(normalizedRule)

        const prefix = applySerialRuleTokens(match[1])
        const sequenceTemplate = match[2]
        const width = sequenceTemplate.length
        let nextSequence = parseInt(sequenceTemplate, 10)
        if (Number.isNaN(nextSequence) || nextSequence < 1)
            nextSequence = 1

        let maxSequence = nextSequence - 1
        if (typeof historyService !== "undefined" && historyService) {
            const records = historyService.loadAll()
            for (let i = 0; i < records.length; ++i) {
                const record = records[i]
                const sn = String(record.serialNumber || record.sn || "")
                if (!sn.startsWith(prefix))
                    continue

                const suffix = sn.slice(prefix.length)
                if (/^\d+$/.test(suffix) && suffix.length === width) {
                    maxSequence = Math.max(maxSequence, parseInt(suffix, 10))
                }
            }
        }

        return prefix + String(maxSequence + 1).padStart(width, "0")
    }

    function buildRecipeOptions() {
        if (!recipeVM || !recipeVM.recipes)
            return []

        const recipes = recipeVM.recipes
        const options = []
        for (let i = 0; i < recipes.length; ++i) {
            const recipe = recipes[i]
            const fileName = String(recipe.fileName || "")
            const fileStem = fileName.replace(/\.json$/i, "")
            options.push({
                label: fileStem || String(recipe.name || "未命名配方"),
                value: fileName || fileStem,
                recipeName: String(recipe.name || ""),
                backlashDeg: recipe.gearBacklashCompensationDeg !== undefined ? recipe.gearBacklashCompensationDeg : 0,
                serialNumberRule: String(recipe.serialNumberRule || "yyyymmdd00001")
            })
        }
        return options
    }

    function applySelectedRecipe(index, regenerateSerial) {
        if (index < 0 || index >= root.recipeOptions.length)
            return

        const option = root.recipeOptions[index]
        commandBar.backlashValue = option.backlashDeg !== undefined ? String(option.backlashDeg) : ""
        if (regenerateSerial)
            commandBar.serialNumber = generateSerialNumber(option.serialNumberRule)

        if (viewModel) {
            viewModel.loadRecipe(String(option.value || option.label || ""))
            viewModel.selectedModel = String(option.value || option.label || "")
        }
    }

    function refreshRecipeOptions(regenerateSerial) {
        const currentValue = commandBar.modelValue
        root.recipeOptions = buildRecipeOptions()
        commandBar.modelOptions = root.recipeOptions

        if (root.recipeOptions.length === 0)
            return

        let nextIndex = 0
        for (let i = 0; i < root.recipeOptions.length; ++i) {
            if (root.recipeOptions[i].value === currentValue) {
                nextIndex = i
                break
            }
        }

        commandBar.modelIndex = nextIndex
        applySelectedRecipe(nextIndex, regenerateSerial)
    }

    function initializeModels() {
        stepModel.clear()
        var steps
        if (hasImpactTest) {
            steps = [
                { name: "冲击测试", detail: "空载正反转冲击制动", state: "wait", elapsed: "00:00.0" },
                { name: "准备/找零", detail: "编码器归零并建立补偿", state: "wait", elapsed: "00:00.0" },
                { name: "空载正反转", detail: "采集正反转电流与转速", state: "wait", elapsed: "00:00.0" },
                { name: "角度定位", detail: "执行五个目标位定位判定", state: "wait", elapsed: "00:00.0" },
                { name: "负载上升", detail: "锁止并采集制动扭矩", state: "wait", elapsed: "00:00.0" },
                { name: "回零结束", detail: "回到零位并汇总结果", state: "wait", elapsed: "00:00.0" }
            ]
        } else {
            steps = [
                { name: "准备/找零", detail: "编码器归零并建立补偿", state: "wait", elapsed: "00:00.0" },
                { name: "空载正反转", detail: "采集正反转电流与转速", state: "wait", elapsed: "00:00.0" },
                { name: "角度定位", detail: "执行五个目标位定位判定", state: "wait", elapsed: "00:00.0" },
                { name: "负载上升", detail: "锁止并采集制动扭矩", state: "wait", elapsed: "00:00.0" },
                { name: "回零结束", detail: "回到零位并汇总结果", state: "wait", elapsed: "00:00.0" }
            ]
        }
        for (let i = 0; i < steps.length; ++i)
            stepModel.append(steps[i])

        angleModel.clear()
        var angleRows = [
            ["①", "49.0°", "--", "--", "±3.0°", "待测"],
            ["②", "113.5°", "--", "--", "±3.0°", "待测"],
            ["①", "49.0°", "--", "--", "±3.0°", "待测"],
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

        idleModel.clear()
        idleModel.append({ direction: "正转", currentAvg: "--", currentMax: "--", speedAvg: "--", speedMax: "--", result: "待测" })
        idleModel.append({ direction: "反转", currentAvg: "--", currentMax: "--", speedAvg: "--", speedMax: "--", result: "待测" })
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
        const next = array ? array.slice() : []
        next.push(value)
        if (next.length > 120)
            next.shift()
        return next
    }

    function populateResultModels() {
        if (!viewModel) return

        if (typeof viewModel.idleForwardResult !== 'undefined') {
            var fwd = viewModel.idleForwardResult
            if (idleModel.count > 0) {
                idleModel.setProperty(0, "currentAvg", typeof fwd.currentAvg === 'number' ? fwd.currentAvg.toFixed(2) : "--")
                idleModel.setProperty(0, "currentMax", typeof fwd.currentMax === 'number' ? fwd.currentMax.toFixed(2) : "--")
                idleModel.setProperty(0, "speedAvg", typeof fwd.speedAvg === 'number' ? fwd.speedAvg.toFixed(0) : "--")
                idleModel.setProperty(0, "speedMax", typeof fwd.speedMax === 'number' ? fwd.speedMax.toFixed(0) : "--")
                idleModel.setProperty(0, "result", fwd.overallPassed ? "OK" : "NG")
            }
        }
        if (typeof viewModel.idleReverseResult !== 'undefined') {
            var rev = viewModel.idleReverseResult
            if (idleModel.count > 1) {
                idleModel.setProperty(1, "currentAvg", typeof rev.currentAvg === 'number' ? rev.currentAvg.toFixed(2) : "--")
                idleModel.setProperty(1, "currentMax", typeof rev.currentMax === 'number' ? rev.currentMax.toFixed(2) : "--")
                idleModel.setProperty(1, "speedAvg", typeof rev.speedAvg === 'number' ? rev.speedAvg.toFixed(0) : "--")
                idleModel.setProperty(1, "speedMax", typeof rev.speedMax === 'number' ? rev.speedMax.toFixed(0) : "--")
                idleModel.setProperty(1, "result", rev.overallPassed ? "OK" : "NG")
            }
        }

        if (typeof viewModel.angleResults !== 'undefined') {
            var angles = viewModel.angleResults
            var markers = []
            for (var i = 0; i < Math.min(angles.length, angleModel.count); i++) {
                var a = angles[i]
                angleModel.setProperty(i, "currentAngle", typeof a.measuredAngleDeg === 'number' ? a.measuredAngleDeg.toFixed(2) + "°" : "--")
                angleModel.setProperty(i, "deviation", typeof a.deviationDeg === 'number' ? a.deviationDeg.toFixed(2) + "°" : "--")
                angleModel.setProperty(i, "result", a.passed ? "OK" : "NG")

                if (typeof a.targetAngleDeg === 'number') {
                    markers.push({
                        angle: a.targetAngleDeg.toFixed(1),
                        detected: a.passed
                    })
                }
            }
            magnetMarkers = markers
        }

        if (typeof viewModel.loadForwardResult !== 'undefined') {
            var lf = viewModel.loadForwardResult
            if (loadModel.count > 0) {
                loadModel.setProperty(0, "brakeCurrent", typeof lf.lockCurrentA === 'number' ? lf.lockCurrentA.toFixed(2) + " A" : "--")
                loadModel.setProperty(0, "torque", typeof lf.lockTorqueNm === 'number' ? lf.lockTorqueNm.toFixed(2) + " N·m" : "--")
                loadModel.setProperty(0, "result", lf.overallPassed ? "OK" : "NG")
            }
        }
        if (typeof viewModel.loadReverseResult !== 'undefined') {
            var lr = viewModel.loadReverseResult
            if (loadModel.count > 1) {
                loadModel.setProperty(1, "brakeCurrent", typeof lr.lockCurrentA === 'number' ? lr.lockCurrentA.toFixed(2) + " A" : "--")
                loadModel.setProperty(1, "torque", typeof lr.lockTorqueNm === 'number' ? lr.lockTorqueNm.toFixed(2) + " N·m" : "--")
                loadModel.setProperty(1, "result", lr.overallPassed ? "OK" : "NG")
            }
        }
    }

    function resetRun() {
        // Reset local state (currentPhaseIndex is computed, no need to reset)
        phaseElapsedSeconds = 0
        chartSpeed = []
        chartTorque = []
        chartCurrent = []
        chartAngle = []
        magnetMarkers = []
        anomalyMessage = ""
        anomalyType = ""
        errorMessage = ""
        resetPhysicsViolations()
        initializeModels()

        if (viewModel) {
            viewModel.resetTest()
        }
    }

    function startRun() {
        if (!commandBar.modelValue) {
            errorMessage = "请先选择配方"
            errorMessageTimer.restart()
            return
        }

        if (!commandBar.serialNumber || commandBar.serialNumber.length < 8) {
            errorMessage = "SN 长度至少 8 位"
            errorMessageTimer.restart()
            return
        }

        if (!viewModel) {
            errorMessage = "ViewModel 未连接，无法启动测试"
            errorMessageTimer.restart()
            return
        }

        const backlashDeg = Number(commandBar.backlashValue)
        if (commandBar.backlashValue !== "" && Number.isNaN(backlashDeg)) {
            errorMessage = "回差补偿必须是数字"
            errorMessageTimer.restart()
            return
        }

        resetRun()
        viewModel.loadRecipe(commandBar.modelValue)
        viewModel.selectedModel = commandBar.modelValue
        viewModel.serialNumber = commandBar.serialNumber
        viewModel.backlashCompensationDeg = Number.isNaN(backlashDeg) ? 0 : backlashDeg
        viewModel.startTest()
    }

    function stopRun() {
        if (viewModel) {
            viewModel.stopTest()
        }
        errorMessage = "急停触发，测试已中断"
        errorMessageTimer.restart()
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
        console.log("[StartupTrace] TestExecutionPage completed")
        commandBar.serialNumber = ""
        commandBar.backlashValue = ""
        initializeModels()
        root.resetPhysicsViolations()

        if (recipeVM)
            recipeVM.loadAll()

        if (!viewModel) {
            console.warn("TestExecutionPage: testViewModel not found in context")
        } else {
            console.log("TestExecutionPage: Connected to testViewModel")
        }

        refreshRecipeOptions(true)
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
                modelOptions: root.recipeOptions
                onStartRequested: root.startRun
                onStopRequested: root.stopRun
                onModelChanged: function(index) {
                    root.applySelectedRecipe(index, true)
                }
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
                idleModel: idleModel
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
                motorOnline: root.motorOnlineValue
                torqueOnline: root.torqueOnlineValue
                encoderOnline: root.encoderOnlineValue
                brakeOnline: root.brakeOnlineValue
                verdictState: root.verdictState
                verdictText: root.verdictText
                metricColorProvider: root.metricColor
                metricValueProvider: root.metricValue
                metricUnitProvider: root.metricUnit
                magnetMarkers: root.magnetMarkers
                anomalyMessage: root.anomalyMessage
                anomalyType: root.anomalyType
                physicsViolations: root.physicsViolations
                physicsViolationStats: root.physicsViolationStats
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
