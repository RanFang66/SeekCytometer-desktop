#ifndef GATE_H
#define GATE_H

#include <QString>
#include <QPointF>
#include <QList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QColor>
#include "MeasurementTypeHelper.h"
#include "DetectorSettings.h"
enum class GateType
{
    RectangleGate,
    PolygonGate,
    EllipseGate,
    IntervalGate,
    QuadrantGate,
    UnknownGate,
};

class Gate
{
public:
    Gate();
    explicit Gate(int worksheetId, QString name, GateType type,  int xAxisSettingId,
                  MeasurementType xMeasurementType, int yAxisSettingId, MeasurementType yMeasurementType, const QList<QPoint> &points = QList<QPoint>(), int parentId = 0);

    static GateType stringToGateType(const QString &str);
    static QString  gateTypeToString(GateType type);

    int id() const;
    int worksheetId() const;
    QString name() const;
    GateType gateType() const;
    const QList<QPoint> &points() const;
    QJsonArray pointsJsonArray() const;
    QString     pointsJsonString() const;
    QString     pointsString() const;


    void getGateRange(int& minX, int& maxX, int& minY, int& maxY) const;


    int xAxisSettingId() const;
    int yAxisSettingId() const;

    int xAxisDetectorId() const;
    int yAxisDetectorId() const;

    QString xAxisName() const;
    QString yAxisName() const;

    const DetectorSettings &xAxisSettings() const;
    const DetectorSettings &yAxisSettings() const;


    MeasurementType xMeasurementType() const;
    MeasurementType yMeasurementType() const;
    int parentId() const;
    QColor color() const;

    static QColor defaultGateColor(int index);

    void setId(int id);
    void setWorksheetId(int worksheetId);
    void setName(const QString &name);
    void setPoints(const QList<QPointF> &points);
    void setPoinst(const QJsonArray &points);
    void setXAxisSettingId(int xAxisSettingId);
    void setYAxisSettingId(int yAxisSettingId);
    void setParentId(int parentId);
    void setXMeasurementType(MeasurementType type);
    void setYMeasurementType(MeasurementType type);
    void setGateType(GateType type);
    void setColor(const QColor &color);


private:
    int                 m_id;
    int                 m_worksheetId;
    QString             m_name;
    GateType            m_type;
    QList<QPoint>       m_points;
    int                 m_xAxisSettingId;
    int                 m_yAxisSettingId;
    MeasurementType     m_xMeasurementType;
    MeasurementType     m_yMeasurementType;
    int                 m_parentId;
    QColor              m_color;

    DetectorSettings    m_xAxisSetting;
    DetectorSettings    m_yAxisSetting;
};


inline int Gate::id() const
{
    return m_id;
}


inline int Gate::worksheetId() const
{
    return m_worksheetId;
}

inline QString Gate::name() const
{
    return m_name;
}

inline GateType Gate::gateType() const
{
    return m_type;
}

inline const QList<QPoint> &Gate::points() const
{
    return m_points;
}

inline int Gate::xAxisSettingId() const
{
    return m_xAxisSettingId;
}

inline int Gate::yAxisSettingId() const
{
    return m_yAxisSettingId;
}

inline int Gate::xAxisDetectorId() const
{
    return m_xAxisSetting.detectorId();
}

inline int Gate::yAxisDetectorId() const
{
    return m_yAxisSetting.detectorId();
}


inline QString Gate::xAxisName() const
{
    return MeasurementTypeHelper::parameterMeasurementType(m_xAxisSetting.parameterName(), m_xMeasurementType);;
}

inline QString Gate::yAxisName() const
{
    if (m_xAxisSettingId != 0) {
        return MeasurementTypeHelper::parameterMeasurementType(m_yAxisSetting.parameterName(), m_yMeasurementType);
    } else {
        return "Count";
    }
}

inline const DetectorSettings &Gate::xAxisSettings() const
{
    return m_xAxisSetting;
}

inline const DetectorSettings &Gate::yAxisSettings() const
{
    return m_yAxisSetting;
}

inline MeasurementType Gate::xMeasurementType() const
{
    return m_xMeasurementType;
}

inline MeasurementType Gate::yMeasurementType() const
{
    return m_yMeasurementType;
}

inline int Gate::parentId() const
{
    return m_parentId;
}

inline void Gate::setId(int id)
{
    m_id = id;
}

inline void Gate::setWorksheetId(int worksheetId)
{
    m_worksheetId = worksheetId;
}

inline void Gate::setName(const QString &name)
{
    m_name = name;
}






inline void Gate::setParentId(int parentId)
{
    m_parentId = parentId;
}

inline void Gate::setXMeasurementType(MeasurementType type)
{
    m_xMeasurementType = type;
}

inline void Gate::setYMeasurementType(MeasurementType type)
{
    m_yMeasurementType = type;
}

inline void Gate::setGateType(GateType type)
{
    m_type = type;
}

inline QColor Gate::color() const
{
    return m_color;
}

inline void Gate::setColor(const QColor &color)
{
    m_color = color;
}

inline QColor Gate::defaultGateColor(int index)
{
    static const QColor palette[] = {
        QColor(0x1f, 0x77, 0xb4), // blue
        QColor(0xff, 0x7f, 0x0e), // orange
        QColor(0x2c, 0xa0, 0x2c), // green
        QColor(0xd6, 0x27, 0x28), // red
        QColor(0x94, 0x67, 0xbd), // purple
        QColor(0x8c, 0x56, 0x4b), // brown
        QColor(0xe3, 0x77, 0xc2), // pink
        QColor(0xbc, 0xbd, 0x22), // olive
        QColor(0x17, 0xbe, 0xcf), // cyan
        QColor(0x7f, 0x7f, 0x7f), // gray
    };
    static const int paletteSize = sizeof(palette) / sizeof(palette[0]);
    return palette[index % paletteSize];
}


#endif // GATE_H
