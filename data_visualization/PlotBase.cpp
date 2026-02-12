#include "PlotBase.h"
#include <QPainter>
#include <QMarginsF>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QImage>
#include <QApplication>


#include "GateItem.h"
#include "WorkSheetScene.h"

#include "AxisLockButtonItem.h"
#include "SaveImageButtonItem.h"
#include "AxisAutoAdjustButton.h"
#include "AxiTypeSwitchButton.h"
#include "AxisManualAdjustButton.h"

PlotBase::PlotBase(const Plot &plot, QGraphicsItem *parent)
    : QGraphicsObject{parent}, m_plot{plot}, m_axisUnlocked(false)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | ItemClipsToShape);

    m_xAxis = new CustomAxis(this);
    m_yAxis = new CustomAxis(this);

    m_xAxis->setAlignment(Qt::AlignBottom);
    m_yAxis->setAlignment(Qt::AlignLeft);
    m_xAxis->setRange(0, 100);
    m_yAxis->setRange(0, 100);
    m_xAxis->setAxisName(m_plot.axisXName());
    m_yAxis->setAxisName(m_plot.axisYName());
    m_title = m_plot.plotName();
    m_titleFont = QFont("Arial", 12);

    m_boundingRect = QRectF(0, 0, 480, 480);

    AxisLockButtonItem *axisLockButton = new AxisLockButtonItem(this);
    axisLockButton->setPos(m_boundingRect.right()-25, m_boundingRect.top() + 5);

    SaveImageButtonItem *saveButton = new SaveImageButtonItem(this);
    saveButton->setPos(m_boundingRect.right()- 75, m_boundingRect.top() + 5);

    AxisAutoAdjustButton *autoButton = new AxisAutoAdjustButton(this);
    autoButton->setPos(m_boundingRect.right() - 50, m_boundingRect.top() + 5);

    AxisManualAdjustButton *manualButton = new AxisManualAdjustButton(this);
    manualButton->setPos(m_boundingRect.right() - 125, m_boundingRect.top() + 5);

    AxiTypeSwitchButton *axisTypeButton = new AxiTypeSwitchButton(this);
    axisTypeButton->setPos(m_boundingRect.right() - 100, m_boundingRect.top() + 5);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);

    updateLayout();
}

void PlotBase::setBoundingRect(const QRectF &rect)
{
    if (m_boundingRect != rect) {
        m_boundingRect = rect;
        updateLayout();
        update();
    }
}

void PlotBase::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        m_plot.setName(title);
        update();
    }
}

void PlotBase::setAxisXName(const QString &name)
{
    m_xAxis->setAxisName(name);
}

void PlotBase::setAxisYName(const QString &name)
{
    m_yAxis->setAxisName(name);
}



void PlotBase::updateLayout()
{
    QRectF validArea = m_boundingRect.marginsRemoved(QMarginsF(PLOT_MARGIN, PLOT_MARGIN, PLOT_MARGIN + 10, PLOT_MARGIN));

    QFontMetrics fm(m_titleFont);
    qreal titleHeight = fm.height() + TITLE_MARGIN;

    m_titleArea = QRectF(validArea.left(), validArea.top(), validArea.width(), titleHeight);

    m_plotArea = validArea;
    m_plotArea.setTop(m_titleArea.bottom());
    m_plotArea.setLeft(validArea.left() + m_yAxis->axisHeight());
    m_plotArea.setBottom(validArea.bottom() - m_xAxis->axisHeight());

    m_axisXArea = QRectF(m_plotArea.left(), m_plotArea.bottom(), m_plotArea.width(), m_xAxis->axisHeight());
    m_axisYArea = QRectF(validArea.left(), m_plotArea.top(), m_yAxis->axisHeight(), m_plotArea.height());

    qDebug() << "Title Area: " << m_titleArea;
    qDebug() << "Plot Area: " << m_plotArea;
    qDebug() << "Axis X Area: " << m_axisXArea;
    qDebug() << "Axis Y Area: " << m_axisYArea;
}

void PlotBase::paintTitle(QPainter *painter)
{
    if (!painter || m_title.isEmpty()) {
        return;
    }

    painter->save();
    painter->setFont(m_titleFont);
    painter->setPen(Qt::black);
    painter->drawText(m_titleArea, Qt::AlignCenter, m_title);
    painter->restore();
}

void PlotBase::paintAxis(QPainter *painter)
{
    if (!painter) {
        return;
    }

    painter->save();
    painter->setPen(Qt::black);
    painter->drawRect(m_plotArea);


    // Draw X Axis
    // ================= X Axis =================
    if (m_xAxis->scaleType() == CustomAxis::Logarithmic) {

        QList<AxisTick> ticks = m_xAxis->generateLogTicks();

        for (const AxisTick &tick : std::as_const(ticks)) {
            double ratio = m_xAxis->mapValueToRatio(tick.value);
            qreal x = m_plotArea.left() + ratio * m_plotArea.width();

            if (tick.isMajor) {
                painter->drawLine(
                    QPointF(x, m_axisXArea.top()),
                    QPointF(x, m_axisXArea.top() + 10)
                    );

                QString label = QString("10^%1")
                                    .arg(int(std::round(std::log10(tick.value))));

                painter->drawText(
                    QRectF(x - 30, m_axisXArea.top() + 12, 60, 20),
                    Qt::AlignCenter,
                    label
                    );
            } else {
                painter->drawLine(
                    QPointF(x, m_axisXArea.top()),
                    QPointF(x, m_axisXArea.top() + 5)
                    );
            }
        }

    } else {
        int xTickNum = m_xAxis->numTicks();
        if (xTickNum <= 0) xTickNum = 5;

        for (int i = 0; i <= xTickNum; ++i) {
            double ratio = double(i) / xTickNum;
            double val = m_xAxis->mapRatioToValue(ratio);
            qreal x = m_plotArea.left() + ratio * m_plotArea.width();

            painter->drawLine(
                QPointF(x, m_axisXArea.top()),
                QPointF(x, m_axisXArea.top() + 8)
                );

            painter->drawText(
                QRectF(x - 25, m_axisXArea.top() + 8, 50, 20),
                Qt::AlignCenter,
                QString::number(val, 'f', 0)
                );
        }
    }

    // Draw Y Axis
    if (m_yAxis->scaleType() == CustomAxis::Logarithmic) {

        QList<AxisTick> ticks = m_yAxis->generateLogTicks();

        for (const AxisTick &tick : std::as_const(ticks)) {
            double ratio = m_yAxis->mapValueToRatio(tick.value);
            qreal y = m_plotArea.bottom() - ratio * m_plotArea.height();

            if (tick.isMajor) {
                // 主刻度
                painter->drawLine(
                    QPointF(m_axisYArea.right(), y),
                    QPointF(m_axisYArea.right() - 10, y)
                    );

                QString label = QString("10^%1")
                                    .arg(int(std::round(std::log10(tick.value))));

                painter->drawText(
                    QRectF(m_axisYArea.right() - 70, y - 10, 60, 20),
                    Qt::AlignRight | Qt::AlignVCenter,
                    label
                    );
            } else {
                // 次刻度
                painter->drawLine(
                    QPointF(m_axisYArea.right(), y),
                    QPointF(m_axisYArea.right() - 5, y)
                    );
            }
        }

    } else {
        // ========= 线性轴 =========
        int yTickNum = m_yAxis->numTicks();
        if (yTickNum <= 0) yTickNum = 5;

        for (int i = 0; i <= yTickNum; ++i) {
            double ratio = double(i) / yTickNum;
            double val = m_yAxis->mapRatioToValue(ratio);
            qreal y = m_plotArea.bottom() - ratio * m_plotArea.height();

            painter->drawLine(
                QPointF(m_axisYArea.right(), y),
                QPointF(m_axisYArea.right() - 8, y)
                );

            painter->drawText(
                QRectF(m_axisYArea.right() - 60, y - 10, 60, 20),
                Qt::AlignCenter,
                QString::number(val, 'f', 0)
                );
        }
    }

    // Draw Y Axis Title At the Left of the Axis in Vertical Direction
    painter->save();
    painter->rotate(-90);
    painter->drawText(QRectF(-m_axisYArea.bottom(), m_axisYArea.left()-10, m_axisYArea.height(), 40), Qt::AlignCenter, m_yAxis->axisName());
    painter->restore();

    painter->restore();
}

void PlotBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    WorkSheetScene *workSheetScene = dynamic_cast<WorkSheetScene*>((scene()));
    if (!workSheetScene)
        return;

    QAction *deleteAction = menu.addAction(QString("Delete Plot %1").arg(title()));
    QObject::connect(deleteAction, &QAction::triggered, workSheetScene, [this, sc=workSheetScene]() {
            QMetaObject::invokeMethod(sc, "onDeletePlot", Qt::QueuedConnection,
                                      Q_ARG(PlotBase*, this));
    });

    for (QGraphicsItem *gate : childItems()) {
        GateItem *item = dynamic_cast<GateItem*>(gate);
        if (!item)
            continue;
        QAction *gateDeleteAction = menu.addAction(QString("Delete Gate %1").arg(item->getGateName()));
        QObject::connect(gateDeleteAction, &QAction::triggered, workSheetScene, [item, sc=workSheetScene]() {
            QMetaObject::invokeMethod(sc, "onDeleteGate", Qt::QueuedConnection,
                                      Q_ARG(GateItem*, item));
        });
    }

    QAction *selectedAction = menu.exec(event->screenPos());

    event->accept();
}

qreal PlotBase::mapValueToXAixs(qreal value) const
{
    double ratio = m_xAxis->mapValueToRatio(value);
    return m_plotArea.left() + ratio * m_plotArea.width();
}

qreal PlotBase::mapValueToYAixs(qreal value) const
{
    double ratio = m_yAxis->mapValueToRatio(value);
    return m_plotArea.bottom() - ratio * m_plotArea.height();
}

qreal PlotBase::mapXAxisToValue(qreal x) const
{
    double ratio = (x - m_plotArea.left()) / m_plotArea.width();
    return m_xAxis->mapRatioToValue(ratio);
}


qreal PlotBase::mapYAxisToValue(qreal y) const
{
    double ratio = (m_plotArea.bottom() - y) / m_plotArea.height();
    return m_yAxis->mapRatioToValue(ratio);
}

QPointF PlotBase::mapPointToPlotArea(const QPointF &point) const
{

    return QPointF(mapValueToXAixs(point.x()), mapValueToYAixs(point.y()));
    // QPointF mappedPoint;
    // mappedPoint.setX(m_plotArea.left() + (point.x() - m_xAxis->minValue()) * (m_plotArea.width() / m_xAxis->range()));
    // mappedPoint.setY(m_plotArea.bottom() - (point.y() - m_yAxis->minValue()) * (m_plotArea.height() / m_yAxis->range()));
    // return mappedPoint;
}



QPointF PlotBase::mapPointToPlotArea(qreal x, qreal y) const
{
    return QPointF(mapValueToXAixs(x), mapValueToYAixs(y));
    // return mapPointToPlotArea(QPointF(x, y));
}

QPointF PlotBase::mapPlotAreaToPoint(const QPointF &point) const
{
    return QPointF(mapXAxisToValue(point.x()), mapYAxisToValue(point.y()));
    // QPointF mappedPoint;
    // mappedPoint.setX(m_xAxis->minValue() + (point.x() - m_plotArea.left()) * (m_xAxis->range() / m_plotArea.width()));
    // mappedPoint.setY(m_yAxis->minValue() + (m_plotArea.bottom() - point.y()) * (m_yAxis->range() / m_plotArea.height()));
    // return mappedPoint;
}

QRectF PlotBase::mapRectToPlotArea(const QRectF &rect) const
{
    QPointF topLeft = mapPointToPlotArea(rect.topLeft());
    QPointF bottomRight = mapPointToPlotArea(rect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

QRectF PlotBase::mapRectToAxis(const QRectF &rect) const
{
    QPointF topLeft = mapPlotAreaToPoint(rect.topLeft());
    QPointF bottomRight = mapPlotAreaToPoint(rect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

bool PlotBase::isInPlotArea(const QPointF &point) const
{
    return m_plotArea.contains(point);
}

QPointF PlotBase::limitPointInPlot(const QPointF &point) const
{
    QPointF limitedPoint = point;
    if (point.x() < m_plotArea.left()) {
        limitedPoint.setX(m_plotArea.left());
    } else if (point.x() > m_plotArea.right()) {
        limitedPoint.setX(m_plotArea.right());
    }

    if (point.y() < m_plotArea.top()) {
        limitedPoint.setY(m_plotArea.top());
    } else if (point.y() > m_plotArea.bottom()) {
        limitedPoint.setY(m_plotArea.bottom());
    }

    return limitedPoint;
}

QPointF PlotBase::limitScenePointInPlot(const QPointF &pointInScene) const
{
    QPointF pointInPlot = mapFromScene(pointInScene);
    QPointF limitedPoint =  limitPointInPlot(pointInPlot);
    return mapToScene(limitedPoint);
}


void PlotBase::updateAxisRange(int xMin, int xMax, int yMin, int yMax)
{
    int xMinVal = m_xAxis->minValue() > xMin ? xMin : m_xAxis->minValue();
    int xMaxVal = m_xAxis->maxValue() < xMax ? xMax : m_xAxis->maxValue();
    int yMinVal = m_yAxis->minValue() > yMin ? yMin : m_yAxis->minValue();
    int yMaxVal = m_yAxis->maxValue() < yMax ? yMax : m_yAxis->maxValue();

    m_xAxis->setRange(xMinVal, xMaxVal);
    m_yAxis->setRange(yMinVal, yMaxVal);
}

void PlotBase::updateAxisRanges(const Gate &gate)
{
    QList<QPoint> points = gate.points();

    int minX = m_xAxis->minValue();
    int maxX = m_xAxis->maxValue();
    int minY = m_yAxis->minValue();
    int maxY = m_yAxis->maxValue();


    for (const QPoint& point : points) {
        if (point.x() < minX) minX = point.x();
        if (point.x() > maxX) maxX = point.x();
        if (point.y() < minY) minY = point.y();
        if (point.y() > maxY) maxY = point.y();
    }
    m_xAxis->setRange(minX, maxX);
    m_yAxis->setRange(minY, maxY);
}

void PlotBase::setAxisUnlocked(bool unlocked)
{
    if (m_axisUnlocked == unlocked)
        return;

    m_axisUnlocked = unlocked;
    update();
}

bool PlotBase::isAxisUnlocked() const
{
    return m_axisUnlocked;
}

void PlotBase::saveToImage()
{
    QString defaultName;
    if (m_title.isEmpty()) {
        defaultName = QString("plot-%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    } else {
        defaultName = QString("%1-%2.png").arg(m_title, QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        tr("Save Plot As Image"),
        defaultName,
        tr("PNG Image (*.png);; JPEG Image (*.jpg)")
        );

    if (fileName.isEmpty()) {
        return;
    }

    qreal dpr = qApp->devicePixelRatio();
    QSize imageSize = m_boundingRect.size().toSize() * dpr;
    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.translate(-m_boundingRect.topLeft());
    this->paint(&painter, nullptr, nullptr);

    painter.end();
    image.save(fileName);
}


QRectF PlotBase::boundingRect() const
{
    return m_boundingRect;
}

void PlotBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!painter) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    // Draw the bounding rect
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawRect(m_boundingRect);
    painter->restore();

    paintTitle(painter);
    paintAxis(painter);
    paintPlot(painter);
    drawCursorValue(painter);
}

void PlotBase::drawCursorValue(QPainter *painter)
{
    if (!m_showCursorValue)
        return;

    const int margin = 6;
    const int padding = 4;

    QString text = QString("X = %1\nY = %2")
                       .arg(m_cursorValue.x(), 0, 'f', 2)
                       .arg(m_cursorValue.y(), 0, 'f', 2);

    QFontMetrics fm(painter->font());
    QRect textRect = fm.boundingRect(QRect(0, 0, 200, 50),
                                     Qt::AlignLeft | Qt::AlignTop,
                                     text);

    QRectF bgRect(
        m_boundingRect.left() + margin,
        m_boundingRect.bottom() - textRect.height() - margin - 10,
        textRect.width() + padding * 2,
        textRect.height() + padding * 2
        );

    // 背景
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 120));
    painter->drawRect(bgRect);

    // 文本
    painter->setPen(Qt::white);
    painter->drawText(
        bgRect.adjusted(padding, padding, -padding, -padding),
        Qt::AlignLeft | Qt::AlignTop,
        text
        );
}












// void PlotBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         if (m_state == DrawingState::DrawingIdle && isDrawing()) {
//             m_state = DrawingState::DrawingStarted;
//             m_gateType = qobject_cast<WorkSheetScene*>(scene())->gateType();
//             Gate gate = Gate(m_plot.workSheetId(), "", m_gateType, m_plot.axisXId(), m_plot.xMeasurementType(), m_plot.axisYId(), m_plot.yMeasurementType());
//             m_gateItem = new RectangleGateItem(gate, this);
//             m_startPos = event->pos();
//             m_gateItem->setPos(m_startPos);
//             scene()->addItem(m_gateItem);
//             qDebug() << "Start Drawing Gate" << Gate::gateTypeToString(m_gateType) << "at pos" << m_startPos;
//         } else if (m_state == DrawingState::DrawingStarted) {
//             m_state = DrawingState::DrawingFinished;
//             qDebug() << "End Drawing Gate" << Gate::gateTypeToString(m_gateType) << "at pos" << event->pos();
//         }
//     }

//     QGraphicsObject::mousePressEvent(event);
// }

// void PlotBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
// {
//     if (m_state == DrawingState::DrawingStarted) {
//         if (m_gateType == GateType::RectangleGate) {
//             QPointF pos = event->pos();
//             qreal w = pos.x() - m_startPos.x();
//             qreal h = pos.y() - m_startPos.y();
//             RectangleGateItem *rectGate = qobject_cast<RectangleGateItem*>(m_gateItem);
//             if (rectGate) {
//                 rectGate->updateRectangle(w, h);
//             }
//         }
//     }
//     QGraphicsObject::mouseMoveEvent(event);
// }

// // void PlotBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
// // {
//     // if (event->button() == Qt::LeftButton && m_state == DrawingState::DrawingStarted) {
//     //     if (m_gateType == GateType::RectangleGate) {
//     //         RectangleGateItem *rectGate = qobject_cast<RectangleGateItem*>(m_gateItem);
//     //         if (rectGate) {
//     //             QRectF rect = rectGate->boundingRect();
//     //         }

//     //         m_state = DrawingState::DrawingFinished;
//     //         qDebug() << "End Drawing Gate" << Gate::gateTypeToString(m_gateType) << "at pos" << event->pos();
//     //     }
//     // }
//     // QGraphicsObject::mouseReleaseEvent(event);
// // }

// bool PlotBase::isDrawing() const
// {
//     WorkSheetScene *scene = qobject_cast<WorkSheetScene*>(this->scene());
//     if (scene) {
//         return scene->isDrawingGate();
//     }
//     return false;
// }

