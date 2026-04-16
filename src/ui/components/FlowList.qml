import QtQuick
import QtQuick.Layouts

Column {
    id: root

    required property AppTheme theme
    required property var model
    spacing: 4

    Repeater {
        model: root.model

        delegate: FlowStepItem {
            required property int index
            property var row: root.model.get(index)
            width: root.width
            theme: root.theme
            order: index + 1
            title: row.name
            detail: row.detail
            elapsedText: row.elapsed
            state: row.state
        }
    }
}
