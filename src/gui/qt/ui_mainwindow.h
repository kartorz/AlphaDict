/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionVocabulary;
    QAction *actionSetting;
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *dictTab;
    QListView *indexListView;
    QLineEdit *inputLineEdit;
    QComboBox *srcLanComboBox;
    QComboBox *detLanComboBox;
    QPushButton *queryButton;
    QLabel *label;
    QTextEdit *dictTextEdit;
    QPushButton *moreButton;
    QPushButton *netdictButton;
    QPushButton *saveButton;
    QToolButton *pgupToolButton2;
    QToolButton *pgupToolButton1;
    QToolButton *pgdownToolButton1;
    QToolButton *pgdownToolButton2;
    QWidget *settingTab;
    QListWidget *dictListWidget;
    QLabel *dictInfoLabel;
    QToolButton *dictUpToolButton;
    QToolButton *dictDownToolButton;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(720, 540);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        actionVocabulary = new QAction(MainWindow);
        actionVocabulary->setObjectName(QStringLiteral("actionVocabulary"));
        actionVocabulary->setIconVisibleInMenu(true);
        actionSetting = new QAction(MainWindow);
        actionSetting->setObjectName(QStringLiteral("actionSetting"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(0, 13, 721, 561));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setFocusPolicy(Qt::NoFocus);
        tabWidget->setLayoutDirection(Qt::LeftToRight);
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Triangular);
        dictTab = new QWidget();
        dictTab->setObjectName(QStringLiteral("dictTab"));
        indexListView = new QListView(dictTab);
        indexListView->setObjectName(QStringLiteral("indexListView"));
        indexListView->setGeometry(QRect(-1, 0, 161, 400));
        indexListView->setAcceptDrops(false);
        indexListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        inputLineEdit = new QLineEdit(dictTab);
        inputLineEdit->setObjectName(QStringLiteral("inputLineEdit"));
        inputLineEdit->setGeometry(QRect(440, 14, 181, 31));
        inputLineEdit->setReadOnly(false);
        srcLanComboBox = new QComboBox(dictTab);
        srcLanComboBox->setObjectName(QStringLiteral("srcLanComboBox"));
        srcLanComboBox->setGeometry(QRect(180, 14, 101, 31));
        detLanComboBox = new QComboBox(dictTab);
        detLanComboBox->setObjectName(QStringLiteral("detLanComboBox"));
        detLanComboBox->setGeometry(QRect(330, 14, 101, 31));
        queryButton = new QPushButton(dictTab);
        queryButton->setObjectName(QStringLiteral("queryButton"));
        queryButton->setGeometry(QRect(628, 14, 61, 31));
        label = new QLabel(dictTab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(290, 21, 31, 17));
        dictTextEdit = new QTextEdit(dictTab);
        dictTextEdit->setObjectName(QStringLiteral("dictTextEdit"));
        dictTextEdit->setGeometry(QRect(170, 60, 550, 340));
        dictTextEdit->setAcceptDrops(false);
        dictTextEdit->setReadOnly(true);
        moreButton = new QPushButton(dictTab);
        moreButton->setObjectName(QStringLiteral("moreButton"));
        moreButton->setGeometry(QRect(615, 410, 71, 31));
        netdictButton = new QPushButton(dictTab);
        netdictButton->setObjectName(QStringLiteral("netdictButton"));
        netdictButton->setGeometry(QRect(506, 410, 71, 31));
        saveButton = new QPushButton(dictTab);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        saveButton->setGeometry(QRect(395, 410, 71, 31));
        pgupToolButton2 = new QToolButton(dictTab);
        pgupToolButton2->setObjectName(QStringLiteral("pgupToolButton2"));
        pgupToolButton2->setGeometry(QRect(4, 414, 25, 27));
        pgupToolButton1 = new QToolButton(dictTab);
        pgupToolButton1->setObjectName(QStringLiteral("pgupToolButton1"));
        pgupToolButton1->setGeometry(QRect(40, 414, 25, 27));
        pgdownToolButton1 = new QToolButton(dictTab);
        pgdownToolButton1->setObjectName(QStringLiteral("pgdownToolButton1"));
        pgdownToolButton1->setGeometry(QRect(83, 414, 25, 27));
        pgdownToolButton2 = new QToolButton(dictTab);
        pgdownToolButton2->setObjectName(QStringLiteral("pgdownToolButton2"));
        pgdownToolButton2->setGeometry(QRect(120, 414, 25, 27));
        tabWidget->addTab(dictTab, QString());
        settingTab = new QWidget();
        settingTab->setObjectName(QStringLiteral("settingTab"));
        settingTab->setEnabled(true);
        dictListWidget = new QListWidget(settingTab);
        dictListWidget->setObjectName(QStringLiteral("dictListWidget"));
        dictListWidget->setGeometry(QRect(0, 0, 256, 491));
        dictListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        dictListWidget->setProperty("showDropIndicator", QVariant(false));
        dictListWidget->setDragEnabled(false);
        dictListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
        dictInfoLabel = new QLabel(settingTab);
        dictInfoLabel->setObjectName(QStringLiteral("dictInfoLabel"));
        dictInfoLabel->setGeometry(QRect(290, 90, 411, 221));
        dictUpToolButton = new QToolButton(settingTab);
        dictUpToolButton->setObjectName(QStringLiteral("dictUpToolButton"));
        dictUpToolButton->setGeometry(QRect(270, 0, 31, 61));
        dictDownToolButton = new QToolButton(settingTab);
        dictDownToolButton->setObjectName(QStringLiteral("dictDownToolButton"));
        dictDownToolButton->setGeometry(QRect(270, 360, 31, 71));
        tabWidget->addTab(settingTab, QString());
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionSetting);
        toolBar->addAction(actionVocabulary);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionVocabulary->setText(QApplication::translate("MainWindow", "vocabulary", 0));
        actionSetting->setText(QApplication::translate("MainWindow", "setting", 0));
#ifndef QT_NO_TOOLTIP
        indexListView->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_ACCESSIBILITY
        indexListView->setAccessibleName(QString());
#endif // QT_NO_ACCESSIBILITY
        queryButton->setText(QApplication::translate("MainWindow", "Query", 0));
        label->setText(QApplication::translate("MainWindow", "----->", 0));
        moreButton->setText(QApplication::translate("MainWindow", "more", 0));
        netdictButton->setText(QApplication::translate("MainWindow", "net dict", 0));
        saveButton->setText(QApplication::translate("MainWindow", "save", 0));
        pgupToolButton2->setText(QApplication::translate("MainWindow", "<<", 0));
        pgupToolButton1->setText(QApplication::translate("MainWindow", "<", 0));
        pgdownToolButton1->setText(QApplication::translate("MainWindow", ">", 0));
        pgdownToolButton2->setText(QApplication::translate("MainWindow", ">>", 0));
        tabWidget->setTabText(tabWidget->indexOf(dictTab), QApplication::translate("MainWindow", "dictionary", 0));
#ifndef QT_NO_TOOLTIP
        dictListWidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        dictInfoLabel->setText(QString());
        dictUpToolButton->setText(QApplication::translate("MainWindow", "^\n"
"", 0));
        dictDownToolButton->setText(QApplication::translate("MainWindow", "v", 0));
        tabWidget->setTabText(tabWidget->indexOf(settingTab), QApplication::translate("MainWindow", "setting", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
