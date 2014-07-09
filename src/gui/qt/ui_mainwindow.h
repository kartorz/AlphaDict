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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
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
    QLabel *label;
    QTextEdit *dictTextEdit;
    QToolButton *pgupToolButton2;
    QToolButton *pgupToolButton1;
    QToolButton *pgdownToolButton1;
    QToolButton *pgdownToolButton2;
    QLineEdit *indexLineEdit;
    QToolButton *saveButton;
    QToolButton *netdictButton;
    QToolButton *queryButton;
    QWidget *settingTab;
    QTabWidget *settingTabWidget;
    QWidget *tab;
    QGroupBox *groupBox_3;
    QLabel *uilanLabel;
    QComboBox *uilanComboBox;
    QWidget *tab_2;
    QGroupBox *groupBox;
    QCheckBox *cwsSelectionCheckBox;
    QCheckBox *cwsClipboardCheckBox;
    QListWidget *dictListWidget;
    QToolButton *dictDownToolButton;
    QToolButton *dictUpToolButton;
    QWidget *vocabularyTab;
    QTabWidget *vocabularyTabWidget;
    QWidget *listModeTable;
    QToolButton *vbdelToolButton;
    QToolButton *vbclearToolButton;
    QPlainTextEdit *vbexplTextEdit;
    QListView *vbookListView;
    QWidget *vbModeTab;
    QToolButton *vbpreItemTlBtn;
    QToolButton *vbnextItemTlBtn;
    QLineEdit *vbInput;
    QLabel *vbScoreLabel;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout;
    QLabel *vbExplLabel;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(720, 530);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/appicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setIconSize(QSize(128, 128));
        actionVocabulary = new QAction(MainWindow);
        actionVocabulary->setObjectName(QStringLiteral("actionVocabulary"));
        actionVocabulary->setEnabled(true);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/res/vocabulary.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVocabulary->setIcon(icon1);
        actionVocabulary->setIconVisibleInMenu(true);
        actionSetting = new QAction(MainWindow);
        actionSetting->setObjectName(QStringLiteral("actionSetting"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/res/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSetting->setIcon(icon2);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(1, 0, 718, 444));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setFocusPolicy(Qt::NoFocus);
        tabWidget->setLayoutDirection(Qt::LeftToRight);
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tabWidget->setIconSize(QSize(20, 20));
        tabWidget->setElideMode(Qt::ElideNone);
        tabWidget->setUsesScrollButtons(false);
        dictTab = new QWidget();
        dictTab->setObjectName(QStringLiteral("dictTab"));
        indexListView = new QListView(dictTab);
        indexListView->setObjectName(QStringLiteral("indexListView"));
        indexListView->setGeometry(QRect(-1, 0, 160, 340));
        indexListView->setAcceptDrops(false);
        indexListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        indexListView->setProperty("showDropIndicator", QVariant(false));
        inputLineEdit = new QLineEdit(dictTab);
        inputLineEdit->setObjectName(QStringLiteral("inputLineEdit"));
        inputLineEdit->setGeometry(QRect(450, 14, 190, 32));
        inputLineEdit->setReadOnly(false);
        srcLanComboBox = new QComboBox(dictTab);
        srcLanComboBox->setObjectName(QStringLiteral("srcLanComboBox"));
        srcLanComboBox->setGeometry(QRect(180, 14, 101, 32));
        detLanComboBox = new QComboBox(dictTab);
        detLanComboBox->setObjectName(QStringLiteral("detLanComboBox"));
        detLanComboBox->setGeometry(QRect(330, 14, 101, 32));
        label = new QLabel(dictTab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(289, 21, 32, 18));
        label->setPixmap(QPixmap(QString::fromUtf8(":/res/to_right_arrow.png")));
        dictTextEdit = new QTextEdit(dictTab);
        dictTextEdit->setObjectName(QStringLiteral("dictTextEdit"));
        dictTextEdit->setGeometry(QRect(170, 69, 540, 271));
        dictTextEdit->setAcceptDrops(false);
        dictTextEdit->setReadOnly(true);
        pgupToolButton2 = new QToolButton(dictTab);
        pgupToolButton2->setObjectName(QStringLiteral("pgupToolButton2"));
        pgupToolButton2->setGeometry(QRect(9, 378, 24, 32));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/res/left_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton2->setIcon(icon3);
        pgupToolButton2->setIconSize(QSize(18, 32));
        pgupToolButton2->setAutoRaise(true);
        pgupToolButton2->setArrowType(Qt::NoArrow);
        pgupToolButton1 = new QToolButton(dictTab);
        pgupToolButton1->setObjectName(QStringLiteral("pgupToolButton1"));
        pgupToolButton1->setGeometry(QRect(47, 378, 24, 32));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/res/left_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton1->setIcon(icon4);
        pgupToolButton1->setIconSize(QSize(18, 32));
        pgupToolButton1->setAutoRaise(true);
        pgdownToolButton1 = new QToolButton(dictTab);
        pgdownToolButton1->setObjectName(QStringLiteral("pgdownToolButton1"));
        pgdownToolButton1->setGeometry(QRect(87, 378, 24, 32));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/res/right_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton1->setIcon(icon5);
        pgdownToolButton1->setIconSize(QSize(18, 32));
        pgdownToolButton1->setAutoRaise(true);
        pgdownToolButton2 = new QToolButton(dictTab);
        pgdownToolButton2->setObjectName(QStringLiteral("pgdownToolButton2"));
        pgdownToolButton2->setGeometry(QRect(126, 378, 24, 32));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/res/right_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton2->setIcon(icon6);
        pgdownToolButton2->setIconSize(QSize(18, 32));
        pgdownToolButton2->setAutoRaise(true);
        indexLineEdit = new QLineEdit(dictTab);
        indexLineEdit->setObjectName(QStringLiteral("indexLineEdit"));
        indexLineEdit->setGeometry(QRect(10, 349, 141, 27));
        saveButton = new QToolButton(dictTab);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        saveButton->setGeometry(QRect(572, 356, 48, 48));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/res/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveButton->setIcon(icon7);
        saveButton->setIconSize(QSize(48, 48));
        saveButton->setAutoRaise(true);
        netdictButton = new QToolButton(dictTab);
        netdictButton->setObjectName(QStringLiteral("netdictButton"));
        netdictButton->setGeometry(QRect(641, 356, 48, 48));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/res/web.png"), QSize(), QIcon::Normal, QIcon::Off);
        netdictButton->setIcon(icon8);
        netdictButton->setIconSize(QSize(48, 48));
        netdictButton->setAutoRaise(true);
        queryButton = new QToolButton(dictTab);
        queryButton->setObjectName(QStringLiteral("queryButton"));
        queryButton->setGeometry(QRect(654, 6, 48, 48));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/res/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        queryButton->setIcon(icon9);
        queryButton->setIconSize(QSize(48, 48));
        queryButton->setAutoRaise(true);
        tabWidget->addTab(dictTab, QString());
        settingTab = new QWidget();
        settingTab->setObjectName(QStringLiteral("settingTab"));
        settingTab->setEnabled(true);
        settingTabWidget = new QTabWidget(settingTab);
        settingTabWidget->setObjectName(QStringLiteral("settingTabWidget"));
        settingTabWidget->setGeometry(QRect(0, -3, 717, 414));
        settingTabWidget->setFocusPolicy(Qt::NoFocus);
        settingTabWidget->setTabPosition(QTabWidget::South);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        groupBox_3 = new QGroupBox(tab);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(8, 8, 244, 100));
        groupBox_3->setStyleSheet(QLatin1String("#groupBox_3 { \n"
"     border: 1px solid gray; \n"
"     border-radius: 1px; \n"
" } "));
        uilanLabel = new QLabel(groupBox_3);
        uilanLabel->setObjectName(QStringLiteral("uilanLabel"));
        uilanLabel->setGeometry(QRect(17, 37, 90, 17));
        uilanComboBox = new QComboBox(groupBox_3);
        uilanComboBox->setObjectName(QStringLiteral("uilanComboBox"));
        uilanComboBox->setGeometry(QRect(112, 32, 90, 27));
        settingTabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        groupBox = new QGroupBox(tab_2);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setEnabled(true);
        groupBox->setGeometry(QRect(310, 10, 390, 101));
        groupBox->setAutoFillBackground(false);
        groupBox->setStyleSheet(QLatin1String("#groupBox { \n"
"     border: 1px solid gray; \n"
"     border-radius: 1px; \n"
" } "));
        groupBox->setCheckable(false);
        groupBox->setChecked(false);
        cwsSelectionCheckBox = new QCheckBox(groupBox);
        cwsSelectionCheckBox->setObjectName(QStringLiteral("cwsSelectionCheckBox"));
        cwsSelectionCheckBox->setGeometry(QRect(10, 30, 161, 22));
        cwsClipboardCheckBox = new QCheckBox(groupBox);
        cwsClipboardCheckBox->setObjectName(QStringLiteral("cwsClipboardCheckBox"));
        cwsClipboardCheckBox->setGeometry(QRect(219, 30, 161, 22));
        dictListWidget = new QListWidget(tab_2);
        dictListWidget->setObjectName(QStringLiteral("dictListWidget"));
        dictListWidget->setGeometry(QRect(20, 10, 261, 300));
        dictListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        dictListWidget->setProperty("showDropIndicator", QVariant(false));
        dictListWidget->setDragEnabled(false);
        dictListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
        dictDownToolButton = new QToolButton(tab_2);
        dictDownToolButton->setObjectName(QStringLiteral("dictDownToolButton"));
        dictDownToolButton->setGeometry(QRect(185, 329, 59, 30));
        dictDownToolButton->setFocusPolicy(Qt::ClickFocus);
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/res/down.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictDownToolButton->setIcon(icon10);
        dictDownToolButton->setIconSize(QSize(32, 32));
        dictDownToolButton->setAutoRaise(false);
        dictUpToolButton = new QToolButton(tab_2);
        dictUpToolButton->setObjectName(QStringLiteral("dictUpToolButton"));
        dictUpToolButton->setGeometry(QRect(49, 329, 59, 30));
        dictUpToolButton->setFocusPolicy(Qt::ClickFocus);
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/res/up.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictUpToolButton->setIcon(icon11);
        dictUpToolButton->setIconSize(QSize(32, 32));
        dictUpToolButton->setAutoRaise(false);
        settingTabWidget->addTab(tab_2, QString());
        tabWidget->addTab(settingTab, QString());
        vocabularyTab = new QWidget();
        vocabularyTab->setObjectName(QStringLiteral("vocabularyTab"));
        vocabularyTabWidget = new QTabWidget(vocabularyTab);
        vocabularyTabWidget->setObjectName(QStringLiteral("vocabularyTabWidget"));
        vocabularyTabWidget->setGeometry(QRect(0, -3, 717, 411));
        vocabularyTabWidget->setFocusPolicy(Qt::NoFocus);
        vocabularyTabWidget->setTabPosition(QTabWidget::South);
        listModeTable = new QWidget();
        listModeTable->setObjectName(QStringLiteral("listModeTable"));
        vbdelToolButton = new QToolButton(listModeTable);
        vbdelToolButton->setObjectName(QStringLiteral("vbdelToolButton"));
        vbdelToolButton->setGeometry(QRect(50, 321, 32, 32));
        QIcon icon12;
        icon12.addFile(QStringLiteral(":/res/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbdelToolButton->setIcon(icon12);
        vbdelToolButton->setIconSize(QSize(32, 32));
        vbdelToolButton->setAutoRaise(true);
        vbclearToolButton = new QToolButton(listModeTable);
        vbclearToolButton->setObjectName(QStringLiteral("vbclearToolButton"));
        vbclearToolButton->setGeometry(QRect(175, 322, 32, 32));
        QIcon icon13;
        icon13.addFile(QStringLiteral(":/res/clear_all.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbclearToolButton->setIcon(icon13);
        vbclearToolButton->setIconSize(QSize(32, 32));
        vbclearToolButton->setAutoRaise(true);
        vbexplTextEdit = new QPlainTextEdit(listModeTable);
        vbexplTextEdit->setObjectName(QStringLiteral("vbexplTextEdit"));
        vbexplTextEdit->setGeometry(QRect(306, 36, 368, 240));
        vbexplTextEdit->setUndoRedoEnabled(false);
        vbexplTextEdit->setReadOnly(true);
        vbookListView = new QListView(listModeTable);
        vbookListView->setObjectName(QStringLiteral("vbookListView"));
        vbookListView->setGeometry(QRect(10, 10, 281, 290));
        vocabularyTabWidget->addTab(listModeTable, QString());
        vbModeTab = new QWidget();
        vbModeTab->setObjectName(QStringLiteral("vbModeTab"));
        vbpreItemTlBtn = new QToolButton(vbModeTab);
        vbpreItemTlBtn->setObjectName(QStringLiteral("vbpreItemTlBtn"));
        vbpreItemTlBtn->setGeometry(QRect(109, 187, 24, 52));
        vbpreItemTlBtn->setFocusPolicy(Qt::NoFocus);
        vbpreItemTlBtn->setIcon(icon4);
        vbpreItemTlBtn->setAutoRaise(false);
        vbnextItemTlBtn = new QToolButton(vbModeTab);
        vbnextItemTlBtn->setObjectName(QStringLiteral("vbnextItemTlBtn"));
        vbnextItemTlBtn->setGeometry(QRect(541, 187, 24, 52));
        vbnextItemTlBtn->setFocusPolicy(Qt::NoFocus);
        vbnextItemTlBtn->setIcon(icon5);
        vbnextItemTlBtn->setAutoRaise(false);
        vbInput = new QLineEdit(vbModeTab);
        vbInput->setObjectName(QStringLiteral("vbInput"));
        vbInput->setGeometry(QRect(200, 39, 270, 27));
        vbInput->setFocusPolicy(Qt::TabFocus);
        vbScoreLabel = new QLabel(vbModeTab);
        vbScoreLabel->setObjectName(QStringLiteral("vbScoreLabel"));
        vbScoreLabel->setGeometry(QRect(520, 50, 67, 17));
        scrollArea = new QScrollArea(vbModeTab);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setGeometry(QRect(160, 90, 350, 250));
        scrollArea->setLineWidth(0);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 348, 248));
        verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        vbExplLabel = new QLabel(scrollAreaWidgetContents);
        vbExplLabel->setObjectName(QStringLiteral("vbExplLabel"));
        vbExplLabel->setTextFormat(Qt::PlainText);
        vbExplLabel->setWordWrap(true);

        verticalLayout->addWidget(vbExplLabel);

        scrollArea->setWidget(scrollAreaWidgetContents);
        vocabularyTabWidget->addTab(vbModeTab, QString());
        tabWidget->addTab(vocabularyTab, QString());
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(48, 48));
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionSetting);
        toolBar->addAction(actionVocabulary);
        toolBar->addSeparator();

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);
        settingTabWidget->setCurrentIndex(1);
        vocabularyTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QString());
        actionVocabulary->setText(QString());
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
        label->setText(QString());
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
        saveButton->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        netdictButton->setToolTip(QApplication::translate("MainWindow", "No action", 0));
#endif // QT_NO_TOOLTIP
        netdictButton->setText(QApplication::translate("MainWindow", "...", 0));
        queryButton->setText(QApplication::translate("MainWindow", "...", 0));
        tabWidget->setTabText(tabWidget->indexOf(dictTab), QApplication::translate("MainWindow", "dictionary", 0));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "option", 0));
        uilanLabel->setText(QApplication::translate("MainWindow", "UI Language", 0));
        uilanComboBox->clear();
        uilanComboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "English", 0)
         << QApplication::translate("MainWindow", "\346\261\211\350\257\255", 0)
        );
        settingTabWidget->setTabText(settingTabWidget->indexOf(tab), QApplication::translate("MainWindow", "general", 0));
        groupBox->setTitle(QApplication::translate("MainWindow", "capture word setting", 0));
        cwsSelectionCheckBox->setText(QApplication::translate("MainWindow", "mouse selection", 0));
        cwsClipboardCheckBox->setText(QApplication::translate("MainWindow", "clipboard", 0));
#ifndef QT_NO_TOOLTIP
        dictListWidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        dictDownToolButton->setText(QString());
        dictUpToolButton->setText(QString());
        settingTabWidget->setTabText(settingTabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "dict", 0));
        tabWidget->setTabText(tabWidget->indexOf(settingTab), QApplication::translate("MainWindow", "setting", 0));
        vbdelToolButton->setText(QApplication::translate("MainWindow", "...", 0));
        vbclearToolButton->setText(QApplication::translate("MainWindow", "...", 0));
        vocabularyTabWidget->setTabText(vocabularyTabWidget->indexOf(listModeTable), QApplication::translate("MainWindow", "list", 0));
        vbpreItemTlBtn->setText(QApplication::translate("MainWindow", "...", 0));
        vbnextItemTlBtn->setText(QApplication::translate("MainWindow", "...", 0));
        vbScoreLabel->setText(QString());
        vbExplLabel->setText(QString());
        vocabularyTabWidget->setTabText(vocabularyTabWidget->indexOf(vbModeTab), QApplication::translate("MainWindow", "exam", 0));
        tabWidget->setTabText(tabWidget->indexOf(vocabularyTab), QApplication::translate("MainWindow", "vbook", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
