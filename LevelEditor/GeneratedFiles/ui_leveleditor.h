/********************************************************************************
** Form generated from reading UI file 'leveleditor.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LEVELEDITOR_H
#define UI_LEVELEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LevelEditorClass
{
public:
    QAction *actionCreate_Static_Entity;
    QAction *actionCreate_Dynamic_Entity;
    QAction *actionSaveFile;
    QAction *actionCreate_Gravity_Area;
    QAction *actionCreate_Gravity_Well;
    QAction *actionExit;
    QWidget *centralWidget;
    QTreeWidget *treeWidget;
    QGroupBox *groupBox_Data;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *LevelEditorClass)
    {
        if (LevelEditorClass->objectName().isEmpty())
            LevelEditorClass->setObjectName(QStringLiteral("LevelEditorClass"));
        LevelEditorClass->resize(1920, 1080);
        actionCreate_Static_Entity = new QAction(LevelEditorClass);
        actionCreate_Static_Entity->setObjectName(QStringLiteral("actionCreate_Static_Entity"));
        actionCreate_Dynamic_Entity = new QAction(LevelEditorClass);
        actionCreate_Dynamic_Entity->setObjectName(QStringLiteral("actionCreate_Dynamic_Entity"));
        actionSaveFile = new QAction(LevelEditorClass);
        actionSaveFile->setObjectName(QStringLiteral("actionSaveFile"));
        actionCreate_Gravity_Area = new QAction(LevelEditorClass);
        actionCreate_Gravity_Area->setObjectName(QStringLiteral("actionCreate_Gravity_Area"));
        actionCreate_Gravity_Well = new QAction(LevelEditorClass);
        actionCreate_Gravity_Well->setObjectName(QStringLiteral("actionCreate_Gravity_Well"));
        actionExit = new QAction(LevelEditorClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        centralWidget = new QWidget(LevelEditorClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        treeWidget = new QTreeWidget(centralWidget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QStringLiteral("treeWidget"));
        treeWidget->setGeometry(QRect(1590, 20, 256, 361));
        groupBox_Data = new QGroupBox(centralWidget);
        groupBox_Data->setObjectName(QStringLiteral("groupBox_Data"));
        groupBox_Data->setGeometry(QRect(1500, 440, 381, 221));
        LevelEditorClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(LevelEditorClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1920, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        LevelEditorClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(LevelEditorClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        LevelEditorClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(LevelEditorClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        LevelEditorClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionSaveFile);
        menuFile->addAction(actionExit);
        mainToolBar->addAction(actionCreate_Static_Entity);
        mainToolBar->addAction(actionCreate_Dynamic_Entity);
        mainToolBar->addAction(actionCreate_Gravity_Area);
        mainToolBar->addAction(actionCreate_Gravity_Well);

        retranslateUi(LevelEditorClass);

        QMetaObject::connectSlotsByName(LevelEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *LevelEditorClass)
    {
        LevelEditorClass->setWindowTitle(QApplication::translate("LevelEditorClass", "LevelEditor", 0));
        actionCreate_Static_Entity->setText(QApplication::translate("LevelEditorClass", "Create Static Entity", 0));
        actionCreate_Dynamic_Entity->setText(QApplication::translate("LevelEditorClass", "Create Dynamic Entity", 0));
        actionSaveFile->setText(QApplication::translate("LevelEditorClass", "Save", 0));
        actionSaveFile->setShortcut(QApplication::translate("LevelEditorClass", "Ctrl+S", 0));
        actionCreate_Gravity_Area->setText(QApplication::translate("LevelEditorClass", "Create Gravity Area", 0));
        actionCreate_Gravity_Well->setText(QApplication::translate("LevelEditorClass", "Create Gravity Well", 0));
        actionExit->setText(QApplication::translate("LevelEditorClass", "Exit", 0));
        groupBox_Data->setTitle(QApplication::translate("LevelEditorClass", "Data", 0));
        menuFile->setTitle(QApplication::translate("LevelEditorClass", "File", 0));
    } // retranslateUi

};

namespace Ui {
    class LevelEditorClass: public Ui_LevelEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LEVELEDITOR_H
