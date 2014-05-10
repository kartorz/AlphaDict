/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
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
    QPushButton *netdictButton;
    QPushButton *saveButton;
    QToolButton *pgupToolButton2;
    QToolButton *pgupToolButton1;
    QToolButton *pgdownToolButton1;
    QToolButton *pgdownToolButton2;
    QLineEdit *indexLineEdit;
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
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/vocabulary.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVocabulary->setIcon(icon);
        actionVocabulary->setIconVisibleInMenu(true);
        actionSetting = new QAction(MainWindow);
        actionSetting->setObjectName(QStringLiteral("actionSetting"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/res/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSetting->setIcon(icon1);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 721, 454));
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
        indexListView->setGeometry(QRect(-1, 0, 161, 341));
        indexListView->setAcceptDrops(false);
        indexListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        inputLineEdit = new QLineEdit(dictTab);
        inputLineEdit->setObjectName(QStringLiteral("inputLineEdit"));
        inputLineEdit->setGeometry(QRect(450, 14, 201, 32));
        inputLineEdit->setReadOnly(false);
        srcLanComboBox = new QComboBox(dictTab);
        srcLanComboBox->setObjectName(QStringLiteral("srcLanComboBox"));
        srcLanComboBox->setGeometry(QRect(180, 14, 101, 32));
        detLanComboBox = new QComboBox(dictTab);
        detLanComboBox->setObjectName(QStringLiteral("detLanComboBox"));
        detLanComboBox->setGeometry(QRect(330, 14, 101, 32));
        queryButton = new QPushButton(dictTab);
        queryButton->setObjectName(QStringLiteral("queryButton"));
        queryButton->setGeometry(QRect(662, 10, 48, 48));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/res/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        queryButton->setIcon(icon2);
        queryButton->setIconSize(QSize(48, 48));
        label = new QLabel(dictTab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(290, 21, 31, 17));
        label->setPixmap(QPixmap(QString::fromUtf8(":/res/to_right_arrow.png")));
        dictTextEdit = new QTextEdit(dictTab);
        dictTextEdit->setObjectName(QStringLiteral("dictTextEdit"));
        dictTextEdit->setGeometry(QRect(170, 69, 550, 271));
        dictTextEdit->setAcceptDrops(false);
        dictTextEdit->setReadOnly(true);
        netdictButton = new QPushButton(dictTab);
        netdictButton->setObjectName(QStringLiteral("netdictButton"));
        netdictButton->setGeometry(QRect(650, 360, 48, 48));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/res/web.png"), QSize(), QIcon::Normal, QIcon::Off);
        netdictButton->setIcon(icon3);
        netdictButton->setIconSize(QSize(48, 48));
        saveButton = new QPushButton(dictTab);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        saveButton->setGeometry(QRect(570, 360, 48, 48));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/res/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveButton->setIcon(icon4);
        saveButton->setIconSize(QSize(48, 48));
        pgupToolButton2 = new QToolButton(dictTab);
        pgupToolButton2->setObjectName(QStringLiteral("pgupToolButton2"));
        pgupToolButton2->setGeometry(QRect(9, 383, 24, 32));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/res/left_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton2->setIcon(icon5);
        pgupToolButton2->setIconSize(QSize(18, 32));
        pgupToolButton1 = new QToolButton(dictTab);
        pgupToolButton1->setObjectName(QStringLiteral("pgupToolButton1"));
        pgupToolButton1->setGeometry(QRect(47, 383, 24, 32));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/res/left_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton1->setIcon(icon6);
        pgupToolButton1->setIconSize(QSize(18, 32));
        pgdownToolButton1 = new QToolButton(dictTab);
        pgdownToolButton1->setObjectName(QStringLiteral("pgdownToolButton1"));
        pgdownToolButton1->setGeometry(QRect(87, 383, 24, 32));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/res/right_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton1->setIcon(icon7);
        pgdownToolButton1->setIconSize(QSize(18, 32));
        pgdownToolButton2 = new QToolButton(dictTab);
        pgdownToolButton2->setObjectName(QStringLiteral("pgdownToolButton2"));
        pgdownToolButton2->setGeometry(QRect(126, 383, 24, 32));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/res/right_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton2->setIcon(icon8);
        pgdownToolButton2->setIconSize(QSize(18, 32));
        indexLineEdit = new QLineEdit(dictTab);
        indexLineEdit->setObjectName(QStringLiteral("indexLineEdit"));
        indexLineEdit->setGeometry(QRect(10, 349, 141, 27));
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
        dictInfoLabel->setGeometry(QRect(265, 80, 231, 251));
        dictInfoLabel->setTextFormat(Qt::PlainText);
        dictInfoLabel->setWordWrap(true);
        dictUpToolButton = new QToolButton(settingTab);
        dictUpToolButton->setObjectName(QStringLiteral("dictUpToolButton"));
        dictUpToolButton->setGeometry(QRect(333, 10, 32, 60));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/res/up.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictUpToolButton->setIcon(icon9);
        dictUpToolButton->setIconSize(QSize(32, 32));
        dictDownToolButton = new QToolButton(settingTab);
        dictDownToolButton->setObjectName(QStringLiteral("dictDownToolButton"));
        dictDownToolButton->setGeometry(QRect(333, 350, 32, 60));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/res/down.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictDownToolButton->setIcon(icon10);
        dictDownToolButton->setIconSize(QSize(32, 32));
        tabWidget->addTab(settingTab, QString());
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setIconSize(QSize(48, 48));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionSetting);
        toolBar->addAction(actionVocabulary);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionVocabulary->setText(QString());
#ifndef QT_NO_TOOLTIP
        actionVocabulary->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>vocabulay</p><p><span style=\" font-style:italic;\">not support yet!</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        actionSetting->setText(QString());
#ifndef QT_NO_TOOLTIP
        actionSetting->setToolTip(QApplication::translate("MainWindow", "setting", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        indexListView->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_ACCESSIBILITY
        indexListView->setAccessibleName(QString());
#endif // QT_NO_ACCESSIBILITY
        queryButton->setText(QString());
        label->setText(QString());
#ifndef QT_NO_TOOLTIP
        netdictButton->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>lookup net dictionary</p><p><span style=\" font-style:italic;\">not support yet!</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        netdictButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        saveButton->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>add to vocabulary book</p><p><span style=\" font-style:italic;\">not support yet!</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        saveButton->setText(QString());
        pgupToolButton2->setText(QString());
        pgupToolButton1->setText(QString());
        pgdownToolButton1->setText(QString());
        pgdownToolButton2->setText(QString());
#ifndef QT_NO_TOOLTIP
        indexLineEdit->setToolTip(QApplication::translate("MainWindow", "filter by a prefix", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        indexLineEdit->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        indexLineEdit->setPlaceholderText(QString());
        tabWidget->setTabText(tabWidget->indexOf(dictTab), QApplication::translate("MainWindow", "dictionary", 0));
#ifndef QT_NO_TOOLTIP
        dictListWidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        dictInfoLabel->setText(QString());
        dictUpToolButton->setText(QString());
        dictDownToolButton->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(settingTab), QApplication::translate("MainWindow", "setting", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
