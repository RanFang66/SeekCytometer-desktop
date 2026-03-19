#include "ExperimentsBrowser.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QTreeView>

#include "ExperimentsDAO.h"
#include "SpecimensDAO.h"
#include "TubesDAO.h"
#include "UsersDAO.h"
#include "CytometerSettingsDAO.h"
#include "WorkSheetsDAO.h"
#include "AddNewItemDialog.h"

#include <QActionGroup>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QMessageBox>


ExperimentsBrowser::ExperimentsBrowser(const QString &title, QWidget *parent)
    : QDockWidget{title, parent}
{
    initDockWidget();
}

ExperimentsBrowser::~ExperimentsBrowser()
{
    deleteLater();
}

void ExperimentsBrowser::initDockWidget()
{
    qRegisterMetaType<NodeType>("NodeType");

    m_browserView = new BrowserView(this);


    // m_model = new BrowserDataModel(this);
    // m_theSelection = new QItemSelectionModel(m_model);

    treeView = m_browserView->treeView();
    m_model = m_browserView->browserModel();
    m_theSelection = m_browserView->selectionModel();

    QWidget *mainWidget = new QWidget();

    QToolBar *toolBar = new QToolBar("ToolBar", mainWidget);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(20, 20));
    QActionGroup *addNodeGroup = new QActionGroup(toolBar);

    addExperiment = new QAction(QIcon(":/resource/images/icons/new_experiment.svg"), "", mainWidget);
    addExperiment->setToolTip(tr("New Experiment"));

    addSpecimen = new QAction(QIcon(":/resource/images/icons/new_specimen.svg"), "", mainWidget);
    addSpecimen->setToolTip(tr("New Specimen"));

    addTube = new QAction(QIcon(":/resource/images/icons/new_tube.svg"), "", mainWidget);
    addTube->setToolTip(tr("New Tube"));

    addSettings = new QAction(QIcon(":/resource/images/icons/new_settings.svg"), "", mainWidget);
    addSettings->setToolTip(tr("New Cytometer Settings"));

    addWorkSheet = new QAction(QIcon(":/resource/images/icons/new_worksheet.svg"), "", mainWidget);
    addWorkSheet->setToolTip(tr("New WorkSheet"));

    expandAll = new QAction(QIcon(":/resource/images/icons/expand_all.svg"), "", mainWidget);
    expandAll->setToolTip(tr("Expand All"));

    collapseAll = new QAction(QIcon(":/resource/images/icons/collapse_all.svg"), "", mainWidget);
    collapseAll->setToolTip(tr("Collapse All"));

    addExperiment->setData(QVariant::fromValue(NodeType::Experiment));
    addSpecimen->setData(QVariant::fromValue(NodeType::Specimen));
    addTube->setData(QVariant::fromValue(NodeType::Tube));
    addSettings->setData(QVariant::fromValue(NodeType::Settings));
    addWorkSheet->setData(QVariant::fromValue(NodeType::Worksheet));

    addNodeGroup->addAction(addExperiment);
    addNodeGroup->addAction(addSpecimen);
    addNodeGroup->addAction(addTube);
    addNodeGroup->addAction(addSettings);
    addNodeGroup->addAction(addWorkSheet);


    toolBar->addAction(addExperiment);
    toolBar->addAction(addSpecimen);
    toolBar->addAction(addTube);
    toolBar->addAction(addSettings);
    toolBar->addAction(addWorkSheet);
    toolBar->insertSeparator(addWorkSheet);
    toolBar->addAction(expandAll);
    toolBar->addAction(collapseAll);
    statusBar = new QStatusBar(mainWidget);
    statusBar->showMessage("Status: IDLE");

    // treeView = new QTreeView(mainWidget);
    // treeView->setModel(m_model);
    // treeView->setSelectionModel(m_theSelection);
    // treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    // treeView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_LoginUserIndex = m_model->findIndex(NodeType::User, User::loginUser().name());


    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->addWidget(toolBar);
    layout->addWidget(m_browserView);
    layout->addWidget(statusBar);

    // treeView->setWindowTitle("Experiment Data");
    // treeView->resize(800, 1200);
    // treeView->show();
    // treeView->setHeaderHidden(false);

    mainWidget->setLayout(layout);
    setWidget(mainWidget);

    connect(m_theSelection, &QItemSelectionModel::selectionChanged, this, &ExperimentsBrowser::onSelectionChanged);
    connect(addNodeGroup, &QActionGroup::triggered, this, &ExperimentsBrowser::onAddNewNodeTriggered);
    connect(expandAll, &QAction::triggered, treeView, &QTreeView::expandAll);
    connect(collapseAll, &QAction::triggered, treeView, &QTreeView::collapseAll);
}


bool ExperimentsBrowser::isNodeExists(NodeType nodeType, const QString &name, BrowserData *parentNode) {
    if (!parentNode) {
        return false;
    }

    switch (nodeType) {
    case NodeType::Experiment:
        return ExperimentsDAO().isExperimentExists(name, User::loginUser().id());
    case NodeType::Specimen:
        return SpecimensDAO().isSpecimenExists(name, parentNode->nodeId());
    case NodeType::Tube:
        return TubesDAO().isTubeExists(name, parentNode->nodeId());
    case NodeType::Settings:
        return CytometerSettingsDAO().isCytometerSettingsExists(parentNode->nodeType(), parentNode->nodeId());
    default:
        return false;
    }
}


QModelIndex ExperimentsBrowser::getParentForNewNode(NodeType nodeType)
{
    QModelIndex selectedIndex = treeView->selectionModel()->currentIndex();
    if (!selectedIndex.isValid()) {
        return QModelIndex();
    }

    BrowserData *selectedNode = m_model->nodeFromIndex(selectedIndex);
    if (!selectedNode) {
        return QModelIndex();
    }

    switch (nodeType) {
    case NodeType::Experiment:
        if (m_LoginUserIndex.isValid()) {
            return m_LoginUserIndex;
        }
        break;
    case NodeType::Specimen:
        return m_model->getAncestorIndex(selectedIndex, NodeType::Experiment);
        break;
    case NodeType::Tube:
        return m_model->getAncestorIndex(selectedIndex, NodeType::Specimen);
        break;
    case NodeType::Settings:
        if (selectedNode->nodeType() == NodeType::Tube || selectedNode->nodeType() == NodeType::Specimen){
            for (BrowserData *child : selectedNode->children()) {
                if (child->nodeType() == NodeType::Settings) {
                    return QModelIndex();
                }
            }
            return selectedIndex;
        }
        break;

    case NodeType::Worksheet:
        return m_model->getAncestorIndex(selectedIndex, NodeType::Experiment);
        break;

    default:
        return QModelIndex();
        break;
    }

    return QModelIndex();
}

QModelIndex ExperimentsBrowser::insertNewNode(NodeType nodeType, const QString &name, const QModelIndex &parent)
{
    if (!parent.isValid()) {
        return QModelIndex();
    }

    BrowserData *parentNode = m_model->nodeFromIndex(parent);

    if (isNodeExists(nodeType, name, parentNode)) {
        QMessageBox::warning(this, tr("Duplicate Node Name"), QString("%1-%2 %3").arg(NodeTypeHelper::nodeTypeToString(nodeType), name, tr(" already exists")));
        return QModelIndex();
    }

    int nodeId = insertNewNodeToDB(nodeType, parentNode, name);
    if (nodeId <= 0) {
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to insert new node"));
        return QModelIndex();
    }

    QModelIndex newIndex = m_model->updateNewNode(nodeType, parent, nodeId);
    return newIndex;
}

int ExperimentsBrowser::insertNewNodeToDB(NodeType nodeType, BrowserData *parentNode, const QString &nodeName)
{
    int nodeId = 0;
    switch (nodeType) {
    case NodeType::Experiment:
        nodeId = ExperimentsDAO().insertExperiment(nodeName, parentNode->nodeId());
        break;
    case NodeType::Specimen:
        nodeId = SpecimensDAO().insertSpecimen(nodeName, parentNode->nodeId());
        break;
    case NodeType::Tube:
        nodeId = TubesDAO().insertTube(nodeName, parentNode->nodeId());
        break;
    case NodeType::Settings:
        nodeId = CytometerSettingsDAO().insertCytometerSettings(nodeName, parentNode->nodeType(), parentNode->nodeId());
        break;
    case NodeType::Worksheet:
        nodeId = WorkSheetsDAO().insertWorkSheet(nodeName, (parentNode->nodeType() == NodeType::Experiment), parentNode->nodeId());
        break;
    default:
        break;
    }
    return nodeId;
}


void ExperimentsBrowser::onAddNewNodeTriggered(QAction *action)
{
    NodeType nodeType = action->data().value<NodeType>();


    QModelIndex parentIndex = getParentForNewNode(nodeType);
    QString errorMessage;
    if (!parentIndex.isValid()) {
        errorMessage =  QString("%1 %2").arg(NodeTypeHelper::nodeTypeToString(nodeType), tr(" must have a correct parent node"));
        QMessageBox::warning(this, tr("Selection Error"), errorMessage);
        return;
    }

    qDebug() << QString("Add new %1 node under parent: ").arg(NodeTypeHelper::nodeTypeToString(nodeType)) << parentIndex;
    AddNewItemDialog *dlg = new AddNewItemDialog(NodeTypeHelper::nodeTypeToString(nodeType), this);
    if (dlg->exec() == QDialog::Accepted) {
        QString name = dlg->getItemName();
        delete dlg;

        QModelIndex newNodeIndex = insertNewNode(nodeType, name, parentIndex);
        if (newNodeIndex.isValid()) {
            qDebug() << QString("New %1 node added: ").arg(NodeTypeHelper::nodeTypeToString(nodeType)) << newNodeIndex;
            treeView->setCurrentIndex(newNodeIndex);
        }
        if (nodeType == NodeType::Experiment) {
            QModelIndex settingIndex = insertNewNode(NodeType::Settings, "CytometerSettings", newNodeIndex);
            if (!settingIndex.isValid()) {
                QMessageBox::warning(this, tr("Insert Default Cytometer Settings Failed"), tr("Failed to insert default cytometer settings for new experiment"));
            }
            QModelIndex workSheetIndex = insertNewNode(NodeType::Worksheet, "Global WorkSheet", newNodeIndex);
            if (!workSheetIndex.isValid()) {
                QMessageBox::warning(this, tr("Insert Default WorkSheet Failed"), tr("Failed to insert default worksheet for new experiment"));
            }
        }
    } else {
        delete dlg;
    }

}

void ExperimentsBrowser::onSelectionChanged(const QItemSelection &selected, const QItemSelection &previous)
{
    if (selected.isEmpty()) {
        return;
    }
    QModelIndex currentIndex = selected.indexes().last();
    currentIndexPath = m_model->getNodePath(currentIndex);
    QStringList pathList;
    currentNodePath.clear();
    for (auto index : currentIndexPath) {
        BrowserData *node = m_model->nodeFromIndex(index);
        currentNodePath.append(node);
        pathList << node->nodeName();
    }
    currentPathStr = pathList.join("->");
    statusBar->showMessage(currentPathStr);

    BrowserData *currentNode = m_model->nodeFromIndex(currentIndex);
    NodeType nodeType = currentNode->nodeType();
    switch (nodeType) {
    case NodeType::Experiment:
        emit experimentSelected(currentNode->nodeId());
        break;
    case NodeType::Worksheet:
        emit worksheetSelected(currentNode->nodeId());
        break;
    case NodeType::Settings:
        emit settingsSelected(currentNode->nodeId());
        break;
    default:
        break;
    }
}





