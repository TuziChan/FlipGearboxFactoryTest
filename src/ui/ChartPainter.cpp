#include "ChartPainter.h"
#include <QPainterPath>
#include <cmath>

ChartPainter::ChartPainter(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // Use Image render target to avoid GPU/FBO-related startup crashes on some systems.
    setRenderTarget(QQuickPaintedItem::Image);
    setAntialiasing(true);
}

void ChartPainter::paint(QPainter *painter)
{
    if (!painter)
        return;

    const qreal w = width();
    const qreal h = height();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(0, 0, w, h, m_backgroundColor);

    painter->setPen(QPen(m_gridColor, 1));
    for (int i = 0; i < 5; ++i) {
        const qreal y = i / 4.0 * (h - 20);
        painter->drawLine(QPointF(0, y), QPointF(w, y));
    }

    if (m_speedChannelOn)
        drawSeries(painter, m_speedData, 1600.0, QColor("#0078D4"));
    if (m_torqueChannelOn)
        drawSeries(painter, m_torqueData, 2.0, QColor("#E74856"));
    if (m_currentChannelOn)
        drawSeries(painter, m_currentData, 4.0, QColor("#FF8C00"));
    if (m_angleChannelOn)
        drawSeries(painter, m_angleData, 180.0, QColor("#16C60C"));
}

void ChartPainter::drawSeries(QPainter *painter, const QVariantList &values, double maxValue, const QColor &color)
{
    if (values.size() < 2)
        return;

    const qreal w = width();
    const qreal h = height();

    QPainterPath path;
    bool firstPoint = true;

    for (int i = 0; i < values.size(); ++i) {
        bool ok = false;
        const double val = values[i].toDouble(&ok);
        if (!ok)
            continue;

        const qreal x = i / static_cast<qreal>(std::max<qsizetype>(1, values.size() - 1)) * w;
        const qreal y = h - 20 - (val / maxValue) * (h - 40);

        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }

    painter->setPen(QPen(color, 2));
    painter->drawPath(path);
}

void ChartPainter::setSpeedData(const QVariantList &data)
{
    if (m_speedData == data)
        return;
    m_speedData = data;
    emit speedDataChanged();
    update();
}

void ChartPainter::setTorqueData(const QVariantList &data)
{
    if (m_torqueData == data)
        return;
    m_torqueData = data;
    emit torqueDataChanged();
    update();
}

void ChartPainter::setCurrentData(const QVariantList &data)
{
    if (m_currentData == data)
        return;
    m_currentData = data;
    emit currentDataChanged();
    update();
}

void ChartPainter::setAngleData(const QVariantList &data)
{
    if (m_angleData == data)
        return;
    m_angleData = data;
    emit angleDataChanged();
    update();
}

void ChartPainter::setSpeedChannelOn(bool on)
{
    if (m_speedChannelOn == on)
        return;
    m_speedChannelOn = on;
    emit speedChannelOnChanged();
    update();
}

void ChartPainter::setTorqueChannelOn(bool on)
{
    if (m_torqueChannelOn == on)
        return;
    m_torqueChannelOn = on;
    emit torqueChannelOnChanged();
    update();
}

void ChartPainter::setCurrentChannelOn(bool on)
{
    if (m_currentChannelOn == on)
        return;
    m_currentChannelOn = on;
    emit currentChannelOnChanged();
    update();
}

void ChartPainter::setAngleChannelOn(bool on)
{
    if (m_angleChannelOn == on)
        return;
    m_angleChannelOn = on;
    emit angleChannelOnChanged();
    update();
}

void ChartPainter::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color)
        return;
    m_backgroundColor = color;
    emit backgroundColorChanged();
    update();
}

void ChartPainter::setGridColor(const QColor &color)
{
    if (m_gridColor == color)
        return;
    m_gridColor = color;
    emit gridColorChanged();
    update();
}
