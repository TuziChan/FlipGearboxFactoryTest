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

        const w = root.width
        const h = root.height
        const scale = Math.min(w, h) / 20.0

        ctx.lineWidth = 1.55 * scale
        ctx.lineCap = "round"
        ctx.lineJoin = "round"

        if (root.iconName === "test") {
            ctx.strokeRect(4 * scale, 4 * scale, 4 * scale, 4 * scale)
            ctx.strokeRect(12 * scale, 4 * scale, 4 * scale, 4 * scale)
            ctx.strokeRect(4 * scale, 12 * scale, 4 * scale, 4 * scale)
            ctx.strokeRect(12 * scale, 12 * scale, 4 * scale, 4 * scale)
        } else if (root.iconName === "recipe") {
            for (let i = 0; i < 3; ++i) {
                const y = (5 + i * 5) * scale
                ctx.beginPath()
                ctx.arc(5 * scale, y, 1.0 * scale, 0, Math.PI * 2)
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(8.5 * scale, y)
                ctx.lineTo(16 * scale, y)
                ctx.stroke()
            }
        } else if (root.iconName === "history") {
            ctx.beginPath()
            ctx.moveTo(4.5 * scale, 14 * scale)
            ctx.lineTo(8 * scale, 10.5 * scale)
            ctx.lineTo(11 * scale, 12 * scale)
            ctx.lineTo(15.5 * scale, 7 * scale)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(4.5 * scale, 14 * scale, 1.0 * scale, 0, Math.PI * 2)
            ctx.arc(8 * scale, 10.5 * scale, 1.0 * scale, 0, Math.PI * 2)
            ctx.arc(11 * scale, 12 * scale, 1.0 * scale, 0, Math.PI * 2)
            ctx.arc(15.5 * scale, 7 * scale, 1.0 * scale, 0, Math.PI * 2)
            ctx.fill()
        } else if (root.iconName === "device") {
            ctx.strokeRect(5.5 * scale, 3.5 * scale, 9 * scale, 12 * scale)
            ctx.beginPath()
            ctx.moveTo(8 * scale, 7 * scale)
            ctx.lineTo(12 * scale, 7 * scale)
            ctx.moveTo(8 * scale, 10 * scale)
            ctx.lineTo(12 * scale, 10 * scale)
            ctx.moveTo(8 * scale, 13 * scale)
            ctx.lineTo(10.5 * scale, 13 * scale)
            ctx.stroke()
        } else if (root.iconName === "diagnostics") {
            ctx.beginPath()
            ctx.arc(10 * scale, 10 * scale, 5.5 * scale, 0, Math.PI * 2)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(10 * scale, 10 * scale)
            ctx.lineTo(13.5 * scale, 7.5 * scale)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(10 * scale, 10 * scale, 1.15 * scale, 0, Math.PI * 2)
            ctx.fill()
        } else if (root.iconName === "library") {
            ctx.strokeRect(4.5 * scale, 4.5 * scale, 11 * scale, 11 * scale)
            ctx.beginPath()
            ctx.moveTo(8 * scale, 4.5 * scale)
            ctx.lineTo(8 * scale, 15.5 * scale)
            ctx.moveTo(12 * scale, 4.5 * scale)
            ctx.lineTo(12 * scale, 15.5 * scale)
            ctx.stroke()
        } else if (root.iconName === "chevron-down") {
            ctx.beginPath()
            ctx.moveTo(6 * scale, 8 * scale)
            ctx.lineTo(10 * scale, 12 * scale)
            ctx.lineTo(14 * scale, 8 * scale)
            ctx.stroke()
        } else if (root.iconName === "chevron-up") {
            ctx.beginPath()
            ctx.moveTo(6 * scale, 12 * scale)
            ctx.lineTo(10 * scale, 8 * scale)
            ctx.lineTo(14 * scale, 12 * scale)
            ctx.stroke()
        } else if (root.iconName === "chevron-left") {
            ctx.beginPath()
            ctx.moveTo(12 * scale, 6 * scale)
            ctx.lineTo(8 * scale, 10 * scale)
            ctx.lineTo(12 * scale, 14 * scale)
            ctx.stroke()
        } else if (root.iconName === "chevron-right") {
            ctx.beginPath()
            ctx.moveTo(8 * scale, 6 * scale)
            ctx.lineTo(12 * scale, 10 * scale)
            ctx.lineTo(8 * scale, 14 * scale)
            ctx.stroke()
        } else if (root.iconName === "check") {
            ctx.beginPath()
            ctx.moveTo(5.5 * scale, 10.5 * scale)
            ctx.lineTo(8.5 * scale, 13.5 * scale)
            ctx.lineTo(14.5 * scale, 6.5 * scale)
            ctx.stroke()
        } else if (root.iconName === "close") {
            ctx.beginPath()
            ctx.moveTo(6 * scale, 6 * scale)
            ctx.lineTo(14 * scale, 14 * scale)
            ctx.moveTo(14 * scale, 6 * scale)
            ctx.lineTo(6 * scale, 14 * scale)
            ctx.stroke()
        } else {
            ctx.beginPath()
            ctx.arc(10 * scale, 10 * scale, 5 * scale, 0, Math.PI * 2)
            ctx.stroke()
        }
    }
}
