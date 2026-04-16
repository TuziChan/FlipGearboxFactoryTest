import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string title: ""
    property string description: ""
    property real panelWidth: 288
    default property alias content: contentColumn.data

    function openPopover() {
        root.open = true
        showAnimation.start()
        return true
    }

    function closePopover() {
        hideAnimation.start()
        return true
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open
        z: 50
        width: root.panelWidth
        implicitHeight: column.implicitHeight + 20
        color: root.theme.bgPopover
        border.width: 1
        border.color: Qt.rgba(0, 0, 0, 0.1)
        radius: root.theme.radiusLarge
        opacity: 0
        scale: 0.95

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Qt.rgba(0, 0, 0, 0.1)
            shadowBlur: 0.5
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 2
        }

        ParallelAnimation {
            id: showAnimation
            NumberAnimation {
                target: panel
                property: "opacity"
                from: 0
                to: 1
                duration: 100
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: panel
                property: "scale"
                from: 0.95
                to: 1.0
                duration: 100
                easing.type: Easing.OutCubic
            }
        }

        ParallelAnimation {
            id: hideAnimation
            NumberAnimation {
                target: panel
                property: "opacity"
                from: 1
                to: 0
                duration: 100
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                target: panel
                property: "scale"
                from: 1.0
                to: 0.95
                duration: 100
                easing.type: Easing.InCubic
            }
            onFinished: root.open = false
        }

        Column {
            id: column
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Column {
                width: parent.width
                spacing: 2
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    width: parent.width
                    color: root.theme.textPrimary
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    wrapMode: Text.WordWrap
                }

                Text {
                    visible: root.description.length > 0
                    text: root.description
                    width: parent.width
                    color: root.theme.textSecondary
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
            }

            Column {
                id: contentColumn
                width: parent.width
                spacing: 10
            }
        }
    }
}
