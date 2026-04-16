import QtQuick
import QtQuick.Shapes

// SVG-based icon component with automatic scaling
// Designed in 20x20 coordinate space, scales to any size
Item {
    id: root

    required property string name
    required property color color
    property int iconSize: 20

    readonly property bool isCheck: root.name === "check"
    readonly property bool isChevronDown: root.name === "chevron-down"
    readonly property bool isChevronUp: root.name === "chevron-up"
    readonly property bool isChevronLeft: root.name === "chevron-left"
    readonly property bool isChevronRight: root.name === "chevron-right"
    readonly property bool isClose: root.name === "close"
    readonly property bool isSearch: root.name === "search"
    readonly property bool isInfo: root.name === "info"
    readonly property bool isError: root.name === "error"
    readonly property bool isTest: root.name === "test"
    readonly property bool isRecipe: root.name === "recipe"
    readonly property bool isHistory: root.name === "history"
    readonly property bool isDevice: root.name === "device"
    readonly property bool isDiagnostics: root.name === "diagnostics"
    readonly property bool isLibrary: root.name === "library"
    readonly property bool isKnown: isCheck || isChevronDown || isChevronUp || isChevronLeft || isChevronRight
                                  || isClose || isSearch || isInfo || isError || isTest || isRecipe
                                  || isHistory || isDevice || isDiagnostics || isLibrary

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
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isCheck ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 5.5
            startY: 10.5
            PathLine { x: 8.5; y: 13.5 }
            PathLine { x: 14.5; y: 6.5 }
        }

        // Chevron Down
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isChevronDown ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 8
            PathLine { x: 10; y: 12 }
            PathLine { x: 14; y: 8 }
        }

        // Chevron Up
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isChevronUp ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 12
            PathLine { x: 10; y: 8 }
            PathLine { x: 14; y: 12 }
        }

        // Chevron Left
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isChevronLeft ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 12
            startY: 6
            PathLine { x: 8; y: 10 }
            PathLine { x: 12; y: 14 }
        }

        // Chevron Right
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isChevronRight ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 8
            startY: 6
            PathLine { x: 12; y: 10 }
            PathLine { x: 8; y: 14 }
        }

        // Info icon
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isInfo ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: 10
                centerY: 10
                radiusX: 6
                radiusY: 6
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isInfo ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 10
            startY: 9
            PathLine { x: 10; y: 13 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: root.color
            strokeWidth: root.isInfo ? 1.1 : 0
            startX: 10
            startY: 6.5
            PathLine { x: 10.2; y: 6.5 }
        }

        // Error icon
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isError ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: 10
                centerY: 10
                radiusX: 6
                radiusY: 6
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isError ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 7.4
            startY: 7.4
            PathLine { x: 12.6; y: 12.6 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isError ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 12.6
            startY: 7.4
            PathLine { x: 7.4; y: 12.6 }
        }

        // Close (X)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isClose ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 6
            PathLine { x: 14; y: 14 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isClose ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 14
            startY: 6
            PathLine { x: 6; y: 14 }
        }

        // Search icon (magnifying glass)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isSearch ? 1.55 : 0
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
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isSearch ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 13
            startY: 13
            PathLine { x: 17; y: 17 }
        }

        // Test icon (beaker)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isTest ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 8
            startY: 5
            PathLine { x: 12; y: 5 }
            PathLine { x: 12; y: 7 }
            PathLine { x: 15.5; y: 13.5 }
            PathLine { x: 4.5; y: 13.5 }
            PathLine { x: 8; y: 7 }
            PathLine { x: 8; y: 5 }
        }

        // Recipe icon (list)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isRecipe ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 6
            PathLine { x: 14; y: 6 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isRecipe ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 10
            PathLine { x: 14; y: 10 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isRecipe ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 14
            PathLine { x: 12; y: 14 }
        }

        // History icon (clock)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isHistory ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: 10
                centerY: 10
                radiusX: 6
                radiusY: 6
                startAngle: 0
                sweepAngle: 300
            }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isHistory ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 10
            startY: 7
            PathLine { x: 10; y: 10 }
            PathLine { x: 12.5; y: 11.5 }
        }

        // Device icon (chip)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isDevice ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 6
            startY: 6
            PathLine { x: 14; y: 6 }
            PathLine { x: 14; y: 14 }
            PathLine { x: 6; y: 14 }
            PathLine { x: 6; y: 6 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isDevice ? 1.2 : 0
            startX: 8.5
            startY: 8.5
            PathLine { x: 11.5; y: 8.5 }
            PathLine { x: 11.5; y: 11.5 }
            PathLine { x: 8.5; y: 11.5 }
            PathLine { x: 8.5; y: 8.5 }
        }

        // Diagnostics icon (pulse line)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isDiagnostics ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 4
            startY: 10
            PathLine { x: 7; y: 10 }
            PathLine { x: 8.7; y: 7 }
            PathLine { x: 11; y: 13 }
            PathLine { x: 12.7; y: 10 }
            PathLine { x: 16; y: 10 }
        }

        // Library icon (book)
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isLibrary ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 5
            startY: 5.5
            PathLine { x: 10; y: 4.5 }
            PathLine { x: 10; y: 15.5 }
            PathLine { x: 5; y: 16.5 }
            PathLine { x: 5; y: 5.5 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: root.isLibrary ? 1.55 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 15
            startY: 5.5
            PathLine { x: 10; y: 4.5 }
            PathLine { x: 10; y: 15.5 }
            PathLine { x: 15; y: 16.5 }
            PathLine { x: 15; y: 5.5 }
        }

        // Fallback icon for unsupported names
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: !root.isKnown ? 1.2 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: 10
                centerY: 10
                radiusX: 6
                radiusY: 6
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: "transparent"
            strokeWidth: !root.isKnown ? 1.2 : 0
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 10
            startY: 7
            PathLine { x: 10; y: 11 }
        }
        ShapePath {
            strokeColor: root.color
            fillColor: root.color
            strokeWidth: !root.isKnown ? 1 : 0
            startX: 10
            startY: 13.5
            PathLine { x: 10.2; y: 13.5 }
        }
    }
}
