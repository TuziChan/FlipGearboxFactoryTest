// QtChartsTest.qml - 简单测试 Qt Charts QML API
import QtQuick
import QtQuick.Controls
import QtCharts

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Qt Charts QML API 测试"

    ChartView {
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        antialiasing: true
        title: "测试图表"
        
        ValueAxis {
            id: axisX
            min: 0
            max: 10
            titleText: "X 轴"
        }
        
        ValueAxis {
            id: axisY
            min: 0
            max: 100
            titleText: "Y 轴"
        }
        
        LineSeries {
            name: "测试数据"
            axisX: axisX
            axisY: axisY
            color: "#0078D4"
            width: 2
            
            XYPoint { x: 0; y: 10 }
            XYPoint { x: 1; y: 30 }
            XYPoint { x: 2; y: 25 }
            XYPoint { x: 3; y: 50 }
            XYPoint { x: 4; y: 45 }
            XYPoint { x: 5; y: 70 }
            XYPoint { x: 6; y: 65 }
            XYPoint { x: 7; y: 80 }
            XYPoint { x: 8; y: 75 }
            XYPoint { x: 9; y: 90 }
            XYPoint { x: 10; y: 85 }
        }
    }
    
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        text: "✅ Qt Charts QML API 工作正常！"
        color: "#16C60C"
        font.pixelSize: 16
        font.bold: true
    }
}
