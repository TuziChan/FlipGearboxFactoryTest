// 示例：使用 Qt Graphs 的 QML 代码
import QtQuick
import QtGraphs

GraphsView {
    anchors.fill: parent
    theme: GraphsTheme {
        colorScheme: GraphsTheme.ColorScheme.Dark
    }
    
    LineSeries {
        name: "转速"
        color: "#0078D4"
        XYPoint { x: 0; y: 0 }
        XYPoint { x: 1; y: 100 }
        XYPoint { x: 2; y: 200 }
    }
    
    LineSeries {
        name: "扭矩"
        color: "#E74856"
        XYPoint { x: 0; y: 0 }
        XYPoint { x: 1; y: 1.5 }
        XYPoint { x: 2; y: 1.8 }
    }
}
