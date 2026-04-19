pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import Qt.labs.qmlmodels
import "../components" as Components

Item {
    id: root
    objectName: "historyPage"

    required property Components.AppTheme theme

    property int selectedRecordIndex: -1
    property string selectedRecordId: ""
    property string searchText: ""
    property string filterVerdict: "all"
    property string filterDateRange: "all"

    function currentRecord() {
        if (selectedRecordIndex >= 0 && selectedRecordIndex < historyModel.count)
            return historyModel.get(selectedRecordIndex)
        return null
    }

    ListModel {
        id: historyModel

        Component.onCompleted: {
            if (typeof historyService !== 'undefined' && historyService) {
                var records = historyService.loadAll()
                for (var i = 0; i < records.length; i++) {
                    var r = records[i]
                    append({
                        id: r.id || "",
                        serialNumber: r.serialNumber || "",
                        recipeName: r.recipeName || "",
                        verdict: r.verdict || "",
                        startTime: r.startTime || "",
                        endTime: r.endTime || "",
                        duration: r.duration || "",
                        operator: r.operator_ || r.operator || "",
                        idleForwardCurrent: r.idleForwardCurrentAvg || "",
                        idleForwardCurrentMax: r.idleForwardCurrentMax || "",
                        idleForwardSpeedAvg: r.idleForwardSpeedAvg || "",
                        idleForwardSpeedMax: r.idleForwardSpeedMax || "",
                        idleReverseCurrent: r.idleReverseCurrentAvg || "",
                        idleReverseCurrentMax: r.idleReverseCurrentMax || "",
                        idleReverseSpeedAvg: r.idleReverseSpeedAvg || "",
                        idleReverseSpeedMax: r.idleReverseSpeedMax || "",
                        loadForwardCurrent: r.loadForwardCurrent || "",
                        loadForwardTorque: r.loadForwardTorque || "",
                        loadReverseCurrent: r.loadReverseCurrent || "",
                        loadReverseTorque: r.loadReverseTorque || "",
                        failureReason: r.failureReason || ""
                    })
                }
            }
        }
    }

    function selectRecord(recordId, recordIndex) {
        root.selectedRecordId = recordId
        root.selectedRecordIndex = recordIndex
    }

    function exportRecord() {
        var r = currentRecord()
        if (r && typeof historyService !== 'undefined' && historyService) {
            historyService.exportRecord(r.id, "exports/" + r.id + ".json")
        }
    }

    function exportAll() {
        if (typeof historyService !== 'undefined' && historyService) {
            historyService.exportAll("exports/history_export.json")
        }
    }

    function deleteRecord() {
        if (selectedRecordIndex < 0) return
        if (root.selectedRecordId && typeof historyService !== 'undefined' && historyService) {
            historyService.deleteRecord(root.selectedRecordId)
        }
        historyModel.remove(selectedRecordIndex)
        selectedRecordIndex = -1
        selectedRecordId = ""
    }

    SortFilterProxyModel {
        id: filteredModel
        model: historyModel
        filters: [
            FunctionFilter {
                function filter(data): bool {
                    var matchVerdict = root.filterVerdict === "all" || data.verdict === root.filterVerdict
                    var matchSearch = root.searchText === "" ||
                        (data.serialNumber !== undefined && data.serialNumber.indexOf(root.searchText) >= 0) ||
                        (data.recipeName !== undefined && data.recipeName.indexOf(root.searchText) >= 0)
                    return matchVerdict && matchSearch
                }
            }
        ]
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
                                model: filteredModel
                                spacing: 8

                                delegate: Rectangle {
                                    required property int index
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
                                    color: root.selectedRecordIndex === index ? root.theme.accent + "20" : root.theme.bgSecondary
                                    border.width: root.selectedRecordIndex === index ? 1 : 0
                                    border.color: root.theme.accent

                                    TapHandler {
                                        onTapped: root.selectRecord(id, index)
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
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
                                            onClicked: root.selectRecord(id, index)
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
                                        text: root.currentRecord() ? root.currentRecord().serialNumber : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "配方:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().recipeName : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "判定:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppBadge {
                                        text: root.currentRecord() ? root.currentRecord().verdict : ""
                                        variant: root.currentRecord() && root.currentRecord().verdict === "PASS" ? "success" : "destructive"
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "操作员:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().operator : ""
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
                                        text: root.currentRecord() ? root.currentRecord().startTime : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "结束时间:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().endTime : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "测试时长:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().duration : ""
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
                                        text: "正转电流平均:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleForwardCurrent : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "正转电流峰值:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleForwardCurrentMax : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "正转转速平均:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleForwardSpeedAvg : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "正转转速峰值:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleForwardSpeedMax : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转电流平均:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleReverseCurrent : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转电流峰值:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleReverseCurrentMax : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转转速平均:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleReverseSpeedAvg : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转转速峰值:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().idleReverseSpeedMax : ""
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
                                        text: "正转制动电流:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().loadForwardCurrent : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "正转扭矩:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().loadForwardTorque : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转制动电流:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().loadReverseCurrent : ""
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "反转扭矩:"
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }
                                    Components.AppLabel {
                                        text: root.currentRecord() ? root.currentRecord().loadReverseTorque : ""
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                    visible: root.currentRecord() && root.currentRecord().failureReason.length > 0
                                }

                                Components.AppLabel {
                                    text: "失败原因"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                    visible: root.currentRecord() && root.currentRecord().failureReason.length > 0
                                }

                                Components.AppAlert {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                    variant: "destructive"
                                    description: root.currentRecord() ? root.currentRecord().failureReason : ""
                                    visible: root.currentRecord() && root.currentRecord().failureReason.length > 0
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
