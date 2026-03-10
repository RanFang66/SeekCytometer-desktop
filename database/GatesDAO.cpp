#include "GatesDAO.h"
#include <QJsonDocument>
#include <QJsonArray>

GatesDAO::GatesDAO(QObject *parent)
    : BaseDAO{parent}
{}


int GatesDAO::insertGate(const Gate &gate)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Gates (worksheet_id, gate_name, gate_type, parent_population_id, x_axis_id, y_axis_id, x_mearsure_type, y_mearsure_type, gate_data) "
                  "VALUES (:worksheet_id, :gate_name, :gate_type, :parent_population_id, :x_axis_id, :y_axis_id, :x_mearsure_type, :y_mearsure_type, :gate_data) "
                  "RETURNING gate_id");
    query.bindValue(":worksheet_id", gate.worksheetId());
    query.bindValue(":gate_name", gate.name());
    query.bindValue(":gate_type", Gate::gateTypeToString(gate.gateType()));
    query.bindValue(":parent_population_id", gate.parentId() == 0 ? QVariant() : gate.parentId());
    query.bindValue(":x_axis_id", gate.xAxisSettingId());
    query.bindValue(":y_axis_id", gate.yAxisSettingId() <= 0 ? QVariant() : gate.yAxisSettingId());
    query.bindValue(":x_mearsure_type", MeasurementTypeHelper::measurementTypeToString(gate.xMeasurementType()));
    query.bindValue(":y_mearsure_type", gate.yMeasurementType() == MeasurementType::Unknown ? QVariant() : MeasurementTypeHelper::measurementTypeToString(gate.yMeasurementType()));
    query.bindValue(":gate_data", gate.pointsJsonString());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    } else {
        handleError(__FUNCTION__, query);
        return 0;
    }
}

bool GatesDAO::updateGate(const Gate &gate)
{
    QSqlQuery query;
    query.prepare("UPDATE Gates SET gate_name = :gate_name, gate_type = :gate_type, parent_population_id = :parent_population_id, "
                  "x_axis_id = :x_axis_id, y_axis_id = :y_axis_id, x_mearsure_type = :x_mearsure_type, y_mearsure_type = :y_mearsure_type, gate_data = :gate_data "
                  "WHERE gate_id = :gate_id");
    query.bindValue(":gate_name", gate.name());
    query.bindValue(":gate_type", Gate::gateTypeToString(gate.gateType()));
    query.bindValue(":parent_population_id", gate.parentId() == 0 ? QVariant() : gate.parentId());
    query.bindValue(":x_axis_id", gate.xAxisSettingId());
    query.bindValue(":y_axis_id", gate.yAxisSettingId() <= 0 ? QVariant() : gate.yAxisSettingId());
    query.bindValue(":x_mearsure_type", MeasurementTypeHelper::measurementTypeToString(gate.xMeasurementType()));
    query.bindValue(":y_mearsure_type", gate.yMeasurementType() == MeasurementType::Unknown ? QVariant() : MeasurementTypeHelper::measurementTypeToString(gate.yMeasurementType()));
    query.bindValue(":gate_data", gate.pointsJsonString());
    query.bindValue(":gate_id", gate.id());
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return false;
    }
    return true;
}

bool GatesDAO::deleteGate(int gateId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM Gates WHERE gate_id = :gate_id");
    query.bindValue(":gate_id", gateId);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return false;
    }
    return true;
}

QList<Gate> GatesDAO::fetchGates(int worksheetId) const
{
    QList<Gate> gates;
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE worksheet_id = :worksheet_id");
    query.bindValue(":worksheet_id", worksheetId);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return gates;
    }
    while (query.next()) {
        Gate gate;
        gate.setId(query.value("gate_id").toInt());
        gate.setWorksheetId(query.value("worksheet_id").toInt());
        gate.setName(query.value("gate_name").toString());
        gate.setGateType(Gate::stringToGateType(query.value("gate_type").toString()));
        gate.setParentId(query.value("parent_population_id").toInt());
        gate.setXAxisSettingId(query.value("x_axis_id").toInt());
        gate.setYAxisSettingId(query.value("y_axis_id").toInt());
        gate.setXMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("x_mearsure_type").toString()));
        gate.setYMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("y_mearsure_type").toString()));
        gate.setPoinst(QJsonDocument::fromJson(query.value("gate_data").toByteArray()).array());
        gates.append(gate);
    }
    return gates;
}

QList<Gate> GatesDAO::fetchGates(int worksheetId, int parentId) const
{
    QList<Gate> gates;
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE worksheet_id = :worksheet_id AND parent_population_id = :parent_population_id");
    query.bindValue(":worksheet_id", worksheetId);
    query.bindValue(":parent_population_id", parentId);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return gates;
    }
    while (query.next()) {
        Gate gate;
        gate.setId(query.value("gate_id").toInt());
        gate.setWorksheetId(query.value("worksheet_id").toInt());
        gate.setName(query.value("gate_name").toString());
        gate.setGateType(Gate::stringToGateType(query.value("gate_type").toString()));
        gate.setParentId(query.value("parent_population_id").toInt());
        gate.setXAxisSettingId(query.value("x_axis_id").toInt());
        gate.setYAxisSettingId(query.value("y_axis_id").toInt());
        gate.setXMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("x_mearsure_type").toString()));
        gate.setYMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("y_mearsure_type").toString()));
        gate.setPoinst(QJsonDocument::fromJson(query.value("gate_data").toByteArray()).array());
        gates.append(gate);
    }
    return gates;
}

QList<Gate> GatesDAO::fetchGates(int worksheetId, int xAxisId, MeasurementType xMeasurementType, int yAxisId, MeasurementType yMeasurementType) const
{
    QList<Gate> gates;
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE worksheet_id = :worksheet_id AND x_axis_id = :x_axis_id AND x_mearsure_type = :x_mearsure_type AND y_axis_id = :y_axis_id AND y_mearsure_type = :y_mearsure_type");
    query.bindValue(":worksheet_id", worksheetId);
    query.bindValue(":x_axis_id", xAxisId);
    query.bindValue(":x_mearsure_type", MeasurementTypeHelper::measurementTypeToString(xMeasurementType));
    query.bindValue(":y_axis_id", yAxisId);
    query.bindValue(":y_mearsure_type", MeasurementTypeHelper::measurementTypeToString(yMeasurementType));
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return gates;
    }
    while (query.next()) {
        Gate gate;
        gate.setId(query.value("gate_id").toInt());
        gate.setWorksheetId(query.value("worksheet_id").toInt());
        gate.setName(query.value("gate_name").toString());
        gate.setGateType(Gate::stringToGateType(query.value("gate_type").toString()));
        gate.setParentId(query.value("parent_population_id").toInt());
        gate.setXAxisSettingId(query.value("x_axis_id").toInt());
        gate.setYAxisSettingId(query.value("y_axis_id").toInt());
        gate.setXMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("x_mearsure_type").toString()));
        gate.setYMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("y_mearsure_type").toString()));
        gate.setPoinst(QJsonDocument::fromJson(query.value("gate_data").toByteArray()).array());
        gates.append(gate);
    }
    return gates;
}

Gate GatesDAO::fetchGate(int gateId) const
{
    Gate gate;
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE gate_id = :gate_id");
    query.bindValue(":gate_id", gateId);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return gate;
    }
    if (query.next()) {
        gate.setId(query.value("gate_id").toInt());
        gate.setWorksheetId(query.value("worksheet_id").toInt());
        gate.setName(query.value("gate_name").toString());
        gate.setGateType(Gate::stringToGateType(query.value("gate_type").toString()));
        gate.setParentId(query.value("parent_population_id").toInt());
        gate.setXAxisSettingId(query.value("x_axis_id").toInt());
        gate.setYAxisSettingId(query.value("y_axis_id").toInt());
        gate.setXMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("x_mearsure_type").toString()));
        gate.setYMeasurementType(MeasurementTypeHelper::stringToMeasurementType(query.value("y_mearsure_type").toString()));
        gate.setPoinst(QJsonDocument::fromJson(query.value("gate_data").toByteArray()).array());
    }
    return gate;
}

bool GatesDAO::isGateExists(int gateId) const
{
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE gate_id = :gate_id");
    query.bindValue(":gate_id", gateId);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return false;
    }
    return query.next();
}

bool GatesDAO::isGateExists(int worksheetId, const QString &name) const
{
    QSqlQuery query;
    query.prepare("SELECT * FROM Gates WHERE worksheet_id = :worksheet_id AND gate_name = :gate_name");
    query.bindValue(":worksheet_id", worksheetId);
    query.bindValue(":gate_name", name);
    if (!query.exec()) {
        handleError(__FUNCTION__, query);
        return false;
    }
    return query.next();
}









