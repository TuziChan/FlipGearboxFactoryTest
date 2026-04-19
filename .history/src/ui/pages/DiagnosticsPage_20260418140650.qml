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

    readonly property int pageWidth: root.width
    readonly property string layoutMode: pageWidth >= 1200 ? "wide"
                                         : pageWidth >= 840 ? "medium"
                                         : "compact"
    readonly property int contentGutter: layoutMode === "compact" ? 12 : 16
    readonly property int metricsColumns: layoutMode === "wide" ? 3
                                          : layoutMode === "medium" ? 2
                                          : 1
    readonly property bool hasSidePanel: layoutMode === "wide"

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
            anchors.leftMargin: root.contentGutter
            anchors.rightMargin: root.contentGutter
            anchors.topMargin: root.contentGutter
            anchors.bottomMargin: root.contentGutter
            spacing: root.contentGutter

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

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.contentGutter

                Components.AppTabs {
                    id: mainTabs
                    Layout.fillWidth: true
                    theme: root.theme
                    fillWidth: true
                    overflowBehavior: root.layoutMode === "compact" ? "scroll" : "clip"
                    model: [
                        {text: "设备总览"},
                        {text: "AQMD 电机驱动器"},
                        {text: "DYN200 扭矩传感器"},
                        {text: "单圈绝对值编码器"},
                        {text: "制动电源"}
                    ]
                    showContent: false
                }

                Loader {
                    id: tabContentLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: {
                        switch (mainTabs.currentIndex) {
                            case 0: return overviewTab
                            case 1: return motorTab
                            case 2: return torqueTab
                            case 3: return encoderTab
                            case 4: return brakeTab
                            default: return null
                        }
                    }
                    onLoaded: {
                        if (!item) return
                        item.anchors.fill = tabContentLoader
                        if (item.layoutMode !== undefined) item.layoutMode = root.layoutMode
                        if (item.metricsColumns !== undefined) item.metricsColumns = root.metricsColumns
                        if (item.hasSidePanel !== undefined) item.hasSidePanel = root.hasSidePanel
                        if (item.contentGutter !== undefined) item.contentGutter = root.contentGutter
                    }
                    onItemChanged: {
                        if (!item) return
                        if (item.layoutMode !== undefined) item.layoutMode = Qt.binding(function() { return root.layoutMode })
                        if (item.metricsColumns !== undefined) item.metricsColumns = Qt.binding(function() { return root.metricsColumns })
                        if (item.hasSidePanel !== undefined) item.hasSidePanel = Qt.binding(function() { return root.hasSidePanel })
                        if (item.contentGutter !== undefined) item.contentGutter = Qt.binding(function() { return root.contentGutter })
                    }
                }
            }

            Components.AppCollapsible {
                Layout.fillWidth: true
                title: "通信日志"
                open: false
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Item { Layout.fillWidth: true }

                        Components.AppButton {
                            text: "清空"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: if (root.viewModel) root.viewModel.clearLog()
                        }
                    }

                    QQC.ScrollView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(300, root.height * 0.35)
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

    Component {
        id: overviewTab
        DiagnosticsOverviewTab {
            theme: root.theme
            viewModel: root.viewModel
        }
    }

    Component {
        id: motorTab
        DiagnosticsMotorTab {
            theme: root.theme
            viewModel: root.viewModel
            onMotorForwardRequested: {
                root.pendingMotorAction = "forward"
                root.motorConfirmVisible = true
            }
            onMotorReverseRequested: {
                root.pendingMotorAction = "reverse"
                root.motorConfirmVisible = true
            }
            onMotorStopRequested: {
                if (root.viewModel) root.viewModel.stopMotor()
            }
        }
    }

    Component {
        id: torqueTab
        DiagnosticsTorqueTab {
            theme: root.theme
            viewModel: root.viewModel
        }
    }

    Component {
        id: encoderTab
        DiagnosticsEncoderTab {
            theme: root.theme
            viewModel: root.viewModel
            onEncoderZeroRequested: {
                if (root.viewModel) root.viewModel.setEncoderZero()
            }
        }
    }

    Component {
        id: brakeTab
        DiagnosticsBrakeTab {
            theme: root.theme
            viewModel: root.viewModel
            onBrakeEnableRequested: {
                if (root.viewModel) root.viewModel.setBrakeOutput(true)
            }
            onBrakeDisableRequested: {
                if (root.viewModel) root.viewModel.setBrakeOutput(false)
            }
            onBrakeCurrentSetRequested: function(currentA) {
                if (root.viewModel) root.viewModel.setBrakeCurrent(currentA)
            }
            onBrakeVoltageSetRequested: function(voltageV) {
                if (root.viewModel) root.viewModel.setBrakeVoltage(voltageV)
            }
        }
    }

    Components.AppAlertDialog {
        id: motorConfirmDialog
        theme: root.theme
        open: root.motorConfirmVisible
        title: "确认操作"
        description: root.pendingMotorAction === "forward" ? "确定要启动电机正转吗？" : "确定要启动电机反转吗？"
        confirmText: "确认"
        cancelText: "取消"
        onConfirmed: {
            root.motorConfirmVisible = false
            if (!root.viewModel) return
            if (root.pendingMotorAction === "forward") root.viewModel.setMotorForward()
            else if (root.pendingMotorAction === "reverse") root.viewModel.setMotorReverse()
            root.pendingMotorAction = ""
        }
        onCancelled: {
            root.motorConfirmVisible = false
            root.pendingMotorAction = ""
        }
    }

    Component.onCompleted: {
        if (root.viewModel)
            root.viewModel.refresh()
    }
}
