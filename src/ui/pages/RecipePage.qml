pragma ComponentBehavior: Bound

import QtQuick

import QtQuick.Layouts

import QtQuick.Controls as QQC

import "../components" as Components

import "recipe" as RecipeForms

Item {

    id: root

    objectName: "recipePage"

    required property Components.AppTheme theme

    property int selectedRecipeIndex: -1

    property bool isEditing: false

    property int currentTabIndex: 0

    property var recipeVM: typeof recipeViewModel !== 'undefined' ? recipeViewModel : null

    property string validationError: root.recipeVM ? root.recipeVM.lastError : ""

    function currentRecipe() {

        if (selectedRecipeIndex >= 0 && selectedRecipeIndex < recipeListModel.count)

            return recipeListModel.get(selectedRecipeIndex)

        return null

    }

    function activeRecipeData() {

        if (root.isEditing && root.recipeVM && root.recipeVM.editingRecipe) return root.recipeVM.editingRecipe

        return currentRecipe()

    }

    ListModel {

        id: recipeListModel

    }

    Connections {
        target: root.recipeVM
        enabled: root.recipeVM !== null

        function onRecipesChanged() {
            recipeListModel.clear()
            var recipes = root.recipeVM.recipes
            for (var i = 0; i < recipes.length; i++) {
                recipeListModel.append(recipes[i])
            }
        }
    }

    function selectRecipe(index) {

        selectedRecipeIndex = index

        isEditing = false

        if (root.recipeVM) root.recipeVM.cancelEdit()

    }

    function defaultRecipe() {

        return {

            fileName: "", name: "", description: "",

            homeDutyCycle: 20.0, homeAdvanceDutyCycle: 20.0,

            encoderZeroAngleDeg: 3.0, homeTimeoutMs: 30000,

            idleDutyCycle: 50.0,

            idleForwardSpinupMs: 3000, idleForwardSampleMs: 2000,

            idleReverseSpinupMs: 3000, idleReverseSampleMs: 2000,

            idleForwardCurrentAvgMin: 0.5, idleForwardCurrentAvgMax: 2.0,

            idleForwardCurrentMaxMin: 0.6, idleForwardCurrentMaxMax: 2.5,

            idleForwardSpeedAvgMin: 50.0, idleForwardSpeedAvgMax: 150.0,

            idleForwardSpeedMaxMin: 60.0, idleForwardSpeedMaxMax: 160.0,

            idleReverseCurrentAvgMin: 0.5, idleReverseCurrentAvgMax: 2.0,

            idleReverseCurrentMaxMin: 0.6, idleReverseCurrentMaxMax: 2.5,

            idleReverseSpeedAvgMin: 50.0, idleReverseSpeedAvgMax: 150.0,

            idleReverseSpeedMaxMin: 60.0, idleReverseSpeedMaxMax: 160.0,

            angleTestDutyCycle: 30.0,

            position1TargetDeg: 3.0, position1ToleranceDeg: 3.0,

            position2TargetDeg: 49.0, position2ToleranceDeg: 3.0,

            position3TargetDeg: 113.5, position3ToleranceDeg: 3.0,

            returnZeroToleranceDeg: 1.0, angleTimeoutMs: 15000,

            loadDutyCycle: 50.0, loadSpinupMs: 3000,

            brakeMode: "CC",

            brakeRampStartCurrentA: 0.0, brakeRampEndCurrentA: 3.0,

            brakeRampStartVoltage: 0.0, brakeRampEndVoltage: 12.0,

            loadRampMs: 2000,

            lockSpeedThresholdRpm: 2.0, lockAngleWindowMs: 100,

            lockAngleDeltaDeg: 5.0, lockHoldMs: 500,

            loadForwardCurrentMin: 1.0, loadForwardCurrentMax: 3.0,

            loadForwardTorqueMin: 10.0, loadForwardTorqueMax: 50.0,

            loadReverseCurrentMin: 1.0, loadReverseCurrentMax: 3.0,

            loadReverseTorqueMin: 10.0, loadReverseTorqueMax: 50.0

        }

    }

    function createNewRecipe() {

        if (root.recipeVM) root.recipeVM.beginEdit(root.defaultRecipe())

        root.selectedRecipeIndex = -1

        root.isEditing = true

    }

    function editRecipe() {

        if (selectedRecipeIndex >= 0) {

            var r = currentRecipe()

            if (r) {

                var d = defaultRecipe()

                var keys = Object.keys(d)

                for (var i = 0; i < keys.length; i++) {

                    var k = keys[i]

                    if (typeof r[k] !== 'undefined') d[k] = r[k]

                }

                if (root.recipeVM) root.recipeVM.beginEdit(d)

            }

            root.isEditing = true

        }

    }

    function saveRecipe() {

        if (!root.recipeVM) return

        var savedFileName = root.recipeVM.editingRecipe ? root.recipeVM.editingRecipe.fileName : ""

        var wasNew = root.selectedRecipeIndex < 0

        var ok = root.recipeVM.saveEdit()

        if (!ok) return

        root.isEditing = false

        if (wasNew && savedFileName) {

            for (var i = 0; i < recipeListModel.count; i++) {

                if (recipeListModel.get(i).fileName === savedFileName) {

                    root.selectedRecipeIndex = i

                    break

                }

            }

        }

    }

    function deleteRecipe() {

        if (root.selectedRecipeIndex < 0) return

        var r = currentRecipe()

        if (r && root.recipeVM) {

            root.recipeVM.remove(r.fileName)

        }

        root.selectedRecipeIndex = -1

    }

    function exportRecipe() {

        var r = currentRecipe()

        if (r && root.recipeVM) {

            root.recipeVM.exportTo(r.fileName, "exports/" + r.fileName)

        }

    }

    function importRecipe() {

        if (root.recipeVM) {

            root.recipeVM.importFrom("imports/recipe.json")

        }

    }

    Component.onCompleted: {
        if (root.recipeVM) root.recipeVM.loadAll()
    }

    Rectangle {

        anchors.fill: parent

        color: root.theme.bgColor

        RowLayout {

            anchors.fill: parent

            anchors.margins: 16

            spacing: 16

            Components.AppCard {

                Layout.fillHeight: true

                Layout.preferredWidth: 360

                theme: root.theme

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    spacing: 12

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

                    QQC.ScrollView {

                        Layout.fillWidth: true

                        Layout.fillHeight: true

                        clip: true

                        ListView {

                            id: recipeListView

                            model: recipeListModel

                            spacing: 8

                            delegate: Rectangle {

                                id: recipeDelegate

                                required property int index

                                required property string name

                                required property string fileName

                                width: ListView.view.width

                                height: 80

                                radius: 6

                                color: root.selectedRecipeIndex === recipeDelegate.index ? root.theme.accent + "20" : "transparent"

                                border.width: root.selectedRecipeIndex === recipeDelegate.index ? 1 : 0

                                border.color: root.theme.accent

                                TapHandler {

                                    onTapped: root.selectRecipe(recipeDelegate.index)

                                }

                                ColumnLayout {

                                    anchors.fill: parent

                                    anchors.margins: 12

                                    spacing: 4

                                    Components.AppLabel {

                                        text: recipeDelegate.name

                                        fontSize: 14

                                        fontWeight: 500

                                        theme: root.theme

                                    }

                                    Components.AppLabel {

                                        text: recipeDelegate.fileName

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

            Components.AppCard {

                Layout.fillWidth: true

                Layout.fillHeight: true

                theme: root.theme

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    spacing: 8

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

                            onClicked: { if (root.recipeVM) root.recipeVM.cancelEdit(); root.isEditing = false }

                        }

                    }

                    Components.AppSeparator {

                        Layout.fillWidth: true

                        theme: root.theme

                    }

                    Components.AppAlert {

                        Layout.fillWidth: true

                        theme: root.theme

                        variant: "destructive"

                        description: root.validationError

                        visible: root.validationError.length > 0

                    }

                    Item {

                        visible: root.selectedRecipeIndex < 0 && !root.isEditing

                        Layout.fillWidth: true

                        Layout.fillHeight: true

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

                    ColumnLayout {

                        visible: root.selectedRecipeIndex >= 0 || root.isEditing

                        Layout.fillWidth: true

                        Layout.fillHeight: true

                        spacing: 8

                        Components.AppTabs {

                            id: tabBar

                            Layout.fillWidth: true

                            theme: root.theme

                            model: ["基本信息", "归零参数", "空载测试", "角度定位", "负载测试"]

                            currentIndex: root.currentTabIndex

                            showContent: false

                            onCurrentIndexChanged: root.currentTabIndex = currentIndex

                        }

                        StackLayout {

                            Layout.fillWidth: true

                            Layout.fillHeight: true

                            currentIndex: root.currentTabIndex

                            RecipeForms.RecipeBasicInfoForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                                recipeVM: root.recipeVM

                            }

                            RecipeForms.RecipeHomingForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                                recipeVM: root.recipeVM

                            }

                            RecipeForms.RecipeIdleForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                                recipeVM: root.recipeVM

                            }

                            RecipeForms.RecipeAngleForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                                recipeVM: root.recipeVM

                            }

                            RecipeForms.RecipeLoadForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                                recipeVM: root.recipeVM

                            }

                        }

                    }

                }

            }

        }

    }

}
