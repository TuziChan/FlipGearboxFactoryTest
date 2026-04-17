pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import "../components" as Components

Item {
    id: root
    objectName: "recipePage"

    required property Components.AppTheme theme

    property string selectedRecipeId: ""
    property bool isEditing: false

    ListModel {
        id: recipeListModel

        Component.onCompleted: {
            // 默认配方数据
            append({
                id: "GBX-42A",
                name: "GBX-42A 标准配方",
                description: "42系列齿轮箱标准测试配方",
                homeDutyCycle: 30.0,
                idleDutyCycle: 40.0,
                idleSpeedMin: 80.0,
                idleSpeedMax: 120.0,
                idleCurrentMax: 0.8,
                loadDutyCycle: 50.0,
                loadTorqueMin: 1.20,
                brakeRampStep: 0.05,
                brakeRampInterval: 100,
                lockSpeedThreshold: 5.0,
                lockDuration: 500,
                anglePositions: "3.0, 49.0, 3.0, 113.5, 0.0",
                angleTolerance: 3.0,
                createdAt: "2026-04-15 10:30:00",
                modifiedAt: "2026-04-17 14:20:00"
            })

            append({
                id: "GBX-42B",
                name: "GBX-42B 高精度配方",
                description: "42系列高精度齿轮箱测试配方",
                homeDutyCycle: 25.0,
                idleDutyCycle: 35.0,
                idleSpeedMin: 85.0,
                idleSpeedMax: 115.0,
                idleCurrentMax: 0.7,
                loadDutyCycle: 45.0,
                loadTorqueMin: 1.50,
                brakeRampStep: 0.03,
                brakeRampInterval: 80,
                lockSpeedThreshold: 3.0,
                lockDuration: 600,
                anglePositions: "3.0, 49.0, 3.0, 113.5, 0.0",
                angleTolerance: 2.0,
                createdAt: "2026-04-10 09:15:00",
                modifiedAt: "2026-04-16 16:45:00"
            })

            append({
                id: "GBX-56A",
                name: "GBX-56A 标准配方",
                description: "56系列齿轮箱标准测试配方",
                homeDutyCycle: 35.0,
                idleDutyCycle: 45.0,
                idleSpeedMin: 70.0,
                idleSpeedMax: 110.0,
                idleCurrentMax: 1.0,
                loadDutyCycle: 55.0,
                loadTorqueMin: 1.80,
                brakeRampStep: 0.06,
                brakeRampInterval: 120,
                lockSpeedThreshold: 5.0,
                lockDuration: 500,
                anglePositions: "5.0, 60.0, 5.0, 120.0, 0.0",
                angleTolerance: 3.5,
                createdAt: "2026-04-12 11:00:00",
                modifiedAt: "2026-04-17 10:30:00"
            })
        }
    }

    function selectRecipe(recipeId) {
        selectedRecipeId = recipeId
        isEditing = false
    }

    function createNewRecipe() {
        selectedRecipeId = ""
        isEditing = true
    }

    function editRecipe() {
        if (selectedRecipeId) {
            isEditing = true
        }
    }

    function saveRecipe() {
        // TODO: 实际保存逻辑
        isEditing = false
    }

    function deleteRecipe() {
        if (!selectedRecipeId) return

        for (let i = 0; i < recipeListModel.count; i++) {
            if (recipeListModel.get(i).id === selectedRecipeId) {
                recipeListModel.remove(i)
                selectedRecipeId = ""
                break
            }
        }
    }

    function exportRecipe() {
        // TODO: 导出配方到文件
        console.log("Export recipe:", selectedRecipeId)
    }

    function importRecipe() {
        // TODO: 从文件导入配方
        console.log("Import recipe")
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            // 左侧：配方列表
            Components.AppCard {
                Layout.fillHeight: true
                Layout.preferredWidth: 360
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    // 标题栏
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Components.AppLabel {
                            text: "配方列表"
                            fontSize: 16
                            fontWeight: 600
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppButton {
                            text: "新建"
                            variant: "default"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.createNewRecipe()
                        }

                        Components.AppButton {
                            text: "导入"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.importRecipe()
                        }
                    }

                    Components.AppSeparator {
                        Layout.fillWidth: true
                        theme: root.theme
                    }

                    // 配方列表
                    QQC.ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        ListView {
                            id: recipeListView
                            model: recipeListModel
                            spacing: 8

                            delegate: Rectangle {
                                required property string id
                                required property string name
                                required property string description
                                required property string modifiedAt

                                width: ListView.view.width
                                height: 80
                                radius: 6
                                color: root.selectedRecipeId === id ? root.theme.accent + "20" : "transparent"
                                border.width: root.selectedRecipeId === id ? 1 : 0
                                border.color: root.theme.accent

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.selectRecipe(id)
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 4

                                    Components.AppLabel {
                                        text: name
                                        fontSize: 14
                                        fontWeight: 500
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: description
                                        fontSize: 12
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }

                                    Item { Layout.fillHeight: true }

                                    Components.AppLabel {
                                        text: "修改时间: " + modifiedAt
                                        fontSize: 11
                                        color: root.theme.textMuted
                                        theme: root.theme
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 右侧：配方详情/编辑
            Components.AppCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                theme: root.theme

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    // 标题栏
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Components.AppLabel {
                            text: root.isEditing ? (root.selectedRecipeId ? "编辑配方" : "新建配方") : "配方详情"
                            fontSize: 16
                            fontWeight: 600
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeId
                            text: "编辑"
                            variant: "default"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.editRecipe()
                        }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeId
                            text: "导出"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.exportRecipe()
                        }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeId
                            text: "删除"
                            variant: "destructive"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.deleteRecipe()
                        }

                        Components.AppButton {
                            visible: root.isEditing
                            text: "保存"
                            variant: "default"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.saveRecipe()
                        }

                        Components.AppButton {
                            visible: root.isEditing
                            text: "取消"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: { root.isEditing = false }
                        }
                    }

                    Components.AppSeparator {
                        Layout.fillWidth: true
                        theme: root.theme
                    }

                    // 内容区域
                    QQC.ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 20

                            // 空状态提示
                            Item {
                                visible: !root.selectedRecipeId && !root.isEditing
                                Layout.fillWidth: true
                                Layout.preferredHeight: 200

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 12

                                    Components.AppLabel {
                                        text: "请从左侧选择配方"
                                        fontSize: 14
                                        color: root.theme.textMuted
                                        theme: root.theme
                                        Layout.alignment: Qt.AlignHCenter
                                    }

                                    Components.AppButton {
                                        text: "新建配方"
                                        variant: "default"
                                        theme: root.theme
                                        Layout.alignment: Qt.AlignHCenter
                                        onClicked: root.createNewRecipe()
                                    }
                                }
                            }

                            // 配方参数表单
                            ColumnLayout {
                                visible: root.selectedRecipeId || root.isEditing
                                Layout.fillWidth: true
                                spacing: 16

                                // 基本信息
                                Components.AppLabel {
                                    text: "基本信息"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 16
                                    rowSpacing: 12

                                    Components.AppLabel {
                                        text: "配方ID:"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        placeholderText: "GBX-XXX"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "配方名称:"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        placeholderText: "输入配方名称"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "描述:"
                                        theme: root.theme
                                        Layout.alignment: Qt.AlignTop
                                    }
                                    Components.AppTextarea {
                                        Layout.fillWidth: true
                                        placeholderText: "输入配方描述"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                // 归零参数
                                Components.AppLabel {
                                    text: "归零参数"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 16
                                    rowSpacing: 12

                                    Components.AppLabel {
                                        text: "归零占空比 (%):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "30.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                // 空载参数
                                Components.AppLabel {
                                    text: "空载测试参数"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 16
                                    rowSpacing: 12

                                    Components.AppLabel {
                                        text: "空载占空比 (%):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "40.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "转速下限 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "80.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "转速上限 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "120.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "电流上限 (A):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "0.8"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                // 角度定位参数
                                Components.AppLabel {
                                    text: "角度定位参数"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 16
                                    rowSpacing: 12

                                    Components.AppLabel {
                                        text: "目标角度序列 (°):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "3.0, 49.0, 3.0, 113.5, 0.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "角度容差 (°):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "3.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

                                // 负载测试参数
                                Components.AppLabel {
                                    text: "负载测试参数"
                                    fontSize: 14
                                    fontWeight: 600
                                    theme: root.theme
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 16
                                    rowSpacing: 12

                                    Components.AppLabel {
                                        text: "负载占空比 (%):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "50.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "扭矩下限 (N·m):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "1.20"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "制动电流步进 (A):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "0.05"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "步进间隔 (ms):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "100"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "锁止转速阈值 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "5.0"
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "锁止确认时长 (ms):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: "500"
                                        readOnly: !root.isEditing
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
