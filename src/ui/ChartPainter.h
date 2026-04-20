#pragma once

#include <QQuickPaintedItem>
#include <QVariantList>
#include <QPainter>
#include <QColor>

class ChartPainter : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QVariantList speedData READ speedData WRITE setSpeedData NOTIFY speedDataChanged)
    Q_PROPERTY(QVariantList torqueData READ torqueData WRITE setTorqueData NOTIFY torqueDataChanged)
    Q_PROPERTY(QVariantList currentData READ currentData WRITE setCurrentData NOTIFY currentDataChanged)
    Q_PROPERTY(QVariantList angleData READ angleData WRITE setAngleData NOTIFY angleDataChanged)
    Q_PROPERTY(bool speedChannelOn READ speedChannelOn WRITE setSpeedChannelOn NOTIFY speedChannelOnChanged)
    Q_PROPERTY(bool torqueChannelOn READ torqueChannelOn WRITE setTorqueChannelOn NOTIFY torqueChannelOnChanged)
    Q_PROPERTY(bool currentChannelOn READ currentChannelOn WRITE setCurrentChannelOn NOTIFY currentChannelOnChanged)
    Q_PROPERTY(bool angleChannelOn READ angleChannelOn WRITE setAngleChannelOn NOTIFY angleChannelOnChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)

public:
    explicit ChartPainter(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QVariantList speedData() const { return m_speedData; }
    void setSpeedData(const QVariantList &data);

    QVariantList torqueData() const { return m_torqueData; }
    void setTorqueData(const QVariantList &data);

    QVariantList currentData() const { return m_currentData; }
    void setCurrentData(const QVariantList &data);

    QVariantList angleData() const { return m_angleData; }
    void setAngleData(const QVariantList &data);

    bool speedChannelOn() const { return m_speedChannelOn; }
    void setSpeedChannelOn(bool on);

    bool torqueChannelOn() const { return m_torqueChannelOn; }
    void setTorqueChannelOn(bool on);

    bool currentChannelOn() const { return m_currentChannelOn; }
    void setCurrentChannelOn(bool on);

    bool angleChannelOn() const { return m_angleChannelOn; }
    void setAngleChannelOn(bool on);

    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);

    QColor gridColor() const { return m_gridColor; }
    void setGridColor(const QColor &color);

signals:
    void speedDataChanged();
    void torqueDataChanged();
    void currentDataChanged();
    void angleDataChanged();
    void speedChannelOnChanged();
    void torqueChannelOnChanged();
    void currentChannelOnChanged();
    void angleChannelOnChanged();
    void backgroundColorChanged();
    void gridColorChanged();

private:
    void drawSeries(QPainter *painter, const QVariantList &values, double maxValue, const QColor &color);

    QVariantList m_speedData;
    QVariantList m_torqueData;
    QVariantList m_currentData;
    QVariantList m_angleData;
    bool m_speedChannelOn = true;
    bool m_torqueChannelOn = true;
    bool m_currentChannelOn = true;
    bool m_angleChannelOn = true;
    QColor m_backgroundColor = QColor("#1E1E1E");
    QColor m_gridColor = QColor("#E8E8E8");
};
