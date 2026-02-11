#ifndef CUSTOMAXIS_H
#define CUSTOMAXIS_H

#include <QGraphicsItem>
#include <QFont>
#include <QFontMetrics>

struct AxisTick
{
    double value;     // 实际数值
    bool isMajor;     // 是否主刻度（10^n）
};

class CustomAxis : public QObject
{
public:
    enum ScaleType { Linear, Logarithmic };


    explicit CustomAxis(QObject *parent = nullptr, double minVal = 0, double maxVal = 10000000.0);

    void setRange(double min, double max); // { m_minValue = min; m_maxValue = max;};
    void setTicks(int numTicks) { m_numTicks = numTicks; };
    void setScaleType(ScaleType type);
    void setAxisName(const QString &label) { m_title = label; };
    void setAlignment(Qt::Alignment alignment);
    void setTitleFont(const QFont &font);
    void setTickLabelFont(const QFont &font);

    double mapValueToRatio(double value) const;   // 0~1
    double mapRatioToValue(double ratio) const;
    bool isLog() const { return m_scaleType == Logarithmic; }


    Qt::Orientation orientation() const { return m_orientation; }
    Qt::Alignment alignment() const { return m_alignment; }
    double minValue() const { return m_minValue; }
    double maxValue() const { return m_maxValue; }
    int numTicks() const { return m_numTicks; }
    ScaleType scaleType() const { return m_scaleType; }
    QString axisName() const { return m_title; }
    QFont titleFont() const { return m_titleFont; }
    QFont tickLabelFont() const { return m_tickLabelFont; }
    double axisHeight() const { return m_axisHeight; }
    double axisTickHeight() const { return m_axisTickHeight; }
    double axisTitleHeight() const { return m_axisTitleHeight; }
    qreal range() const {return (m_maxValue - m_minValue); }

    QList<AxisTick> generateLogTicks() const;

private:
    static constexpr int        AXIS_TITLE_MARGIN = 10;
    static constexpr int        AXIS_TICK_MARGIN = 10;

    Qt::Orientation         m_orientation;
    Qt::Alignment           m_alignment;
    double                  m_minValue;
    double                  m_maxValue;
    int                     m_numTicks;
    ScaleType               m_scaleType;
    QString                 m_title;
    QMap<double, QString>   m_tickLabels;
    QFont                   m_tickLabelFont;
    QFont                   m_titleFont;
    int                     m_axisTitleHeight;
    int                     m_axisTickHeight;
    int                     m_axisHeight;

    double                     m_minLimit;
    double                     m_maxLimit;
};


#endif // CUSTOMAXIS_H
