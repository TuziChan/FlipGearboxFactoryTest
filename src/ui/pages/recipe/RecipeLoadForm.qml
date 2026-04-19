pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../../components" as Components
Components.AppScrollArea {
    id: root

    required property var recipe
    required property bool isEditing

    function fv(val, d) { return typeof val === 'number' ? val.toFixed(d) : "" }
    function fi(val) { return typeof val === 'number' ? val.toString() : "" }

    property bool showCC: root.recipe ? root.recipe.brakeMode !== "CV" : true
    property bool showCV: root.recipe ? root.recipe.brakeMode === "CV" : false

    ColumnLayout {
        id: contentColumn
        width: parent.width - parent.leftPadding - parent.rightPadding
        spacing: 12

        Components.AppLabel { text: "通用参数"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "负载占空比 (%):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadDutyCycle, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadDutyCycle = parseFloat(text) || 0
            }

            Components.AppLabel { text: "电机稳定时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.loadSpinupMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadSpinupMs = parseInt(text) || 0
            }

            Components.AppLabel { text: "制动模式 (CC/CV):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? root.recipe.brakeMode : "CC"
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.brakeMode = (text === "CV") ? "CV" : "CC"
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "恒流参数"; fontSize: 13; fontWeight: 600; theme: root.theme; visible: root.showCC }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8
            visible: root.showCC

            Components.AppLabel { text: "制动起始电流 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.brakeRampStartCurrentA, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.brakeRampStartCurrentA = parseFloat(text) || 0
            }

            Components.AppLabel { text: "制动终止电流 (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.brakeRampEndCurrentA, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.brakeRampEndCurrentA = parseFloat(text) || 0
            }
        }

        Components.AppLabel { text: "恒压参数"; fontSize: 13; fontWeight: 600; theme: root.theme; visible: root.showCV }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8
            visible: root.showCV

            Components.AppLabel { text: "制动起始电压 (V):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.brakeRampStartVoltage, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.brakeRampStartVoltage = parseFloat(text) || 0
            }

            Components.AppLabel { text: "制动终止电压 (V):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.brakeRampEndVoltage, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.brakeRampEndVoltage = parseFloat(text) || 0
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "斜坡与锁止"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "斜坡时间 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.loadRampMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadRampMs = parseInt(text) || 0
            }

            Components.AppLabel { text: "锁止转速阈值 (RPM):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.lockSpeedThresholdRpm, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.lockSpeedThresholdRpm = parseFloat(text) || 0
            }

            Components.AppLabel { text: "锁止角度窗口 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.lockAngleWindowMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.lockAngleWindowMs = parseInt(text) || 0
            }

            Components.AppLabel { text: "锁止角度变化量 (°):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.lockAngleDeltaDeg, 1) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.lockAngleDeltaDeg = parseFloat(text) || 0
            }

            Components.AppLabel { text: "锁止确认时长 (ms):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fi(root.recipe.lockHoldMs) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.lockHoldMs = parseInt(text) || 0
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "正转限值"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "电流 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadForwardCurrentMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadForwardCurrentMin = parseFloat(text) || 0
            }

            Components.AppLabel { text: "电流 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadForwardCurrentMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadForwardCurrentMax = parseFloat(text) || 0
            }

            Components.AppLabel { text: "扭矩 Min (N·m):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadForwardTorqueMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadForwardTorqueMin = parseFloat(text) || 0
            }

            Components.AppLabel { text: "扭矩 Max (N·m):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadForwardTorqueMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadForwardTorqueMax = parseFloat(text) || 0
            }
        }

        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }

        Components.AppLabel { text: "反转限值"; fontSize: 13; fontWeight: 600; theme: root.theme }

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "电流 Min (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadReverseCurrentMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadReverseCurrentMin = parseFloat(text) || 0
            }

            Components.AppLabel { text: "电流 Max (A):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadReverseCurrentMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadReverseCurrentMax = parseFloat(text) || 0
            }

            Components.AppLabel { text: "扭矩 Min (N·m):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadReverseTorqueMin, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadReverseTorqueMin = parseFloat(text) || 0
            }

            Components.AppLabel { text: "扭矩 Max (N·m):"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                text: root.recipe ? fv(root.recipe.loadReverseTorqueMax, 2) : ""
                readOnly: !root.isEditing; theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.loadReverseTorqueMax = parseFloat(text) || 0
            }
        }
    }
}
