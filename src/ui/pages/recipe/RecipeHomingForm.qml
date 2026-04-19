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

            Components.AppLabel { text: "归零占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.homeDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("homeDutyCycle", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "前进占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.homeAdvanceDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("homeAdvanceDutyCycle", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "编码器零点角度 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.encoderZeroAngleDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("encoderZeroAngleDeg", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "归零超时 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.homeTimeoutMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("homeTimeoutMs", parseInt(text) || 0)
            }
        }
    }
}
