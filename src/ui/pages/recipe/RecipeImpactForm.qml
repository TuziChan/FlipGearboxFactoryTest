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

    property bool impactEnabled: root.recipe ? root.recipe.impactTestEnabled : false

    ColumnLayout {
        id: contentColumn
        width: parent.width - parent.leftPadding - parent.rightPadding
        spacing: 12

        Components.AppLabel { text: "冲击测试配置"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "启用冲击测试:"; theme: root.theme }
            Components.AppCheckbox {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.preferredWidth: implicitWidth
                Layout.preferredHeight: implicitHeight
                checked: root.impactEnabled
                disabled: !root.isEditing
                theme: root.theme
                onToggled: function(checked) { if (root.recipeVM) root.recipeVM.updateEditField("impactTestEnabled", checked) }
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme; visible: root.impactEnabled }

        Components.AppLabel { text: "测试参数"; fontSize: 13; fontWeight: 600; theme: root.theme; visible: root.impactEnabled }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8
            visible: root.impactEnabled

            Components.AppLabel { text: "冲击占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactDutyCycle", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "稳定时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.impactSpinupMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactSpinupMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "循环次数:"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.impactCycles) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactCycles", parseInt(text) || 0)
            }

            Components.AppLabel { text: "制动电流 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactBrakeCurrentA, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactBrakeCurrentA", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "制动持续时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.impactBrakeOnMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactBrakeOnMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "制动间隔时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.impactBrakeOffMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactBrakeOffMs", parseInt(text) || 0)
            }

            Components.AppLabel { text: "超时时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.impactTimeoutMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactTimeoutMs", parseInt(text) || 0)
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme; visible: root.impactEnabled }

        Components.AppLabel { text: "正向冲击限值"; fontSize: 13; fontWeight: 600; theme: root.theme; visible: root.impactEnabled }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8
            visible: root.impactEnabled

            Components.AppLabel { text: "电流最小值 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactForwardCurrentMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactForwardCurrentMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流最大值 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactForwardCurrentMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactForwardCurrentMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "扭矩最小值 (Nm):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactForwardTorqueMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactForwardTorqueMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "扭矩最大值 (Nm):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactForwardTorqueMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactForwardTorqueMax", parseFloat(text) || 0)
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme; visible: root.impactEnabled }

        Components.AppLabel { text: "反向冲击限值"; fontSize: 13; fontWeight: 600; theme: root.theme; visible: root.impactEnabled }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8
            visible: root.impactEnabled

            Components.AppLabel { text: "电流最小值 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactReverseCurrentMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactReverseCurrentMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "电流最大值 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactReverseCurrentMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactReverseCurrentMax", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "扭矩最小值 (Nm):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactReverseTorqueMin, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactReverseTorqueMin", parseFloat(text) || 0)
            }

            Components.AppLabel { text: "扭矩最大值 (Nm):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.impactReverseTorqueMax, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipeVM) root.recipeVM.updateEditField("impactReverseTorqueMax", parseFloat(text) || 0)
            }
        }
    }
}
