/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
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
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
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
    QAction *actionHelp;
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *dictTab;
    QListView *indexListView;
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
    QComboBox *inputComboBox;
    QWidget *vocabularyTab;
    QLineEdit *vbInput;
    QRadioButton *vbExplHideradioButton;
    QTableView *vbookTableView;
    QToolButton *vbdelToolButton;
    QToolButton *vbclearToolButton;
    QLabel *vbExplLabel;
    QComboBox *vbModeComboBox;
    QToolButton *vbpreItemTlBtn;
    QToolButton *vbnextItemTlBtn;
    QLabel *vbExamScoreLabel;
    QComboBox *vbcmbRepeatCount;
    QWidget *settingTab;
    QTabWidget *settingTabWidget;
    QWidget *tab;
    QGroupBox *groupBox_3;
    QLabel *uilanLabel;
    QComboBox *uilanComboBox;
    QLabel *fontSizeLabel;
    QComboBox *fontsizeComboBox;
    QLabel *fontLabel;
    QCheckBox *systemTrayCheckBox;
    QFontComboBox *fontComboBox;
    QToolButton *resetSettingToolButton;
    QCheckBox *nonRepeatSelectionCheckBox;
    QWidget *tab_2;
    QGroupBox *cwsGroupBox;
    QGroupBox *cwsGroupBox1;
    QCheckBox *cwsEnableCheckBox;
    QGroupBox *cwsGroupBox2;
    QCheckBox *cwsSelectionCheckBox;
    QCheckBox *cwsClipboardCheckBox;
    QCheckBox *cwsMouseCheckBox;
    QGroupBox *cwsGroupBox3;
    QCheckBox *cwsAutoCloseEnCheckBox;
    QLabel *label_2;
    QComboBox *cwsShortcutkeyComboBox;
    QLabel *cwsShortcutkeylabel;
    QLabel *cwsShortcutkeylabel_2;
    QListWidget *dictListWidget;
    QToolButton *dictDownToolButton;
    QToolButton *dictUpToolButton;
    QWidget *helpTab;
    QHBoxLayout *horizontalLayout;
    QTextEdit *helpTextEdit;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(782, 548);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(782, 548));
        MainWindow->setMaximumSize(QSize(782, 548));
        MainWindow->setSizeIncrement(QSize(0, 0));
        MainWindow->setFocusPolicy(Qt::NoFocus);
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
        actionHelp = new QAction(MainWindow);
        actionHelp->setObjectName(QStringLiteral("actionHelp"));
        actionHelp->setCheckable(false);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/res/help.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHelp->setIcon(icon3);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setFocusPolicy(Qt::NoFocus);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 782, 461));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setFocusPolicy(Qt::NoFocus);
        tabWidget->setLayoutDirection(Qt::LeftToRight);
        tabWidget->setStyleSheet(QLatin1String("#tabWidget { \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px; \n"
" } "));
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tabWidget->setIconSize(QSize(20, 20));
        tabWidget->setElideMode(Qt::ElideNone);
        tabWidget->setUsesScrollButtons(false);
        dictTab = new QWidget();
        dictTab->setObjectName(QStringLiteral("dictTab"));
        indexListView = new QListView(dictTab);
        indexListView->setObjectName(QStringLiteral("indexListView"));
        indexListView->setGeometry(QRect(-1, 0, 213, 340));
        indexListView->setAcceptDrops(false);
        indexListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        indexListView->setProperty("showDropIndicator", QVariant(false));
        srcLanComboBox = new QComboBox(dictTab);
        srcLanComboBox->setObjectName(QStringLiteral("srcLanComboBox"));
        srcLanComboBox->setGeometry(QRect(236, 14, 113, 32));
        detLanComboBox = new QComboBox(dictTab);
        detLanComboBox->setObjectName(QStringLiteral("detLanComboBox"));
        detLanComboBox->setGeometry(QRect(397, 14, 112, 32));
        label = new QLabel(dictTab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(356, 21, 32, 18));
        label->setPixmap(QPixmap(QString::fromUtf8(":/res/to_right_arrow.png")));
        dictTextEdit = new QTextEdit(dictTab);
        dictTextEdit->setObjectName(QStringLiteral("dictTextEdit"));
        dictTextEdit->setGeometry(QRect(229, 70, 540, 271));
        dictTextEdit->setAcceptDrops(false);
        dictTextEdit->setReadOnly(true);
        pgupToolButton2 = new QToolButton(dictTab);
        pgupToolButton2->setObjectName(QStringLiteral("pgupToolButton2"));
        pgupToolButton2->setGeometry(QRect(9, 378, 24, 32));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/res/left_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton2->setIcon(icon4);
        pgupToolButton2->setIconSize(QSize(18, 32));
        pgupToolButton2->setAutoRaise(true);
        pgupToolButton2->setArrowType(Qt::NoArrow);
        pgupToolButton1 = new QToolButton(dictTab);
        pgupToolButton1->setObjectName(QStringLiteral("pgupToolButton1"));
        pgupToolButton1->setGeometry(QRect(47, 378, 24, 32));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/res/left_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgupToolButton1->setIcon(icon5);
        pgupToolButton1->setIconSize(QSize(18, 32));
        pgupToolButton1->setAutoRaise(true);
        pgdownToolButton1 = new QToolButton(dictTab);
        pgdownToolButton1->setObjectName(QStringLiteral("pgdownToolButton1"));
        pgdownToolButton1->setGeometry(QRect(87, 378, 24, 32));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/res/right_arrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton1->setIcon(icon6);
        pgdownToolButton1->setIconSize(QSize(18, 32));
        pgdownToolButton1->setAutoRaise(true);
        pgdownToolButton2 = new QToolButton(dictTab);
        pgdownToolButton2->setObjectName(QStringLiteral("pgdownToolButton2"));
        pgdownToolButton2->setGeometry(QRect(126, 378, 24, 32));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/res/right_arrow2.png"), QSize(), QIcon::Normal, QIcon::Off);
        pgdownToolButton2->setIcon(icon7);
        pgdownToolButton2->setIconSize(QSize(18, 32));
        pgdownToolButton2->setAutoRaise(true);
        indexLineEdit = new QLineEdit(dictTab);
        indexLineEdit->setObjectName(QStringLiteral("indexLineEdit"));
        indexLineEdit->setGeometry(QRect(10, 349, 141, 27));
        saveButton = new QToolButton(dictTab);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        saveButton->setGeometry(QRect(705, 350, 48, 48));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/res/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveButton->setIcon(icon8);
        saveButton->setIconSize(QSize(48, 48));
        saveButton->setAutoRaise(true);
        netdictButton = new QToolButton(dictTab);
        netdictButton->setObjectName(QStringLiteral("netdictButton"));
        netdictButton->setGeometry(QRect(630, 350, 48, 48));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/res/web.png"), QSize(), QIcon::Normal, QIcon::Off);
        netdictButton->setIcon(icon9);
        netdictButton->setIconSize(QSize(48, 48));
        netdictButton->setAutoRaise(true);
        queryButton = new QToolButton(dictTab);
        queryButton->setObjectName(QStringLiteral("queryButton"));
        queryButton->setGeometry(QRect(729, 6, 48, 48));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/res/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        queryButton->setIcon(icon10);
        queryButton->setIconSize(QSize(48, 48));
        queryButton->setAutoRaise(true);
        inputComboBox = new QComboBox(dictTab);
        inputComboBox->setObjectName(QStringLiteral("inputComboBox"));
        inputComboBox->setGeometry(QRect(524, 14, 200, 32));
        inputComboBox->setFocusPolicy(Qt::WheelFocus);
        inputComboBox->setEditable(true);
        tabWidget->addTab(dictTab, QString());
        vocabularyTab = new QWidget();
        vocabularyTab->setObjectName(QStringLiteral("vocabularyTab"));
        vbInput = new QLineEdit(vocabularyTab);
        vbInput->setObjectName(QStringLiteral("vbInput"));
        vbInput->setGeometry(QRect(127, 30, 290, 32));
        vbInput->setReadOnly(false);
        vbExplHideradioButton = new QRadioButton(vocabularyTab);
        vbExplHideradioButton->setObjectName(QStringLiteral("vbExplHideradioButton"));
        vbExplHideradioButton->setGeometry(QRect(670, 30, 81, 23));
        vbookTableView = new QTableView(vocabularyTab);
        vbookTableView->setObjectName(QStringLiteral("vbookTableView"));
        vbookTableView->setGeometry(QRect(70, 80, 460, 331));
        QPalette palette;
        QBrush brush(QColor(0, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush1(QColor(0, 0, 255, 128));
        brush1.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Active, QPalette::PlaceholderText, brush1);
#endif
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        QBrush brush2(QColor(0, 0, 255, 128));
        brush2.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Inactive, QPalette::PlaceholderText, brush2);
#endif
        QBrush brush3(QColor(124, 120, 119, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush3);
        QBrush brush4(QColor(0, 0, 255, 128));
        brush4.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Disabled, QPalette::PlaceholderText, brush4);
#endif
        vbookTableView->setPalette(palette);
        QFont font;
        font.setFamily(QStringLiteral("Sans Serif"));
        vbookTableView->setFont(font);
        vbookTableView->setFocusPolicy(Qt::StrongFocus);
        vbookTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        vbookTableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        vbookTableView->setSelectionMode(QAbstractItemView::SingleSelection);
        vbookTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        vbookTableView->horizontalHeader()->setVisible(false);
        vbookTableView->horizontalHeader()->setHighlightSections(true);
        vbookTableView->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        vbookTableView->verticalHeader()->setDefaultSectionSize(50);
        vbookTableView->verticalHeader()->setHighlightSections(false);
        vbdelToolButton = new QToolButton(vocabularyTab);
        vbdelToolButton->setObjectName(QStringLiteral("vbdelToolButton"));
        vbdelToolButton->setGeometry(QRect(20, 110, 32, 32));
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/res/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbdelToolButton->setIcon(icon11);
        vbdelToolButton->setIconSize(QSize(32, 32));
        vbdelToolButton->setAutoRaise(true);
        vbclearToolButton = new QToolButton(vocabularyTab);
        vbclearToolButton->setObjectName(QStringLiteral("vbclearToolButton"));
        vbclearToolButton->setGeometry(QRect(20, 160, 32, 32));
        QIcon icon12;
        icon12.addFile(QStringLiteral(":/res/clear_all.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbclearToolButton->setIcon(icon12);
        vbclearToolButton->setIconSize(QSize(32, 32));
        vbclearToolButton->setAutoRaise(true);
        vbExplLabel = new QLabel(vocabularyTab);
        vbExplLabel->setObjectName(QStringLiteral("vbExplLabel"));
        vbExplLabel->setGeometry(QRect(555, 80, 205, 320));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush5(QColor(0, 0, 255, 128));
        brush5.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette1.setBrush(QPalette::Active, QPalette::PlaceholderText, brush5);
#endif
        palette1.setBrush(QPalette::Inactive, QPalette::Text, brush);
        QBrush brush6(QColor(0, 0, 255, 128));
        brush6.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette1.setBrush(QPalette::Inactive, QPalette::PlaceholderText, brush6);
#endif
        palette1.setBrush(QPalette::Disabled, QPalette::Text, brush3);
        QBrush brush7(QColor(0, 0, 255, 128));
        brush7.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette1.setBrush(QPalette::Disabled, QPalette::PlaceholderText, brush7);
#endif
        vbExplLabel->setPalette(palette1);
        vbExplLabel->setFont(font);
        vbExplLabel->setScaledContents(false);
        vbExplLabel->setWordWrap(true);
        vbModeComboBox = new QComboBox(vocabularyTab);
        vbModeComboBox->setObjectName(QStringLiteral("vbModeComboBox"));
        vbModeComboBox->setGeometry(QRect(20, 30, 88, 32));
        vbpreItemTlBtn = new QToolButton(vocabularyTab);
        vbpreItemTlBtn->setObjectName(QStringLiteral("vbpreItemTlBtn"));
        vbpreItemTlBtn->setGeometry(QRect(17, 210, 38, 38));
        QIcon icon13;
        icon13.addFile(QStringLiteral(":/res/up-48x48.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbpreItemTlBtn->setIcon(icon13);
        vbpreItemTlBtn->setIconSize(QSize(48, 48));
        vbpreItemTlBtn->setAutoRaise(true);
        vbnextItemTlBtn = new QToolButton(vocabularyTab);
        vbnextItemTlBtn->setObjectName(QStringLiteral("vbnextItemTlBtn"));
        vbnextItemTlBtn->setGeometry(QRect(18, 266, 38, 38));
        QIcon icon14;
        icon14.addFile(QStringLiteral(":/res/down-48x48.png"), QSize(), QIcon::Normal, QIcon::Off);
        vbnextItemTlBtn->setIcon(icon14);
        vbnextItemTlBtn->setIconSize(QSize(48, 48));
        vbnextItemTlBtn->setAutoRaise(true);
        vbExamScoreLabel = new QLabel(vocabularyTab);
        vbExamScoreLabel->setObjectName(QStringLiteral("vbExamScoreLabel"));
        vbExamScoreLabel->setGeometry(QRect(531, 30, 109, 25));
        vbcmbRepeatCount = new QComboBox(vocabularyTab);
        vbcmbRepeatCount->setObjectName(QStringLiteral("vbcmbRepeatCount"));
        vbcmbRepeatCount->setGeometry(QRect(438, 30, 61, 31));
        vbcmbRepeatCount->setLayoutDirection(Qt::LeftToRight);
        vbcmbRepeatCount->setEditable(false);
        tabWidget->addTab(vocabularyTab, QString());
        settingTab = new QWidget();
        settingTab->setObjectName(QStringLiteral("settingTab"));
        settingTab->setEnabled(true);
        settingTabWidget = new QTabWidget(settingTab);
        settingTabWidget->setObjectName(QStringLiteral("settingTabWidget"));
        settingTabWidget->setGeometry(QRect(0, 0, 777, 410));
        settingTabWidget->setFocusPolicy(Qt::NoFocus);
        settingTabWidget->setStyleSheet(QLatin1String("#setting_Tab { \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px; \n"
" } "));
        settingTabWidget->setTabPosition(QTabWidget::West);
        settingTabWidget->setElideMode(Qt::ElideNone);
        settingTabWidget->setDocumentMode(true);
        settingTabWidget->setTabsClosable(false);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        tab->setStyleSheet(QLatin1String("#tab { \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px; \n"
" } "));
        groupBox_3 = new QGroupBox(tab);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(0, 0, 380, 406));
        groupBox_3->setStyleSheet(QLatin1String("#groupBox_3 { \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px; \n"
" } "));
        uilanLabel = new QLabel(groupBox_3);
        uilanLabel->setObjectName(QStringLiteral("uilanLabel"));
        uilanLabel->setGeometry(QRect(25, 60, 118, 26));
        uilanComboBox = new QComboBox(groupBox_3);
        uilanComboBox->setObjectName(QStringLiteral("uilanComboBox"));
        uilanComboBox->setGeometry(QRect(149, 60, 210, 27));
        fontSizeLabel = new QLabel(groupBox_3);
        fontSizeLabel->setObjectName(QStringLiteral("fontSizeLabel"));
        fontSizeLabel->setGeometry(QRect(25, 161, 102, 17));
        fontsizeComboBox = new QComboBox(groupBox_3);
        fontsizeComboBox->setObjectName(QStringLiteral("fontsizeComboBox"));
        fontsizeComboBox->setGeometry(QRect(147, 156, 211, 27));
        fontLabel = new QLabel(groupBox_3);
        fontLabel->setObjectName(QStringLiteral("fontLabel"));
        fontLabel->setGeometry(QRect(25, 113, 87, 17));
        systemTrayCheckBox = new QCheckBox(groupBox_3);
        systemTrayCheckBox->setObjectName(QStringLiteral("systemTrayCheckBox"));
        systemTrayCheckBox->setGeometry(QRect(25, 290, 157, 25));
        systemTrayCheckBox->setLayoutDirection(Qt::LeftToRight);
        systemTrayCheckBox->setTristate(false);
        fontComboBox = new QFontComboBox(groupBox_3);
        fontComboBox->setObjectName(QStringLiteral("fontComboBox"));
        fontComboBox->setEnabled(true);
        fontComboBox->setGeometry(QRect(146, 107, 209, 27));
        fontComboBox->setEditable(false);
        resetSettingToolButton = new QToolButton(groupBox_3);
        resetSettingToolButton->setObjectName(QStringLiteral("resetSettingToolButton"));
        resetSettingToolButton->setGeometry(QRect(10, 340, 318, 31));
        nonRepeatSelectionCheckBox = new QCheckBox(groupBox_3);
        nonRepeatSelectionCheckBox->setObjectName(QStringLiteral("nonRepeatSelectionCheckBox"));
        nonRepeatSelectionCheckBox->setGeometry(QRect(25, 210, 222, 25));
        nonRepeatSelectionCheckBox->setLayoutDirection(Qt::LeftToRight);
        nonRepeatSelectionCheckBox->setTristate(false);
        settingTabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tab_2->setStyleSheet(QLatin1String("#tab_2 { \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px; \n"
" } "));
        cwsGroupBox = new QGroupBox(tab_2);
        cwsGroupBox->setObjectName(QStringLiteral("cwsGroupBox"));
        cwsGroupBox->setEnabled(true);
        cwsGroupBox->setGeometry(QRect(309, 10, 424, 371));
        cwsGroupBox->setAutoFillBackground(false);
        cwsGroupBox->setStyleSheet(QLatin1String("#cwsGroupBox { \n"
"     border: 1px solid gray; \n"
"     border-radius: 1px; \n"
" } "));
        cwsGroupBox->setCheckable(false);
        cwsGroupBox->setChecked(false);
        cwsGroupBox1 = new QGroupBox(cwsGroupBox);
        cwsGroupBox1->setObjectName(QStringLiteral("cwsGroupBox1"));
        cwsGroupBox1->setGeometry(QRect(10, 40, 384, 101));
        cwsGroupBox1->setAutoFillBackground(false);
        cwsGroupBox1->setStyleSheet(QLatin1String("#cwsGroupBox1{ \n"
"     border: 0px solid gray; \n"
"     border-radius: 0px;\n"
" }"));
        cwsGroupBox1->setFlat(false);
        cwsGroupBox1->setCheckable(false);
        cwsEnableCheckBox = new QCheckBox(cwsGroupBox1);
        cwsEnableCheckBox->setObjectName(QStringLiteral("cwsEnableCheckBox"));
        cwsEnableCheckBox->setGeometry(QRect(8, 38, 91, 22));
        cwsGroupBox2 = new QGroupBox(cwsGroupBox);
        cwsGroupBox2->setObjectName(QStringLiteral("cwsGroupBox2"));
        cwsGroupBox2->setGeometry(QRect(10, 160, 385, 71));
        cwsGroupBox2->setStyleSheet(QLatin1String("#cwsGroupBox2{ \n"
"     border: 0px solid gray; \n"
"     border-radius: 1px; \n"
" } "));
        cwsGroupBox2->setFlat(false);
        cwsSelectionCheckBox = new QCheckBox(cwsGroupBox2);
        cwsSelectionCheckBox->setObjectName(QStringLiteral("cwsSelectionCheckBox"));
        cwsSelectionCheckBox->setGeometry(QRect(9, 38, 177, 22));
        cwsClipboardCheckBox = new QCheckBox(cwsGroupBox2);
        cwsClipboardCheckBox->setObjectName(QStringLiteral("cwsClipboardCheckBox"));
        cwsClipboardCheckBox->setGeometry(QRect(218, 38, 129, 22));
        cwsMouseCheckBox = new QCheckBox(cwsGroupBox2);
        cwsMouseCheckBox->setObjectName(QStringLiteral("cwsMouseCheckBox"));
        cwsMouseCheckBox->setEnabled(true);
        cwsMouseCheckBox->setGeometry(QRect(335, 38, 63, 20));
        cwsMouseCheckBox->setCheckable(true);
        cwsGroupBox3 = new QGroupBox(cwsGroupBox);
        cwsGroupBox3->setObjectName(QStringLiteral("cwsGroupBox3"));
        cwsGroupBox3->setGeometry(QRect(10, 261, 380, 80));
        cwsGroupBox3->setStyleSheet(QLatin1String("#cwsGroupBox3{ \n"
"     border: 0px solid gray; \n"
"     border-radius: 1px; \n"
" } "));
        cwsAutoCloseEnCheckBox = new QCheckBox(cwsGroupBox3);
        cwsAutoCloseEnCheckBox->setObjectName(QStringLiteral("cwsAutoCloseEnCheckBox"));
        cwsAutoCloseEnCheckBox->setGeometry(QRect(9, 38, 133, 21));
        label_2 = new QLabel(cwsGroupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(382, 110, 16, 17));
        cwsShortcutkeyComboBox = new QComboBox(cwsGroupBox);
        cwsShortcutkeyComboBox->setObjectName(QStringLiteral("cwsShortcutkeyComboBox"));
        cwsShortcutkeyComboBox->setGeometry(QRect(282, 105, 85, 27));
        cwsShortcutkeyComboBox->setEditable(false);
        cwsShortcutkeylabel = new QLabel(cwsGroupBox);
        cwsShortcutkeylabel->setObjectName(QStringLiteral("cwsShortcutkeylabel"));
        cwsShortcutkeylabel->setGeometry(QRect(22, 108, 131, 27));
        cwsShortcutkeylabel_2 = new QLabel(cwsGroupBox);
        cwsShortcutkeylabel_2->setObjectName(QStringLiteral("cwsShortcutkeylabel_2"));
        cwsShortcutkeylabel_2->setGeometry(QRect(158, 108, 121, 20));
        dictListWidget = new QListWidget(tab_2);
        dictListWidget->setObjectName(QStringLiteral("dictListWidget"));
        dictListWidget->setGeometry(QRect(20, 10, 261, 349));
        dictListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        dictListWidget->setProperty("showDropIndicator", QVariant(false));
        dictListWidget->setDragEnabled(false);
        dictListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
        dictDownToolButton = new QToolButton(tab_2);
        dictDownToolButton->setObjectName(QStringLiteral("dictDownToolButton"));
        dictDownToolButton->setGeometry(QRect(185, 370, 59, 30));
        dictDownToolButton->setFocusPolicy(Qt::ClickFocus);
        QIcon icon15;
        icon15.addFile(QStringLiteral(":/res/down.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictDownToolButton->setIcon(icon15);
        dictDownToolButton->setIconSize(QSize(32, 32));
        dictDownToolButton->setAutoRaise(false);
        dictUpToolButton = new QToolButton(tab_2);
        dictUpToolButton->setObjectName(QStringLiteral("dictUpToolButton"));
        dictUpToolButton->setGeometry(QRect(49, 370, 59, 30));
        dictUpToolButton->setFocusPolicy(Qt::ClickFocus);
        QIcon icon16;
        icon16.addFile(QStringLiteral(":/res/up.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictUpToolButton->setIcon(icon16);
        dictUpToolButton->setIconSize(QSize(32, 32));
        dictUpToolButton->setAutoRaise(false);
        settingTabWidget->addTab(tab_2, QString());
        tabWidget->addTab(settingTab, QString());
        helpTab = new QWidget();
        helpTab->setObjectName(QStringLiteral("helpTab"));
        horizontalLayout = new QHBoxLayout(helpTab);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        helpTextEdit = new QTextEdit(helpTab);
        helpTextEdit->setObjectName(QStringLiteral("helpTextEdit"));
        helpTextEdit->setReadOnly(true);

        horizontalLayout->addWidget(helpTextEdit);

        tabWidget->addTab(helpTab, QString());
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

        toolBar->addAction(actionVocabulary);
        toolBar->addAction(actionSetting);
        toolBar->addAction(actionHelp);
        toolBar->addSeparator();

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);
        settingTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QString());
        actionVocabulary->setText(QString());
#ifndef QT_NO_TOOLTIP
        actionVocabulary->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Vocabulary book</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        actionSetting->setText(QString());
#ifndef QT_NO_TOOLTIP
        actionSetting->setToolTip(QApplication::translate("MainWindow", "Setting", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        actionHelp->setText(QString());
#ifndef QT_NO_TOOLTIP
        actionHelp->setToolTip(QApplication::translate("MainWindow", "Help", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        indexListView->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_ACCESSIBILITY
        indexListView->setAccessibleName(QString());
#endif // QT_NO_ACCESSIBILITY
#ifndef QT_NO_TOOLTIP
        srcLanComboBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Select source language.</p><p>'any' means not specified.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        detLanComboBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Select destination language.</p><p>'any' means not specified.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        label->setText(QString());
        pgupToolButton2->setText(QString());
        pgupToolButton1->setText(QString());
        pgdownToolButton1->setText(QString());
        pgdownToolButton2->setText(QString());
#ifndef QT_NO_TOOLTIP
        indexLineEdit->setToolTip(QApplication::translate("MainWindow", "Filter by a prefix", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        indexLineEdit->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        indexLineEdit->setPlaceholderText(QString());
#ifndef QT_NO_TOOLTIP
        saveButton->setToolTip(QApplication::translate("MainWindow", "Add to vocabulary book", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        saveButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        netdictButton->setToolTip(QApplication::translate("MainWindow", "No action", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        netdictButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        queryButton->setToolTip(QApplication::translate("MainWindow", "Lookup", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        queryButton->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(dictTab), QApplication::translate("MainWindow", "Dictionary", Q_NULLPTR));
        vbExplHideradioButton->setText(QApplication::translate("MainWindow", "Hide", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        vbdelToolButton->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Delete a item</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        vbdelToolButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        vbclearToolButton->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Delete all items</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        vbclearToolButton->setText(QString());
        vbExplLabel->setText(QString());
#ifndef QT_NO_TOOLTIP
        vbpreItemTlBtn->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Move  up a  item</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        vbpreItemTlBtn->setText(QString());
#ifndef QT_NO_TOOLTIP
        vbnextItemTlBtn->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Move down a item</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        vbnextItemTlBtn->setText(QString());
        vbExamScoreLabel->setText(QString());
        vbcmbRepeatCount->clear();
        vbcmbRepeatCount->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "3", Q_NULLPTR)
         << QApplication::translate("MainWindow", "5", Q_NULLPTR)
         << QApplication::translate("MainWindow", "8", Q_NULLPTR)
         << QApplication::translate("MainWindow", "12", Q_NULLPTR)
         << QApplication::translate("MainWindow", "20", Q_NULLPTR)
         << QApplication::translate("MainWindow", "100", Q_NULLPTR)
        );
#ifndef QT_NO_TOOLTIP
        vbcmbRepeatCount->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>How many times for studying a word</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        vbcmbRepeatCount->setCurrentText(QApplication::translate("MainWindow", "3", "0"));
        tabWidget->setTabText(tabWidget->indexOf(vocabularyTab), QApplication::translate("MainWindow", "VocabularyBook", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        groupBox_3->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>After double click selection on <span style=\" color:#0000ff;\">emacs</span>, left button click will do the selection again,<span style=\" color:#ff0000;\"> temporarily turn on </span> this switch  can prevent this situation.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Option", Q_NULLPTR));
        uilanLabel->setText(QApplication::translate("MainWindow", "UI Language", Q_NULLPTR));
        uilanComboBox->clear();
        uilanComboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "English", Q_NULLPTR)
         << QApplication::translate("MainWindow", "\346\261\211\350\257\255", Q_NULLPTR)
        );
#ifndef QT_NO_TOOLTIP
        uilanComboBox->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        fontSizeLabel->setText(QApplication::translate("MainWindow", "Font Size", Q_NULLPTR));
        fontsizeComboBox->clear();
        fontsizeComboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "9", Q_NULLPTR)
         << QApplication::translate("MainWindow", "10", Q_NULLPTR)
         << QApplication::translate("MainWindow", "11", Q_NULLPTR)
        );
#ifndef QT_NO_TOOLTIP
        fontsizeComboBox->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        fontLabel->setText(QApplication::translate("MainWindow", "Font", Q_NULLPTR));
        systemTrayCheckBox->setText(QApplication::translate("MainWindow", "System Tray ", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        fontComboBox->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        resetSettingToolButton->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        resetSettingToolButton->setText(QApplication::translate("MainWindow", "Restore to Default Setting", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        nonRepeatSelectionCheckBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>After double click selection on <span style=\" color:#0000ff;\">Emacs,</span> left click will do the selection again,<span style=\" color:#0000ff;\"/><span style=\" color:#ff0000;\">turn on</span> this switch can prevent this situation\343\200\202</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        nonRepeatSelectionCheckBox->setText(QApplication::translate("MainWindow", "Non-repeat selection", Q_NULLPTR));
        settingTabWidget->setTabText(settingTabWidget->indexOf(tab), QApplication::translate("MainWindow", "General", Q_NULLPTR));
        cwsGroupBox->setTitle(QApplication::translate("MainWindow", "Capture Word Setting", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsGroupBox1->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        cwsGroupBox1->setTitle(QApplication::translate("MainWindow", "Enable Setting", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsEnableCheckBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Enable/disable capture</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsEnableCheckBox->setText(QApplication::translate("MainWindow", "Enable", Q_NULLPTR));
        cwsGroupBox2->setTitle(QApplication::translate("MainWindow", "Capture Method", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsSelectionCheckBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Capture by mouse selection or double click text (highlighted text).</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsSelectionCheckBox->setText(QApplication::translate("MainWindow", "Mouse Selection", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsClipboardCheckBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Capture word of clipboard.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsClipboardCheckBox->setText(QApplication::translate("MainWindow", "Clipboard", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsMouseCheckBox->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Capture word by mouse over.</p><p>Not support  windows_x64  yet.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsMouseCheckBox->setText(QApplication::translate("MainWindow", "Mouse Over", Q_NULLPTR));
        cwsGroupBox3->setTitle(QApplication::translate("MainWindow", "Capture Dialog", Q_NULLPTR));
        cwsAutoCloseEnCheckBox->setText(QApplication::translate("MainWindow", "Autoclose", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", ">", Q_NULLPTR));
        cwsShortcutkeyComboBox->clear();
        cwsShortcutkeyComboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "KEY_A", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_B", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_C", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_D", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_E", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_F", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_G", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_H", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_I", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_J", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_K", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_L", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_M", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_N", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_O", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_P", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_Q", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_R", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_S", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_T", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_U", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_V", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_W", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_X", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_Y", Q_NULLPTR)
         << QApplication::translate("MainWindow", "KEY_Z", Q_NULLPTR)
        );
        cwsShortcutkeyComboBox->setCurrentText(QApplication::translate("MainWindow", "KEY_A", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsShortcutkeylabel->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">User can use this 'Shortcut Key' disable or enable capture temporarily, It don't change 'enable' state. </p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">AlphaDict uses Ctrl+Alt as a modifier, choose a hotkey from key 'a' to 'z' (not casp lock).</p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">On X11, if  shortcut key does't work, you can try clicking AlphaDict window.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsShortcutkeylabel->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Shortcut Key</span></p></body></html>", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        cwsShortcutkeylabel_2->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">User can use this 'Shortcut Key' disable or enable capture temporarily, It don't change 'enable' state. </p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">AlphaDict uses Ctrl+Alt as a modifier, choose a hotkey from key 'a' to 'z' (not casp lock).</p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">On X11, if  shortcut key does't work, you can try clicking AlphaDict window.</p></body></html>", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        cwsShortcutkeylabel_2->setText(QApplication::translate("MainWindow", "<html><head/><body><p>&lt;Ctrl + Alt +</p></body></html>", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        dictListWidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        dictDownToolButton->setText(QString());
        dictUpToolButton->setText(QString());
        settingTabWidget->setTabText(settingTabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "Dict", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(settingTab), QApplication::translate("MainWindow", "Setting", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(helpTab), QApplication::translate("MainWindow", "Help", Q_NULLPTR));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
