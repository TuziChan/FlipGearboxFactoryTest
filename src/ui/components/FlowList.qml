import QtQuick
import QtQuick.Layouts

Column {
    id: root

    required property AppTheme theme
    required property var model
    spacing: 4

    function rowAt(index) {
        if (Array.isArray(root.model))
            return root.model[index]
        if (root.model && typeof root.model.get === "function")
            return root.model.get(index)
        return null
    }

    Repeater {
        model: root.model

        delegate: FlowStepItem {
            required property int index
            property var row: root.rowAt(index) || ({})
            width: root.width
            theme: root.theme
            order: index + 1
            title: row.name || ""
            detail: row.detail || ""
            elapsedText: row.elapsed || ""
            state: row.state || "pending"
        }
    }
}
