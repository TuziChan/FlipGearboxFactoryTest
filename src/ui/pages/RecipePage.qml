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

    property var editingRecipe: null

    property int currentTabIndex: 0

    property var recipeServiceRef: typeof recipeService !== 'undefined' ? recipeService : null

    function currentRecipe() {

        if (selectedRecipeIndex >= 0 && selectedRecipeIndex < recipeListModel.count)

            return recipeListModel.get(selectedRecipeIndex)

        return null

    }

    function activeRecipeData() {

        if (root.isEditing && root.editingRecipe) return root.editingRecipe

        return currentRecipe()

    }

    ListModel {

        id: recipeListModel

        Component.onCompleted: {

            if (root.recipeServiceRef) {

                var recipes = root.recipeServiceRef.loadAll()

                for (var i = 0; i < recipes.length; i++) {

                    recipeListModel.append(recipes[i])

                }

            }

        }

    }

    function selectRecipe(index) {

        selectedRecipeIndex = index

        isEditing = false

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

        root.editingRecipe = defaultRecipe()

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

                root.editingRecipe = d

            }

            root.isEditing = true

        }

    }

    function saveRecipe() {

        var data = root.editingRecipe

        if (!data) return

        if (root.recipeServiceRef) {

            var ok = root.recipeServiceRef.save(data)

            if (!ok) {

                console.warn("Failed to save recipe")

                return

            }

        }

        if (root.selectedRecipeIndex >= 0) {

            var keys = Object.keys(data)

            for (var i = 0; i < keys.length; i++) {

                recipeListModel.setProperty(root.selectedRecipeIndex, keys[i], data[keys[i]])

            }

        } else {

            recipeListModel.append(data)

            root.selectedRecipeIndex = recipeListModel.count - 1

        }

        root.isEditing = false

        root.editingRecipe = null

    }

    function deleteRecipe() {

        if (root.selectedRecipeIndex < 0) return

        var r = currentRecipe()

        if (r && root.recipeServiceRef) {

            root.recipeServiceRef.remove(r.fileName)

        }

        recipeListModel.remove(root.selectedRecipeIndex)

        root.selectedRecipeIndex = -1

    }

    function exportRecipe() {

        var r = currentRecipe()

        if (r && root.recipeServiceRef) {

            root.recipeServiceRef.exportTo(r.fileName, "exports/" + r.fileName)

        }

    }

    function importRecipe() {

        if (root.recipeServiceRef) {

            var result = root.recipeServiceRef.importFrom("imports/recipe.json")

            if (result && result.fileName) {

                recipeListModel.append(result)

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

                                MouseArea {

                                    anchors.fill: parent

                                    onClicked: root.selectRecipe(recipeDelegate.index)

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

                            onClicked: { root.isEditing = false; root.editingRecipe = null }

                        }

                    }

                    Components.AppSeparator {

                        Layout.fillWidth: true

                        theme: root.theme

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

                            }

                            RecipeForms.RecipeHomingForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                            }

                            RecipeForms.RecipeIdleForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                            }

                            RecipeForms.RecipeAngleForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                            }

                            RecipeForms.RecipeLoadForm {

                                theme: root.theme

                                recipe: root.activeRecipeData()

                                isEditing: root.isEditing

                            }

                        }

                    }

                }

            }

        }

    }

}

