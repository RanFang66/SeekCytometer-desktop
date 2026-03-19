#include "GatesModel.h"

GatesModel::GatesModel(QObject *parent)
    : QAbstractTableModel{parent}
{}


int GatesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_gateList.size();
}

int GatesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return GateColumn::GateColumnCount;
}

QVariant GatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Gate &gate = m_gateList.at(index.row());

    if (role == Qt::DecorationRole && index.column() == GateColumn::ColorColumn) {
        return gate.color();
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case GateColumn::GateIDColumn:
            return gate.id();
        case GateColumn::WorkSheetIdColumn:
            return gate.worksheetId();
        case GateColumn::GateNameColumn:
            return gate.name();
        case GateColumn::GateTypeColumn:
            return Gate::gateTypeToString(gate.gateType());
        case GateColumn::ColorColumn:
            return gate.color().name();
        case GateColumn::XAxisColumn:
            return gate.xAxisName();
        case GateColumn::YAxisColumn:
            return gate.yAxisName();
        case GateColumn::GatePointsColumn:
            return gate.pointsString();
        case GateColumn::CountColumn: {
            auto it = m_statistics.find(gate.id());
            if (it != m_statistics.end()) return it->count;
            return 0;
        }
        case GateColumn::MeanColumn: {
            auto it = m_statistics.find(gate.id());
            if (it != m_statistics.end()) return it->meanString();
            return "-";
        }
        case GateColumn::StdDevColumn: {
            auto it = m_statistics.find(gate.id());
            if (it != m_statistics.end()) return it->stdDevString();
            return "-";
        }
        case GateColumn::CVColumn: {
            auto it = m_statistics.find(gate.id());
            if (it != m_statistics.end()) return it->cvString();
            return "-";
        }
        default:
            return QVariant();
        }
    }
    return QVariant();
}


QVariant GatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return m_headerData.at(section);
        }
    }
    return QVariant();
}

void GatesModel::resetGateModel(int worksheetId)
{
    QMutexLocker locker(&m_mutex);
    beginResetModel();
    m_gateList = GatesDAO().fetchGates(worksheetId);
    m_statistics.clear();
    endResetModel();

    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

int GatesModel::addGate(const Gate &gate)
{
    QMutexLocker locker(&m_mutex);
    int newId = GatesDAO().insertGate(gate);

    if (newId > 0) {
        Gate insertedGate = gate;
        insertedGate.setId(newId);
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_gateList.append(insertedGate);
        endInsertRows();

        emit dataChanged(index(rowCount()-1, 0), index(rowCount()-1, columnCount()-1));
    }
    return newId;
}

bool GatesModel::removeGate(int row)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }
    QMutexLocker locker(&m_mutex);
    int gateId = m_gateList[row].id();
    if (GatesDAO().deleteGate(gateId)) {
        beginRemoveRows(QModelIndex(), row, row);
        m_gateList.removeAt(row);
        endRemoveRows();

        emit dataChanged(index(row, 0), index(row, columnCount()-1));
        return true;
    } else {
        qDebug() << QString("Delete gate in database failed.");
        return false;
    }
}

bool GatesModel::removeGate(const Gate &gate)
{
    int row = 0;
    for (row = 0; row < rowCount(); row++) {
        if (gate.id() == m_gateList[row].id()) {
            break;
        }
    }
    if (row == rowCount()) {
        qDebug() << QString("Remove gate failed! Gate(id = %1) is not found in current gates model.").arg(gate.id());
        return false;
    }

    QMutexLocker locker(&m_mutex);
    int gateId = m_gateList[row].id();
    if (GatesDAO().deleteGate(gateId)) {
        beginRemoveRows(QModelIndex(), row, row);
        m_gateList.removeAt(row);
        endRemoveRows();

        emit dataChanged(index(row, 0), index(row, columnCount()-1));
        return true;
    } else {
        qDebug() << QString("Delete gate in database failed.");
        return false;
    }

}

void GatesModel::updateGate(int row, const Gate &gate)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    if (GatesDAO().updateGate(gate)) {
        m_gateList[row] = gate;
        emit dataChanged(index(row, 0), index(row, columnCount()-1));
    }
}

const Gate GatesModel::getGate(int row) const
{
    if (row < 0 || row >= rowCount()) {
        return Gate();
    }
    return m_gateList[row];
}

const QList<Gate> &GatesModel::getGateList() const
{
    return m_gateList;
}

void GatesModel::updateGateStatistics(int gateId, const GateStatistics &stats)
{
    QMutexLocker locker(&m_mutex);
    m_statistics[gateId] = stats;

    // Find row for this gateId and emit dataChanged for statistics columns
    for (int row = 0; row < m_gateList.size(); ++row) {
        if (m_gateList[row].id() == gateId) {
            emit dataChanged(index(row, CountColumn), index(row, CVColumn));
            break;
        }
    }
}

