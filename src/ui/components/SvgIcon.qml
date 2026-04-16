import QtQuick
import QtQuick.Shapes

// SVG-based icon component with automatic scaling
// Designed in 20x20 coordinate space, scales to any size
Item {
    id: root

    required property string name
    required property color color
    property int iconSize: 20

    implicitWidth: iconSize
    implicitHeight: iconSize

    Shape {
        id: shape
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        // Transform to scale from 20x20 design space to actual size
        transform: Scale {
            xScale: root.width / 20.0
            yScale: root.height / 20.0
            origin.x: 0
            origin.y: 0
        }

        // Check icon (✓)
        ShapePath {
            visible: root.name === "check"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 5.5
            startY: 10.5
            PathLine { x: 8.5; y: 13.5 }
            PathLine { x: 14.5; y: 6.5 }
        }

        // Chevron Down
        ShapePath {
            visible: root.name === "chevron-down"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 8
            PathLine { x: 10; y: 12 }
            PathLine { x: 14; y: 8 }
        }

        // Chevron Up
        ShapePath {
            visible: root.name === "chevron-up"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 12
            PathLine { x: 10; y: 8 }
            PathLine { x: 14; y: 12 }
        }

        // Chevron Left
        ShapePath {
            visible: root.name === "chevron-left"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 12
            startY: 6
            PathLine { x: 8; y: 10 }
            PathLine { x: 12; y: 14 }
        }

        // Chevron Right
        ShapePath {
            visible: root.name === "chevron-right"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 8
            startY: 6
            PathLine { x: 12; y: 10 }
            PathLine { x: 8; y: 14 }
        }

        // Close (X)
        ShapePath {
            visible: root.name === "close"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 6
            PathLine { x: 14; y: 14 }
        }
        ShapePath {
            visible: root.name === "close"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 14
            startY: 6
            PathLine { x: 6; y: 14 }
        }

        // Search icon (magnifying glass)
        ShapePath {
            visible: root.name === "search"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: 9
                centerY: 9
                radiusX: 5
                radiusY: 5
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            visible: root.name === "search"
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: 1.55
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 13
            startY: 13
            PathLine { x: 17; y: 17 }
        }
    }
}
