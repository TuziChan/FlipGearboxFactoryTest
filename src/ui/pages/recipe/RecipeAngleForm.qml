pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../../components" as Components
Components.AppScrollArea {
    id: root

    required property var recipe
    required property bool isEditing
    property var recipeVM: null

    function fv(val, d) { return typeof val === 'number' ? val.toFixed(d) : "" }
    function fi(val) { return typeof val === 'number' ? val.toString() : "" }

    ColumnLayout {
        id: contentColumn
        width: parent.width - parent.leftPadding - parent.rightPadding
        spacing: 8

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "角度测试占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.angleTestDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("angleTestDutyCycle", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位1目标角度 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position1TargetDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position1TargetDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位1容差 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position1ToleranceDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position1ToleranceDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位2目标角度 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position2TargetDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position2TargetDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位2容差 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position2ToleranceDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position2ToleranceDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位3目标角度 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position3TargetDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position3TargetDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "位3容差 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.position3ToleranceDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("position3ToleranceDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "回零容差 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.returnZeroToleranceDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("returnZeroToleranceDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "角度超时 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.angleTimeoutMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("angleTimeoutMs", parseInt(text) || 0)
            }
        }
    }
}
