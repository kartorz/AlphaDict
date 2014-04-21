/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.2
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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
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
    QWidget *toolTab;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

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
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(10, 0, 711, 481));
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
        indexListView->setGeometry(QRect(0, 0, 141, 451));
        inputLineEdit = new QLineEdit(dictTab);
        inputLineEdit->setObjectName(QStringLiteral("inputLineEdit"));
        inputLineEdit->setGeometry(QRect(440, 30, 181, 31));
        inputLineEdit->setReadOnly(false);
        srcLanComboBox = new QComboBox(dictTab);
        srcLanComboBox->setObjectName(QStringLiteral("srcLanComboBox"));
        srcLanComboBox->setGeometry(QRect(160, 30, 111, 31));
        detLanComboBox = new QComboBox(dictTab);
        detLanComboBox->setObjectName(QStringLiteral("detLanComboBox"));
        detLanComboBox->setGeometry(QRect(320, 30, 111, 31));
        queryButton = new QPushButton(dictTab);
        queryButton->setObjectName(QStringLiteral("queryButton"));
        queryButton->setGeometry(QRect(628, 30, 61, 31));
        label = new QLabel(dictTab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(280, 37, 31, 17));
        dictTextEdit = new QTextEdit(dictTab);
        dictTextEdit->setObjectName(QStringLiteral("dictTextEdit"));
        dictTextEdit->setGeometry(QRect(160, 80, 531, 371));
        dictTextEdit->setReadOnly(true);
        tabWidget->addTab(dictTab, QString());
        toolTab = new QWidget();
        toolTab->setObjectName(QStringLiteral("toolTab"));
        tabWidget->addTab(toolTab, QString());
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 720, 25));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
#ifndef QT_NO_ACCESSIBILITY
        indexListView->setAccessibleName(QString());
#endif // QT_NO_ACCESSIBILITY
        queryButton->setText(QApplication::translate("MainWindow", "Query", 0));
        label->setText(QApplication::translate("MainWindow", "----->", 0));
        tabWidget->setTabText(tabWidget->indexOf(dictTab), QApplication::translate("MainWindow", "dictionary", 0));
        tabWidget->setTabText(tabWidget->indexOf(toolTab), QApplication::translate("MainWindow", "Tool | setting", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
