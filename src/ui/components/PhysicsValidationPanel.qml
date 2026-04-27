pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: root

    required property AppTheme theme
    property var violations: []
    property var violationStats: ({})
    property bool running: false
    property bool panelVisible: true

    function severityColor(severity) {
        if (severity === "critical" || severity === "严重")
            return root.theme.danger
        if (severity === "warning" || severity === "警告")
            return root.theme.warn
        if (severity === "info" || severity === "提示")
            return root.theme.textSecondary
        return root.theme.okColor
    }

    function severityBgColor(severity) {
        if (severity === "critical" || severity === "严重")
            return root.theme.dangerWeak
        if (severity === "warning" || severity === "警告")
            return root.theme.warnWeak
        if (severity === "info" || severity === "提示")
            return root.theme.sectionColor
        return root.theme.okWeak
    }

    function severityLabel(sev) {
        if (sev === "critical" || sev === "严重")
            return "严重"
        if (sev === "warning" || sev === "警告")
            return "警告"
        if (sev === "info" || sev === "提示")
            return "提示"
        return "正常"
    }

    // Compatibility helpers for different stats formats (mock vs ViewModel)
    function statTotal() {
        const s = root.violationStats || {}
        return s.totalCount !== undefined ? s.totalCount : (s.totalViolations !== undefined ? s.totalViolations : 0)
    }

    function statCritical() {
        const s = root.violationStats || {}
        return s.criticalCount !== undefined ? s.criticalCount : 0
    }

    function statWarning() {
        const s = root.violationStats || {}
        return s.warningCount !== undefined ? s.warningCount : 0
    }

    function statInfo() {
        const s = root.violationStats || {}
        return s.infoCount !== undefined ? s.infoCount : 0
    }

    function statTrend() {
        const s = root.violationStats || {}
        return s.trendData !== undefined ? s.trendData : []
    }

    // Violation item compatibility helpers
    function vRuleName(item) { return item.ruleName || "未知规则" }
    function vSeverity(item) {
        if (item.severity !== undefined) return item.severity
        if (item.errorPercent !== undefined) {
            return item.errorPercent > 0.20 ? "critical" : "warning"
        }
        return "warning"
    }
    function vActualValue(item) {
        if (item.actualValue !== undefined) return String(item.actualValue)
        return "--"
    }
    function vExpectedRange(item) {
        if (item.expectedRange !== undefined) return String(item.expectedRange)
        if (item.expectedValue !== undefined) return String(item.expectedValue)
        if (item.threshold !== undefined) return "阈值: " + String(item.threshold)
        return "--"
    }
    function vTimestamp(item) {
        if (item.timestamp !== undefined) return String(item.timestamp)
        if (item.lastCheckTime !== undefined) return String(item.lastCheckTime)
        return ""
    }
    function vDetails(item) {
        if (item.details !== undefined) return String(item.details)
        if (item.message !== undefined) return String(item.message)
        return ""
    }

    radius: root.theme.radiusLarge
    color: root.theme.cardColor

    Component.onCompleted: console.log("[StartupTrace] PhysicsValidationPanel completed")
    border.color: root.theme.dividerColor
    border.width: 1
    implicitHeight: root.panelVisible ? contentLayout.implicitHeight + 32 : headerRow.implicitHeight + 24
    clip: true

    Behavior on implicitHeight {
        NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
    }

    ColumnLayout {
        id: contentLayout
        width: parent.width - 28
        x: 14
        y: 14
        spacing: 14

        // Header
        RowLayout {
            id: headerRow
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "物理规律检查"
                color: root.theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }

            Rectangle {
                visible: root.running
                width: 8
                height: 8
                radius: 4
                color: root.theme.okColor

                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                visible: root.violations.length > 0
                radius: 10
                implicitWidth: countText.width + 14
                implicitHeight: 20
                color: root.theme.dangerWeak

                Text {
                    id: countText
                    anchors.centerIn: parent
                    text: String(root.violations.length)
                    color: root.theme.danger
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Text {
                text: root.panelVisible ? "收起" : "展开"
                color: root.theme.textMuted
                font.pixelSize: 11

                TapHandler {
                    onTapped: root.panelVisible = !root.panelVisible
                }
            }
        }

        // Stats summary row
        RowLayout {
            visible: root.panelVisible
            Layout.fillWidth: true
            spacing: 10

            Repeater {
                model: [
                    { label: "总违规", value: root.statTotal(), color: root.theme.textPrimary },
                    { label: "严重", value: root.statCritical(), color: root.theme.danger },
                    { label: "警告", value: root.statWarning(), color: root.theme.warn },
                    { label: "提示", value: root.statInfo(), color: root.theme.textSecondary }
                ]

                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: root.theme.radiusMedium
                    color: root.theme.bgSecondary
                    border.color: root.theme.dividerColor
                    border.width: 1

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            text: modelData.value
                            color: modelData.color
                            font.pixelSize: 18
                            font.bold: true
                            font.family: "Consolas"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            text: modelData.label
                            color: root.theme.textMuted
                            font.pixelSize: 10
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }

        // Trend & Distribution Charts
        RowLayout {
            visible: root.panelVisible
            Layout.fillWidth: true
            spacing: 10

            // Violation trend chart
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                radius: root.theme.radiusMedium
                color: root.theme.bgSecondary
                border.color: root.theme.dividerColor
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    Text {
                        text: "违规次数趋势"
                        color: root.theme.textSecondary
                        font.pixelSize: 10
                        font.bold: true
                    }

                    Canvas {
                        id: trendCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            const ctx = getContext("2d")
                            const w = width
                            const h = height
                            ctx.clearRect(0, 0, w, h)

                            const trend = root.statTrend()
                            if (trend.length < 2) {
                                ctx.fillStyle = root.theme.textMuted
                                ctx.font = "11px sans-serif"
                                ctx.textAlign = "center"
                                ctx.fillText("暂无数据", w / 2, h / 2)
                                return
                            }

                            const maxVal = Math.max(...trend, 1)
                            const padLeft = 4
                            const padBottom = 4
                            const chartW = w - padLeft - 4
                            const chartH = h - padBottom - 4

                            // Grid
                            ctx.strokeStyle = root.theme.darkMode ? "#333" : "#E8E8E8"
                            ctx.lineWidth = 1
                            ctx.beginPath()
                            for (let i = 0; i <= 3; i++) {
                                const y = padBottom + chartH * i / 3
                                ctx.moveTo(padLeft, y)
                                ctx.lineTo(w - 4, y)
                            }
                            ctx.stroke()

                            // Bar chart
                            const barW = chartW / trend.length * 0.7
                            const step = chartW / trend.length
                            for (let i = 0; i < trend.length; i++) {
                                const val = trend[i]
                                const barH = (val / maxVal) * chartH
                                const x = padLeft + i * step + (step - barW) / 2
                                const y = padBottom + chartH - barH

                                if (val > 0) {
                                    ctx.fillStyle = root.theme.warn
                                    ctx.fillRect(x, y, barW, barH)
                                }
                            }
                        }

                        Connections {
                            target: root
                            function onViolationStatsChanged() { trendCanvas.requestPaint() }
                        }
                    }
                }
            }

            // Severity distribution
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                radius: root.theme.radiusMedium
                color: root.theme.bgSecondary
                border.color: root.theme.dividerColor
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    Text {
                        text: "严重程度分布"
                        color: root.theme.textSecondary
                        font.pixelSize: 10
                        font.bold: true
                    }

                    Canvas {
                        id: distCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            const ctx = getContext("2d")
                            const w = width
                            const h = height
                            ctx.clearRect(0, 0, w, h)

                            const total = root.violations.length
                            if (total === 0) {
                                ctx.fillStyle = root.theme.textMuted
                                ctx.font = "11px sans-serif"
                                ctx.textAlign = "center"
                                ctx.fillText("暂无违规", w / 2, h / 2)
                                return
                            }

                            const critical = root.statCritical()
                            const warning = root.statWarning()
                            const info = root.statInfo()

                            const cx = w / 2
                            const cy = h / 2 + 2
                            const radius = Math.min(w, h) / 2 - 14
                            let startAngle = -Math.PI / 2

                            const slices = [
                                { value: critical, color: root.theme.danger, label: "严重" },
                                { value: warning, color: root.theme.warn, label: "警告" },
                                { value: info, color: root.theme.textSecondary, label: "提示" }
                            ]

                            slices.forEach(slice => {
                                if (slice.value > 0) {
                                    const angle = (slice.value / total) * 2 * Math.PI
                                    ctx.beginPath()
                                    ctx.moveTo(cx, cy)
                                    ctx.arc(cx, cy, radius, startAngle, startAngle + angle)
                                    ctx.closePath()
                                    ctx.fillStyle = slice.color
                                    ctx.fill()
                                    startAngle += angle
                                }
                            })

                            // Center hole for donut style
                            ctx.beginPath()
                            ctx.arc(cx, cy, radius * 0.55, 0, 2 * Math.PI)
                            ctx.fillStyle = root.theme.bgSecondary
                            ctx.fill()

                            // Center text
                            ctx.fillStyle = root.theme.textPrimary
                            ctx.font = "bold 12px sans-serif"
                            ctx.textAlign = "center"
                            ctx.textBaseline = "middle"
                            ctx.fillText(String(total), cx, cy - 1)
                        }

                        Connections {
                            target: root
                            function onViolationsChanged() { distCanvas.requestPaint() }
                        }
                    }
                }
            }
        }

        // Violation list
        ColumnLayout {
            visible: root.panelVisible
            Layout.fillWidth: true
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: "违规记录"
                    color: root.theme.textSecondary
                    font.pixelSize: 10
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Text {
                    visible: root.violations.length === 0
                    text: "暂无违规记录"
                    color: root.theme.okColor
                    font.pixelSize: 11
                }
            }

            Rectangle {
                visible: root.violations.length > 0
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(violationList.contentHeight + 4, 220)
                color: root.theme.bgSecondary
                radius: root.theme.radiusMedium
                border.color: root.theme.dividerColor
                border.width: 1
                clip: true

                ListView {
                    id: violationList
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 2
                    clip: true
                    model: root.violations
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index

                        width: violationList.width - 8
                        x: 4
                        height: violationRow.implicitHeight + 12
                        radius: root.theme.radiusSmall
                        color: root.severityBgColor(root.vSeverity(modelData))
                        border.color: root.theme.dividerColor
                        border.width: 0.5

                        ColumnLayout {
                            id: violationRow
                            width: parent.width - 16
                            x: 8
                            y: 6
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Rectangle {
                                    Layout.preferredWidth: 40
                                    Layout.preferredHeight: 18
                                    radius: 4
                                    color: root.severityColor(root.vSeverity(modelData))

                                    Text {
                                        anchors.centerIn: parent
                                        text: root.severityLabel(root.vSeverity(modelData))
                                        color: "white"
                                        font.pixelSize: 9
                                        font.bold: true
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: root.vRuleName(modelData)
                                    color: root.theme.textPrimary
                                    font.pixelSize: 11
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: root.vTimestamp(modelData)
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                    font.family: "Consolas"
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    text: "当前值:"
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                }

                                Text {
                                    text: root.vActualValue(modelData)
                                    color: root.theme.textPrimary
                                    font.pixelSize: 10
                                    font.family: "Consolas"
                                }

                                Text {
                                    text: "期望范围:"
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                }

                                Text {
                                    text: root.vExpectedRange(modelData)
                                    color: root.theme.textPrimary
                                    font.pixelSize: 10
                                    font.family: "Consolas"
                                }
                            }

                            Text {
                                visible: root.vDetails(modelData).length > 0
                                Layout.fillWidth: true
                                text: root.vDetails(modelData)
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }
        }
    }
}
