pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property int orientation: Qt.Horizontal
    property bool live: true
    property bool disabled: false
    property real value: 0
    property var values: [0]
    readonly property bool pressed: trackMouse.pressed

    property bool _syncing: false

    signal valueCommitted(var values)

    function _toArray(input) {
        if (Array.isArray(input))
            return input.slice()
        if (input !== undefined && input !== null && input.length !== undefined) {
            const converted = []
            for (let index = 0; index < input.length; ++index)
                converted.push(input[index])
            return converted
        }
        return [input]
    }

    function _clamp(nextValue) {
        return Math.max(root.from, Math.min(root.to, Number(nextValue)))
    }

    function _snap(nextValue) {
        if (root.stepSize <= 0)
            return root._clamp(nextValue)
        const steps = Math.round((root._clamp(nextValue) - root.from) / root.stepSize)
        return root.from + steps * root.stepSize
    }

    function _normalized(nextValues) {
        const candidateValues = root._toArray(nextValues)
        const input = candidateValues.length > 0 ? candidateValues : [root.value]
        const normalized = []
        for (let i = 0; i < input.length; ++i)
            normalized.push(root._snap(input[i]))
        normalized.sort((a, b) => a - b)
        return normalized
    }

    function _positionForValue(nextValue) {
        if (root.to <= root.from)
            return 0
        return (nextValue - root.from) / (root.to - root.from)
    }

    function setValue(nextValue) {
        const snapped = root._snap(nextValue)
        root._syncing = true
        root.value = snapped
        root.values = [snapped]
        root._syncing = false
        root.valueCommitted(root.values.slice())
        return true
    }

    function setValues(nextValues) {
        const normalized = root._normalized(nextValues)
        root._syncing = true
        root.values = normalized
        root.value = normalized[0]
        root._syncing = false
        root.valueCommitted(root.values.slice())
        return true
    }

    function setNearestHandleFromPosition(position) {
        if (root.disabled)
            return false

        const normalizedPosition = Math.max(0, Math.min(1, position))
        const nextValue = root.from + normalizedPosition * (root.to - root.from)
        const snapped = root._snap(nextValue)
        const current = root._normalized(root.values)
        let nearestIndex = 0
        let nearestDistance = Math.abs(current[0] - snapped)

        for (let index = 1; index < current.length; ++index) {
            const distance = Math.abs(current[index] - snapped)
            if (distance < nearestDistance) {
                nearestDistance = distance
                nearestIndex = index
            }
        }

        current[nearestIndex] = snapped
        return root.setValues(current)
    }

    onValueChanged: {
        if (!root._syncing)
            root.setValue(value)
    }

    onValuesChanged: {
        if (!root._syncing)
            root.setValues(values)
    }

    Component.onCompleted: {
        if (!Array.isArray(root.values) || root.values.length === 0)
            root.values = [root.value]
        else
            root.setValues(root.values)
    }

    implicitWidth: root.orientation === Qt.Vertical ? 28 : 220
    implicitHeight: root.orientation === Qt.Vertical ? 160 : 24

    Rectangle {
        id: track
        x: root.orientation === Qt.Horizontal ? 0 : (parent.width - width) / 2
        y: root.orientation === Qt.Horizontal ? (parent.height - height) / 2 : 0
        width: root.orientation === Qt.Horizontal ? parent.width : 6
        height: root.orientation === Qt.Horizontal ? 6 : parent.height
        radius: 999
        color: root.theme.muted

        Rectangle {
            readonly property var normalizedValues: root._normalized(root.values)
            readonly property real startPos: normalizedValues.length > 0 ? root._positionForValue(normalizedValues[0]) : 0
            readonly property real endPos: normalizedValues.length > 0 ? root._positionForValue(normalizedValues[normalizedValues.length - 1]) : 0

            x: root.orientation === Qt.Horizontal ? startPos * parent.width : 0
            y: root.orientation === Qt.Horizontal ? 0 : parent.height - endPos * parent.height
            width: root.orientation === Qt.Horizontal ? Math.max(0, (endPos - startPos) * parent.width) : parent.width
            height: root.orientation === Qt.Horizontal ? parent.height : Math.max(0, (endPos - startPos) * parent.height)
            radius: parent.radius
            color: root.theme.accent
        }

        MouseArea {
            id: trackMouse
            anchors.fill: parent
            enabled: !root.disabled
            onPressed: function(mouseEvent) {
                const nextPosition = root.orientation === Qt.Horizontal ? mouseEvent.x / track.width : 1.0 - mouseEvent.y / track.height
                root.setNearestHandleFromPosition(nextPosition)
            }
            onPositionChanged: function(mouseEvent) {
                if (root.live && pressed) {
                    const nextPosition = root.orientation === Qt.Horizontal ? mouseEvent.x / track.width : 1.0 - mouseEvent.y / track.height
                    root.setNearestHandleFromPosition(nextPosition)
                }
            }
            onReleased: function(mouseEvent) {
                if (!root.live) {
                    const nextPosition = root.orientation === Qt.Horizontal ? mouseEvent.x / track.width : 1.0 - mouseEvent.y / track.height
                    root.setNearestHandleFromPosition(nextPosition)
                }
            }
        }
    }

    Repeater {
        model: root._normalized(root.values)

        delegate: Rectangle {
            required property var modelData
            width: 16
            height: 16
            radius: 8
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.accent
            x: root.orientation === Qt.Horizontal
               ? root._positionForValue(Number(modelData)) * (root.width - width)
               : (root.width - width) / 2
            y: root.orientation === Qt.Horizontal
               ? (root.height - height) / 2
               : (1.0 - root._positionForValue(Number(modelData))) * (root.height - height)
        }
    }
}
