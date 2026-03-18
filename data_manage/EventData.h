#ifndef EVENTDATA_H
#define EVENTDATA_H


#include <QObject>
#include <QVector>
#include "MeasurementTypeHelper.h"

class EventData
{
public:
    EventData();
    explicit EventData(const QVector<int> &enabledChannels);
    explicit EventData(const QVector<int> &enabledChannels, const QByteArray &bytes);
    explicit EventData(const QVector<int> &enabledChannels, QDataStream &stream);
    int getWidth(int channelId) const;
    int getHeight(int channelId) const;
    int getArea(int channelId) const;
    int getData(int channelId, MeasurementType type) const;
    bool isEnabledSort() const;
    bool isRealSorted() const;
    quint8 validChPulse() const;
    bool isValidSpeedMeasure() const;
    bool isValidChPulse(int channelId) const;


    const QVector<int> &getEnabledChannels() const;


    bool isValidEvent() const;
    int getEventId() const;
    quint32 getDiffTimeUs() const;
    quint32 getPostTimeUs() const;

    void setEventId(int id);
    void setData(int channelId, MeasurementType type, int val);
    void setEnableSort(bool enabled);
    void setSorted(bool enabled);
    void setValidSpeedMeasure(bool enabled);
    void setDiffTimeUs(quint32 time);
    void setPostTimeUs(quint32 time);
    void setValidChPulse(quint8 val);

    static constexpr quint32 HEAD_MAGIC = 0x55AA55AA;
    static constexpr quint32 TAIL_MAGIC = 0xAA55AA55;
private:
    quint32                 eventId;
    quint32                 diffTimeUs;
    quint32                 postTimeUs;
    bool                    enableSorted;
    bool                    isSorted;
    bool                    isValidMeasure;
    bool                    isValidData;
    QVector<int>            enabledChannels;
    QVector<QVector<int>>   data;
    quint8                  chPulseValid;
    QHash<int, int>         channelIndexMap;
};

Q_DECLARE_METATYPE(EventData)
Q_DECLARE_METATYPE(QList<EventData>)
Q_DECLARE_METATYPE(QList<EventData>*)

inline int EventData::getHeight(int channelId) const
{
    return data[enabledChannels.indexOf(channelId)].at(static_cast<int>(MeasurementType::Height));
}


inline int EventData::getWidth(int channelId) const
{
    return data[enabledChannels.indexOf(channelId)].at(static_cast<int>(MeasurementType::Width));
}


inline int EventData::getArea(int channelId) const
{
    return data[enabledChannels.indexOf(channelId)].at(static_cast<int>(MeasurementType::Area));
}



inline bool EventData::isEnabledSort() const
{
    return enableSorted;
}

inline bool EventData::isRealSorted() const
{
    return isSorted;
}

inline quint8 EventData::validChPulse() const
{
    return chPulseValid;
}

inline bool EventData::isValidSpeedMeasure() const
{
    return isValidMeasure;
}

inline bool EventData::isValidChPulse(int channelId) const
{
    bool isValid = ((chPulseValid & (0x01 << channelId)) != 0);
    return isValid;
}

inline const QVector<int> &EventData::getEnabledChannels() const
{
    return enabledChannels;
}

inline int EventData::getEventId() const
{
    return eventId;
}

inline quint32 EventData::getDiffTimeUs() const
{
    return diffTimeUs;
}

inline quint32 EventData::getPostTimeUs() const
{
    return postTimeUs;
}

inline void EventData::setEventId(int id)
{
    eventId = id;
}



inline void EventData::setEnableSort(bool enabled)
{
    enableSorted = enabled;
}

inline void EventData::setSorted(bool enabled)
{
    isSorted = enabled;
}

inline void EventData::setValidSpeedMeasure(bool enabled)
{
    isValidMeasure = enabled;
}

inline void EventData::setDiffTimeUs(quint32 time)
{
    diffTimeUs = time;
}

inline void EventData::setPostTimeUs(quint32 time)
{
    postTimeUs = time;
}

inline void EventData::setValidChPulse(quint8 val)
{
    chPulseValid = val;
}

inline bool EventData::isValidEvent() const
{
    return isValidData;
}



#endif // EVENTDATA_H
