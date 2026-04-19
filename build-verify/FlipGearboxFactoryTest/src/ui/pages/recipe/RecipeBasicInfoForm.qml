pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../../components" as Components
Components.AppScrollArea {
    id: root

    required property var recipe
    required property bool isEditing

    ColumnLayout {
        id: contentColumn
        width: parent.width - parent.leftPadding - parent.rightPadding
        spacing: 8

        GridLayout {
            Layout.fillWidth: true
            columns: contentColumn.width > 600 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 8

            Components.AppLabel { text: "配方ID:"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                placeholderText: "GBX-XXX"
                text: root.recipe ? root.recipe.fileName : ""
                readOnly: !root.isEditing
                theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.fileName = text
            }

            Components.AppLabel { text: "配方名称:"; theme: root.theme }
            Components.AppInput {
                Layout.fillWidth: true
                placeholderText: "输入配方名称"
                text: root.recipe ? root.recipe.name : ""
                readOnly: !root.isEditing
                theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.name = text
            }

            Components.AppLabel { text: "描述:"; theme: root.theme; Layout.alignment: Qt.AlignTop }
            Components.AppTextarea {
                Layout.fillWidth: true
                placeholderText: "输入配方描述"
                text: root.recipe ? (root.recipe.description || "") : ""
                readOnly: !root.isEditing
                theme: root.theme
                onTextChanged: if (root.isEditing && root.recipe) root.recipe.description = text
            }
        }
    }
}
