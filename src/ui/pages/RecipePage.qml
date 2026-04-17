pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import "../components" as Components

Item {
    id: root
    objectName: "recipePage"

    required property Components.AppTheme theme

    property int selectedRecipeIndex: -1
    property bool isEditing: false

    function currentRecipe() {
        if (selectedRecipeIndex >= 0 && selectedRecipeIndex < recipeListModel.count)
            return recipeListModel.get(selectedRecipeIndex)
        return null
    }

    ListModel {
        id: recipeListModel

        Component.onCompleted: {
            if (typeof recipeService !== 'undefined' && recipeService) {
                var recipes = recipeService.loadAll()
                for (var i = 0; i < recipes.length; i++) {
                    var r = recipes[i]
                    append({
                        fileName: r.fileName || "",
                        name: r.name || "",
                        homeDutyCycle: r.homeDutyCycle || 0,
                        idleDutyCycle: r.idleDutyCycle || 0,
                        idleSpeedMin: r.idleForwardSpeedAvgMin || 0,
                        idleSpeedMax: r.idleForwardSpeedAvgMax || 0,
                        idleCurrentMax: r.idleForwardCurrentAvgMax || 0,
                        loadDutyCycle: r.loadDutyCycle || 0,
                        loadTorqueMin: r.loadForwardTorqueMin || 0,
                        brakeRampStep: r.brakeRampEndCurrentA || 0,
                        brakeRampInterval: r.loadRampMs || 0,
                        lockSpeedThreshold: r.lockSpeedThresholdRpm || 0,
                        lockDuration: r.lockHoldMs || 0,
                        anglePositions: (r.position1TargetDeg || 0) + ", " + (r.position2TargetDeg || 0) + ", " + (r.position1TargetDeg || 0) + ", " + (r.position3TargetDeg || 0) + ", 0.0",
                        angleTolerance: r.position1ToleranceDeg || 0
                    })
                }
            }
        }
    }

    function selectRecipe(index) {
        selectedRecipeIndex = index
        isEditing = false
    }

    function createNewRecipe() {
        selectedRecipeIndex = -1
        isEditing = true
    }

    function editRecipe() {
        if (selectedRecipeIndex >= 0) {
            isEditing = true
        }
    }

    function saveRecipe() {
        var r = currentRecipe()
        if (!r) return
        if (typeof recipeService !== 'undefined' && recipeService) {
            var data = {
                fileName: r.fileName,
                name: r.name,
                homeDutyCycle: r.homeDutyCycle,
                idleDutyCycle: r.idleDutyCycle,
                idleForwardSpeedAvgMin: r.idleSpeedMin,
                idleForwardSpeedAvgMax: r.idleSpeedMax,
                idleForwardCurrentAvgMax: r.idleCurrentMax,
                loadDutyCycle: r.loadDutyCycle,
                loadForwardTorqueMin: r.loadTorqueMin,
                brakeRampEndCurrentA: r.brakeRampStep,
                loadRampMs: r.brakeRampInterval,
                lockSpeedThresholdRpm: r.lockSpeedThreshold,
                lockHoldMs: r.lockDuration,
                position1ToleranceDeg: r.angleTolerance
            }
            recipeService.save(data)
        }
        isEditing = false
    }

    function deleteRecipe() {
        if (selectedRecipeIndex < 0) return
        var r = currentRecipe()
        if (r && typeof recipeService !== 'undefined' && recipeService) {
            recipeService.remove(r.fileName)
        }
        recipeListModel.remove(selectedRecipeIndex)
        selectedRecipeIndex = -1
    }

    function exportRecipe() {
        var r = currentRecipe()
        if (r && typeof recipeService !== 'undefined' && recipeService) {
            recipeService.exportTo(r.fileName, "exports/" + r.fileName)
        }
    }

    function importRecipe() {
        if (typeof recipeService !== 'undefined' && recipeService) {
            var result = recipeService.importFrom("imports/recipe.json")
            if (result && result.fileName) {
                recipeListModel.append({
                    fileName: result.fileName || "",
                    name: result.name || "",
                    homeDutyCycle: result.homeDutyCycle || 0,
                    idleDutyCycle: result.idleDutyCycle || 0,
                    idleSpeedMin: result.idleForwardSpeedAvgMin || 0,
                    idleSpeedMax: result.idleForwardSpeedAvgMax || 0,
                    idleCurrentMax: result.idleForwardCurrentAvgMax || 0,
                    loadDutyCycle: result.loadDutyCycle || 0,
                    loadTorqueMin: result.loadForwardTorqueMin || 0,
                    brakeRampStep: result.brakeRampEndCurrentA || 0,
                    brakeRampInterval: result.loadRampMs || 0,
                    lockSpeedThreshold: result.lockSpeedThresholdRpm || 0,
                    lockDuration: result.lockHoldMs || 0,
                    anglePositions: (result.position1TargetDeg || 0) + ", " + (result.position2TargetDeg || 0) + ", " + (result.position1TargetDeg || 0) + ", " + (result.position3TargetDeg || 0) + ", 0.0",
                    angleTolerance: result.position1ToleranceDeg || 0
                })
            }
        }
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
                                required property int index
                                required property string name
                                required property string fileName

                                width: ListView.view.width
                                height: 80
                                radius: 6
                                color: root.selectedRecipeIndex === index ? root.theme.accent + "20" : "transparent"
                                border.width: root.selectedRecipeIndex === index ? 1 : 0
                                border.color: root.theme.accent

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.selectRecipe(index)
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
                                        text: fileName
                                        fontSize: 12
                                        color: root.theme.textSecondary
                                        theme: root.theme
                                    }

                                    Item { Layout.fillHeight: true }
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
                            text: root.isEditing ? (root.selectedRecipeIndex >= 0 ? "编辑配方" : "新建配方") : "配方详情"
                            fontSize: 16
                            fontWeight: 600
                            theme: root.theme
                        }

                        Item { Layout.fillWidth: true }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeIndex >= 0
                            text: "编辑"
                            variant: "default"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.editRecipe()
                        }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeIndex >= 0
                            text: "导出"
                            variant: "outline"
                            size: "sm"
                            theme: root.theme
                            onClicked: root.exportRecipe()
                        }

                        Components.AppButton {
                            visible: !root.isEditing && root.selectedRecipeIndex >= 0
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
                                visible: root.selectedRecipeIndex < 0 && !root.isEditing
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
                                visible: root.selectedRecipeIndex >= 0 || root.isEditing
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
                                        text: root.currentRecipe() ? root.currentRecipe().fileName : ""
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
                                        text: root.currentRecipe() ? root.currentRecipe().name : ""
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
                                        text: root.currentRecipe() ? root.currentRecipe().fileName : ""
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
                                        text: root.currentRecipe() ? root.currentRecipe().homeDutyCycle.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

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
                                        text: root.currentRecipe() ? root.currentRecipe().idleDutyCycle.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "转速下限 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().idleSpeedMin.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "转速上限 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().idleSpeedMax.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "电流上限 (A):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().idleCurrentMax.toFixed(2) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

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
                                        text: root.currentRecipe() ? root.currentRecipe().anglePositions : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "角度容差 (°):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().angleTolerance.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }
                                }

                                Components.AppSeparator {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                }

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
                                        text: root.currentRecipe() ? root.currentRecipe().loadDutyCycle.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "扭矩下限 (N·m):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().loadTorqueMin.toFixed(2) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "制动电流步进 (A):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().brakeRampStep.toFixed(2) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "步进间隔 (ms):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().brakeRampInterval.toString() : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "锁止转速阈值 (RPM):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().lockSpeedThreshold.toFixed(1) : ""
                                        readOnly: !root.isEditing
                                        theme: root.theme
                                    }

                                    Components.AppLabel {
                                        text: "锁止确认时长 (ms):"
                                        theme: root.theme
                                    }
                                    Components.AppInput {
                                        Layout.fillWidth: true
                                        text: root.currentRecipe() ? root.currentRecipe().lockDuration.toString() : ""
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
