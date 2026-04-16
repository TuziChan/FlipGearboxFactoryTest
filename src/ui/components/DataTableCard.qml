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

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.dividerColor

    implicitHeight: tableColumn.implicitHeight + 12

    Column {
        id: tableColumn
        width: parent.width
        anchors.margins: 6
        anchors.fill: parent
        spacing: 0

        Rectangle {
            width: parent.width
            height: 30
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
                        text: modelData
                        color: root.theme.textSecondary
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }
        }

        Repeater {
            model: root.rowModel

            delegate: Rectangle {
                required property int index
                property var rowItem: root.rowModel.get(index)

                width: parent.width
                height: 32
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
                            property string cellKey: modelData
                            property string cellValue: rowItem[cellKey]
                            text: cellValue
                            color: cellKey === root.resultKey
                                   ? (cellValue === "OK" ? root.theme.okColor : cellValue === "NG" ? root.theme.ngColor : root.theme.textMuted)
                                   : root.theme.textPrimary
                            font.pixelSize: 11
                            font.bold: cellKey === root.resultKey && cellValue !== root.pendingText
                        }
                    }
                }
            }
        }
    }
}
