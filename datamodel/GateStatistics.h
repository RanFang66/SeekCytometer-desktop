#ifndef GATESTATISTICS_H
#define GATESTATISTICS_H

#include <QString>
#include <cmath>

struct GateStatistics
{
    int count = 0;
    double meanX = 0.0;
    double meanY = 0.0;
    double stdDevX = 0.0;
    double stdDevY = 0.0;
    double cvX = 0.0;
    double cvY = 0.0;
    bool is1D = true; // true for IntervalGate (histogram), false for 2D gates

    QString meanString() const {
        if (count == 0) return "-";
        if (is1D) return QString::number(meanX, 'f', 2);
        return QString("(%1, %2)").arg(meanX, 0, 'f', 2).arg(meanY, 0, 'f', 2);
    }

    QString stdDevString() const {
        if (count == 0) return "-";
        if (is1D) return QString::number(stdDevX, 'f', 2);
        return QString("(%1, %2)").arg(stdDevX, 0, 'f', 2).arg(stdDevY, 0, 'f', 2);
    }

    QString cvString() const {
        if (count == 0) return "-";
        if (is1D) return QString("%1%").arg(cvX, 0, 'f', 2);
        return QString("(%1%, %2%)").arg(cvX, 0, 'f', 2).arg(cvY, 0, 'f', 2);
    }
};

#endif // GATESTATISTICS_H
