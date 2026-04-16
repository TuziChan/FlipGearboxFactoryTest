import QtQuick

Canvas {
    id: root

    required property string iconName
    required property color iconColor

    implicitWidth: 20
    implicitHeight: 20

    onIconNameChanged: requestPaint()
    onIconColorChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        ctx.strokeStyle = root.iconColor
        ctx.fillStyle = root.iconColor
        ctx.lineWidth = 1.55
        ctx.lineCap = "round"
        ctx.lineJoin = "round"

        if (root.iconName === "test") {
            ctx.strokeRect(4, 4, 4, 4)
            ctx.strokeRect(12, 4, 4, 4)
            ctx.strokeRect(4, 12, 4, 4)
            ctx.strokeRect(12, 12, 4, 4)
        } else if (root.iconName === "recipe") {
            for (let i = 0; i < 3; ++i) {
                const y = 5 + i * 5
                ctx.beginPath()
                ctx.arc(5, y, 1.0, 0, Math.PI * 2)
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(8.5, y)
                ctx.lineTo(16, y)
                ctx.stroke()
            }
        } else if (root.iconName === "history") {
            ctx.beginPath()
            ctx.moveTo(4.5, 14)
            ctx.lineTo(8, 10.5)
            ctx.lineTo(11, 12)
            ctx.lineTo(15.5, 7)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(4.5, 14, 1.0, 0, Math.PI * 2)
            ctx.arc(8, 10.5, 1.0, 0, Math.PI * 2)
            ctx.arc(11, 12, 1.0, 0, Math.PI * 2)
            ctx.arc(15.5, 7, 1.0, 0, Math.PI * 2)
            ctx.fill()
        } else if (root.iconName === "device") {
            ctx.strokeRect(5.5, 3.5, 9, 12)
            ctx.beginPath()
            ctx.moveTo(8, 7)
            ctx.lineTo(12, 7)
            ctx.moveTo(8, 10)
            ctx.lineTo(12, 10)
            ctx.moveTo(8, 13)
            ctx.lineTo(10.5, 13)
            ctx.stroke()
        } else if (root.iconName === "diagnostics") {
            ctx.beginPath()
            ctx.arc(10, 10, 5.5, 0, Math.PI * 2)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(10, 10)
            ctx.lineTo(13.5, 7.5)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(10, 10, 1.15, 0, Math.PI * 2)
            ctx.fill()
        } else if (root.iconName === "library") {
            ctx.strokeRect(4.5, 4.5, 11, 11)
            ctx.beginPath()
            ctx.moveTo(8, 4.5)
            ctx.lineTo(8, 15.5)
            ctx.moveTo(12, 4.5)
            ctx.lineTo(12, 15.5)
            ctx.stroke()
        } else if (root.iconName === "chevron-down") {
            ctx.beginPath()
            ctx.moveTo(6, 8)
            ctx.lineTo(10, 12)
            ctx.lineTo(14, 8)
            ctx.stroke()
        } else if (root.iconName === "chevron-up") {
            ctx.beginPath()
            ctx.moveTo(6, 12)
            ctx.lineTo(10, 8)
            ctx.lineTo(14, 12)
            ctx.stroke()
        } else if (root.iconName === "chevron-left") {
            ctx.beginPath()
            ctx.moveTo(12, 6)
            ctx.lineTo(8, 10)
            ctx.lineTo(12, 14)
            ctx.stroke()
        } else if (root.iconName === "chevron-right") {
            ctx.beginPath()
            ctx.moveTo(8, 6)
            ctx.lineTo(12, 10)
            ctx.lineTo(8, 14)
            ctx.stroke()
        } else if (root.iconName === "check") {
            ctx.beginPath()
            ctx.moveTo(5.5, 10.5)
            ctx.lineTo(8.5, 13.5)
            ctx.lineTo(14.5, 6.5)
            ctx.stroke()
        } else if (root.iconName === "close") {
            ctx.beginPath()
            ctx.moveTo(6, 6)
            ctx.lineTo(14, 14)
            ctx.moveTo(14, 6)
            ctx.lineTo(6, 14)
            ctx.stroke()
        } else {
            ctx.beginPath()
            ctx.arc(10, 10, 5, 0, Math.PI * 2)
            ctx.stroke()
        }
    }
}
