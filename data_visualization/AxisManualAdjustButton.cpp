#include "AxisManualAdjustButton.h"
#include <QCursor>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDoubleValidator>
#include <QGraphicsView>

AxisManualAdjustButton::AxisManualAdjustButton(PlotBase *plot)
    :QGraphicsObject(plot), m_plot(plot)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::PointingHandCursor);
}

QRectF AxisManualAdjustButton::boundingRect() const
{
    return QRectF(0, 0, 20, 20);
}

void AxisManualAdjustButton::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    p->setRenderHint(QPainter::Antialiasing);

    // 绘制一个简单的图标，可以后续替换为实际图标
    p->setPen(QPen(QColor(100, 100, 100), 2));
    p->setBrush(QBrush(QColor(200, 200, 200)));
    p->drawRect(boundingRect().adjusted(2, 2, -2, -2));

    // 绘制一个表示手动调整的符号（类似设置图标）
    p->setPen(QPen(QColor(50, 50, 50), 1.5));
    p->drawLine(6, 10, 14, 10);
    p->drawLine(10, 6, 10, 14);
}

void AxisManualAdjustButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    showAxisRangeDialog();
    event->accept();
}

void AxisManualAdjustButton::showAxisRangeDialog()
{
    // 创建对话框
    QDialog dialog;
    dialog.setWindowTitle("手动调整坐标轴范围");
    dialog.setMinimumWidth(300);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    // 创建输入框
    QLineEdit *xMinEdit = new QLineEdit();
    QLineEdit *xMaxEdit = new QLineEdit();
    QLineEdit *yMinEdit = new QLineEdit();
    QLineEdit *yMaxEdit = new QLineEdit();

    // 设置验证器（只允许输入数字）
    QDoubleValidator *validator = new QDoubleValidator();
    xMinEdit->setValidator(validator);
    xMaxEdit->setValidator(validator);
    yMinEdit->setValidator(validator);
    yMaxEdit->setValidator(validator);

    // 获取当前坐标轴范围并设置为默认值
    CustomAxis *xAxis = m_plot->xAxis();
    CustomAxis *yAxis = m_plot->yAxis();

    double xMin = xAxis->minValue();
    double xMax = xAxis->maxValue();
    double yMin = yAxis->minValue();
    double yMax = yAxis->maxValue();

    xMinEdit->setText(QString::number(xMin));
    xMaxEdit->setText(QString::number(xMax));
    yMinEdit->setText(QString::number(yMin));
    yMaxEdit->setText(QString::number(yMax));

    // 添加到表单布局
    formLayout->addRow("X轴最小值:", xMinEdit);
    formLayout->addRow("X轴最大值:", xMaxEdit);
    formLayout->addRow("Y轴最小值:", yMinEdit);
    formLayout->addRow("Y轴最大值:", yMaxEdit);

    mainLayout->addLayout(formLayout);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接按钮信号
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        // 获取输入值
        bool xMinOk, xMaxOk, yMinOk, yMaxOk;
        double newXMin = xMinEdit->text().toDouble(&xMinOk);
        double newXMax = xMaxEdit->text().toDouble(&xMaxOk);
        double newYMin = yMinEdit->text().toDouble(&yMinOk);
        double newYMax = yMaxEdit->text().toDouble(&yMaxOk);

        // 验证输入
        if (xMinOk && xMaxOk && yMinOk && yMaxOk) {
            if (newXMin < newXMax && newYMin < newYMax) {
                // 更新坐标轴范围
                m_plot->updateAxisRange(newXMin, newXMax, newYMin, newYMax);
                m_plot->update();
            }
        }
    }
}
