pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import "../components" as Components

Item {
    id: root
    objectName: "historyPage"

    required property Components.AppTheme theme

    property string selectedRecordId: ""
    property string searchText: ""
    property string filterVerdict: "all"
    property string filterDateRange: "all"

    ListModel {
        id: historyModel

        Component.onCompleted: {
            append({
                id: "REC-20260417-001",
                serialNumber: "GBX42A-20260417-001",
                recipeName: "GBX-42A 标准配方",
                verdict: "PASS",
                startTime: "2026-04-17 14:25:30",
                endTime: "2026-04-17 14:27:15",
                duration: "1:45",
                operator: "张三",
                idleForwardCurrent: "0.65",
                idleReverseCurrent: "0.68",
                loadForwardTorque: "1.35",
                loadReverseTorque: "1.42"
            })

            append({
                id: "REC-20260417-002",
                serialNumber: "GBX42A-20260417-002",
                recipeName: "GBX-42A 标准配方",
                verdict: "FAIL",
                startTime: "2026-04-17 14:30:10",
                endTime: "2026-04-17 14:31:45",
                duration: "1:35",
                operator: "张三",
                idleForwardCurrent: "0.92",
                idleReverseCurrent: "0.88",
                loadForwardTorque: "1.05",
                loadReverseTorque: "1.12",
                failureReason: "负载扭矩不足"
            })

            append({
                id: "REC-20260417-003",
                serialNumber: "GBX42A-20260417-003",
                recipeName: "GBX-42A 标准配方",
                verdict: "PASS",
                startTime: "2026-04-17 14:35:20",
                endTime: "2026-04-17 14:37:05",
                duration: "1:45",
                operator: "张三",
                idleForwardCurrent: "0.58",
                idleReverseCurrent: "0.62",
                loadForwardTorque: "1.28",
                loadReverseTorque: "1.38"
            })

            append({
                id: "REC-20260417-004",
                serialNumber: "GBX42B-20260417-001",
                recipeName: "GBX-42B 高精度配方",
                verdict: "PASS",
                startTime: "2026-04-17 15:10:15",
                endTime: "2026-04-17 15:12:20",
                duration: "2:05",
                operator: "李四",
                idleForwardCurrent: "0.52",
                idleReverseCurrent: "0.55",
                loadForwardTorque: "1.62",
                loadReverseTorque: "1.68"
            })

            append({
                id: "REC-20260416-015",
                serialNumber: "GBX42A-20260416-015",
                recipeName: "GBX-42A 标准配方",
                verdict: "FAIL",
                startTime: "2026-04-16 16:45:30",
                endTime: "2026-04-16 16:46:50",
                duration: "1:20",
                operator: "王五",
                idleForwardCurrent: "0.75",
                idleReverseCurrent: "0.78",
                loadForwardTorque: "1.15",
                loadReverseTorque: "0.98",
                failureReason: "角度定位超差"
            })
        }
    }

    function selectRecord(recordId) {
        selectedRecordId = recordId
    }

    function exportRecord() {
        console.log("Export record:", selectedRecordId)
    }

    function exportAll() {
        console.log("Export all records")
    }

    function deleteRecord() {
        if (!selectedRecordId) return

        for (let i = 0; i < historyModel.count; i++) {
            if (historyModel.get(i).id === selectedRecordId) {
                historyModel.remove(i)
                selectedRecordId = ""
                break
            }
        }
    }

    function getFilteredModel() {
        // TODO: 实现实际的过滤逻辑
        return historyModel
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                Components.AppCard {
                    Layout.fillWidth: true
                    theme: root.theme

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Components.AppLabel {
                                text: "历史记录"
                                fontSize: 16
                                fontWeight: 600
                                theme: root.theme
                            }

                            Item { Layout.fillWidth: true }

                            Components.AppButton {
                                text: "导出全部"
                                variant: "outline"
                                size: "sm"
                                theme: root.theme
                                onClicked: root.exportAll()
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Components.AppInput {
                                Layout.fillWidth: true
                                placeholderText: "搜索 SN 或配方名称..."
                                text: root.searchText
                                theme: root.theme
                                onTextChanged: root.searchText = text
                            }

                            Components.AppSelect {
                                Layout.preferredWidth: 120
                                currentValue: root.filterVerdict
                                model: [
                                    {value: "all", label: "全部"},
                                    {value: "PASS", label: "合格"},
                                    {value: "FAIL", label: "不合格"}
                                ]
                                theme: root.theme
                                onCurrentValueChanged: root.filterVerdict = currentValue
                            }

                            Components.AppSelect {
                                Layout.preferredWidth: 120
                                currentValue: root.filterDateRange
                                model: [
                                    {value: "all", label: "全部时间"},
                                    {value: "today", label: "今天"},
                                    {value: "week", label: "本周"},
                                    {value: "month", label: "本月"}
                                ]
                                theme: root.theme
                                onCurrentValueChanged: root.filterDateRange = currentValue
                            }
                        }
                    }
                }

                Components.AppCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    theme: root.theme

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        Components.AppLabel {
                            text: "测试记录列表 (" + historyModel.count + " 条)"
                            fontSize: 14
                            fontWeight: 500
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

                            ListView {
                                id: historyListView
                                model: historyModel
                                spacing: 8

                                delegate: Rectangle {
                                    required property string id
                                    required property string serialNumber
                                    required property string recipeName
                                    required property string verdict
                                    required property string startTime
                                    required property string duration
                                    required property string operator

                                    width: ListView.view.width
                                    height: 90
                                    radius: 6
                                    color: root.selectedRecordId === id ? root.theme.accent + "20" : root.theme.bgSecondary
                                    border.width: root.selectedRecordId === id ? 1 : 0
                                    border.color: root.theme.accent

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.selectRecord(id)
                                    }

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 12

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 4

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 8

                                                Components.AppLabel {
                                                    text: serialNumber
                                                    fontSize: 14
                                                    fontWeight: 600
                                                    theme: root.theme
                                                }

                                                Components.AppBadge {
                                                    text: verdict
                                                    variant: verdict === "PASS" ? "success" : "destructive"
                                                    theme: root.theme
                                                }
                                            }

                                            Components.AppLabel {
                                                text: recipeName
                                                fontSize: 12
                                                color: root.theme.textSecondary
                                                theme: root.theme
                                            }

                                            Item { Layout.fillHeight: true }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 16

                                                Components.AppLabel {
                                                    text: "时间: " + startTime
                                                    fontSize: 11
                                                    color: root.theme.textMuted
                                                    theme: root.theme
                                                }

                                                Components.AppLabel {
                                                    text: "时长: " + duration
                                                    fontSize: 11
                                                    color: root.theme.textMuted
                                                    theme: root.theme
                                                }

                                                Components.AppLabel {
                                                    text: "操作员: " + operator
                                                    fontSize: 11
                                                    color: root.theme.textMuted
                                                    theme: root.theme
                                                }
                                            }
                                        }

                                        Components.AppButton {
                                            text: "查看"
                                            variant: "ghost"
                                            size: "sm"
                                            theme: root.theme
                                            onClicked: root.selectRecord(id)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Components.AppCard {
                Layout.preferredWidth: 480
                Layout.fillHeight: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Components.AppLabel {
                            text: "测试详情"
                            fontSize: 16
                            fontWeight: 600
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppButton {
                            visible: root.selectedRecordId
                            text: "导出"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.exportRecord()
                        }

                        Components.AppButton {
                            visible: root.selectedRecordId
                            text: "删除"
                            variant: "destructive"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.deleteRecord()
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

                        ColumnLayout {
                            width: parent.width - 20
                            spacing: 16

                            Item {
                                visible: !root.selectedRecordId
                                Layout.fillWidth: true
                                Layout.preferredHeight: 200

                                Components.AppLabel {
                                    anchors.centerIn: parent
                                    text: "请从左侧选择记录"
                                    fontSize: 14
                                    color: root.theme.textMuted
                                    theme: root.theme
                                }
                            }

                            ColumnLayout {
                                visible: root.selectedRecordId
                                Layout.fillWidth: true
                                spacing: 16

                                Components.AppLabel {
                                    text: "基本信息"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 12
                                    rowSpacing: 8

                                    Components.AppLabel {
                                        text: "记录ID:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.selectedRecordId
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "产品SN:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "GBX42A-20260417-001"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "配方:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "GBX-42A 标准配方"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "判定:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppBadge {
                                        text: "PASS"
                                        variant: "success"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "操作员:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "张三"
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                Components.AppLabel {
                                    text: "测试时间"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 12
                                    rowSpacing: 8

                                    Components.AppLabel {
                                        text: "开始时间:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "2026-04-17 14:25:30"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "结束时间:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "2026-04-17 14:27:15"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "测试时长:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "1:45"
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                Components.AppLabel {
                                    text: "空载测试结果"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 12
                                    rowSpacing: 8

                                    Components.AppLabel {
                                        text: "正转电流:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "0.65 A"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转电流:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "0.68 A"
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                Components.AppLabel {
                                    text: "负载测试结果"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 12
                                    rowSpacing: 8

                                    Components.AppLabel {
                                        text: "正转扭矩:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "1.35 N·m"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转扭矩:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: "1.42 N·m"
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
