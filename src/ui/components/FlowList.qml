pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Column {
    id: root

    required property AppTheme theme
    required property var model
    spacing: 0

    function rowAt(index) {
        if (Array.isArray(root.model))
            return root.model[index]
        if (root.model && typeof root.model.get === "function")
            return root.model.get(index)
        return null
    }

    Repeater {
        model: root.model

        delegate: Column {
            required property int index
            width: root.width

            FlowStepItem {
                width: parent.width
                theme: root.theme
                order: index + 1
                title: {
                    var r = root.rowAt(index) || ({})
                    return r.name || ""
                }
                detail: {
                    var r = root.rowAt(index) || ({})
                    return r.detail || ""
                }
                elapsedText: {
                    var r = root.rowAt(index) || ({})
                    return r.elapsed || ""
                }
                state: {
                    var r = root.rowAt(index) || ({})
                    return r.state || "pending"
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: root.theme.dividerColor
                visible: index < root.model.length - 1
                anchors.left: parent.left
                anchors.leftMargin: 12
            }
        }
    }
}

