pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property var headers
    required property var rowModel
    required property var columnKeys
    required property var resultKey
    required property string pendingText

    function rowAt(index) {
        if (Array.isArray(root.rowModel))
            return root.rowModel[index]
        if (root.rowModel && typeof root.rowModel.get === "function")
            return root.rowModel.get(index)
        return null
    }

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.dividerColor

    implicitHeight: tableColumn.implicitHeight + 12

    ColumnLayout {
        id: tableColumn
        anchors.margins: 6
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: root.theme.sectionColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14

                Repeater {
                    model: root.headers

                    delegate: Text {
                        required property string modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        text: modelData
                        color: root.theme.textSecondary
                        font.pixelSize: 10
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        Repeater {
            model: root.rowModel

            delegate: Rectangle {
                required property int index
                property var rowItem: root.rowAt(index) || ({})

                Layout.fillWidth: true
                Layout.preferredHeight: 32
                color: "transparent"
                border.color: root.theme.dividerColor
                border.width: 0.5

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Repeater {
                        model: root.columnKeys

                        delegate: Text {
                            required property string modelData
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            property string cellKey: modelData
                            property string cellValue: rowItem[cellKey] !== undefined ? String(rowItem[cellKey]) : ""
                            text: cellValue
                            color: cellKey === root.resultKey
                                   ? (cellValue === "OK" ? root.theme.okColor : cellValue === "NG" ? root.theme.ngColor : root.theme.textMuted)
                                   : root.theme.textPrimary
                            font.pixelSize: 11
                            font.bold: cellKey === root.resultKey && cellValue !== root.pendingText
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}
