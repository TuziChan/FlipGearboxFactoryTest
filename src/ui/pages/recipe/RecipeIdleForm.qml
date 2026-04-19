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
        spacing: 12

        Components.AppLabel { text: "时序参数"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "空载占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleDutyCycle", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "正转稳定时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.idleForwardSpinupMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSpinupMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "正转采样窗口 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.idleForwardSampleMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSampleMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "反转稳定时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.idleReverseSpinupMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSpinupMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "反转采样窗口 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.idleReverseSampleMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSampleMs", parseInt(text) || 0)
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "正转限值"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "电流平均 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardCurrentAvgMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardCurrentAvgMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流平均 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardCurrentAvgMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardCurrentAvgMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流峰值 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardCurrentMaxMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardCurrentMaxMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流峰值 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardCurrentMaxMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardCurrentMaxMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速平均 Min (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardSpeedAvgMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSpeedAvgMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速平均 Max (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardSpeedAvgMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSpeedAvgMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速峰值 Min (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardSpeedMaxMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSpeedMaxMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速峰值 Max (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleForwardSpeedMaxMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleForwardSpeedMaxMax", parseFloat(text) || 0)
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "反转限值"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "电流平均 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseCurrentAvgMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseCurrentAvgMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流平均 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseCurrentAvgMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseCurrentAvgMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流峰值 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseCurrentMaxMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseCurrentMaxMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流峰值 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseCurrentMaxMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseCurrentMaxMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速平均 Min (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseSpeedAvgMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSpeedAvgMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速平均 Max (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseSpeedAvgMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSpeedAvgMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速峰值 Min (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseSpeedMaxMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSpeedMaxMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "转速峰值 Max (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.idleReverseSpeedMaxMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("idleReverseSpeedMaxMax", parseFloat(text) || 0)
            }
        }
    }
}
