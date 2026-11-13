/*****************************************************************************
 *  AlphaDict
 *  Copyright (C) 2004-2026  AlphaDict project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#ifndef _QMLBRIDGE_H_
#define _QMLBRIDGE_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>
#include <QtCore/QMap>

#include "dict/iDict.h"
#include "DictIndexModel.h"
#include "DictListModel.h"

class Configure;
class VBookModel;

/*! The single object QML talks to.
 *
 *  It owns the list models, translates QML invocations into the message
 *  queue and turns the messages coming back from QtMessager into plain
 *  properties QML can bind to.  Window behaviour (move/hide/quit) is
 *  requested through signals so that the bridge stays free of widgets.
 */
class QmlBridge : public QObject
{
    Q_OBJECT

    /* QObject* instead of the concrete model pointers: a Q_PROPERTY whose type
     * is a class pointer needs qRegisterMetaType() to be readable by
     * QMetaProperty::read() (which is what QML uses).  Both models are only
     * consumed as ListView models from QML, so the base pointer is enough. */
    Q_PROPERTY(QObject* indexModel READ indexModel CONSTANT)
    Q_PROPERTY(QObject* dictModel  READ dictModel  CONSTANT)

    Q_PROPERTY(QVariantList expl      READ expl      NOTIFY explChanged)
    Q_PROPERTY(QString      query     READ query     NOTIFY queryChanged)
    Q_PROPERTY(QVariantList capWord   READ capWord   NOTIFY capWordChanged)

    Q_PROPERTY(int indexCount READ indexCount NOTIFY indexCountChanged)
    Q_PROPERTY(int dictCount  READ dictCount  NOTIFY dictCountChanged)

    /*! Whether the QML side shows the index.  While hidden the model is not
     *  filled: a language switch must not rebuild a list nobody reads. */
    Q_PROPERTY(bool indexVisible READ indexVisible WRITE setIndexVisible
               NOTIFY indexVisibleChanged)

    Q_PROPERTY(QVariantList vbook     READ vbook     NOTIFY vbookChanged)
    Q_PROPERTY(int          vBookMode READ vBookMode WRITE setVBookMode NOTIFY vBookModeChanged)
    Q_PROPERTY(int          vBookRow  READ vBookRow  NOTIFY vbookChanged)
    Q_PROPERTY(QString      vBookWord READ vBookWord NOTIFY vbookChanged)
    Q_PROPERTY(QString      vBookExpl READ vBookExpl NOTIFY vbookChanged)

    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString helpText   READ helpText   NOTIFY helpTextChanged)

    Q_PROPERTY(int     fontSize   READ fontSize   WRITE setFontSize   NOTIFY fontSizeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    /* slider bounds live in Configure.h */
    Q_PROPERTY(int minFontSize     READ minFontSize     CONSTANT)
    Q_PROPERTY(int maxFontSize     READ maxFontSize     CONSTANT)
    Q_PROPERTY(int defaultFontSize READ defaultFontSize CONSTANT)
    Q_PROPERTY(bool    hasDict    READ hasDict    NOTIFY dictStateChanged)
    /* current dictionary summary + entry count, shown in the status bar */
    Q_PROPERTY(QString dictInfo   READ dictInfo   NOTIFY dictStateChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    /* data directory actually in use, shown by the settings page */
    Q_PROPERTY(QString dataDir    READ dataDir    CONSTANT)

    /* settings page */
    Q_PROPERTY(bool systemTray   READ systemTray   WRITE setSystemTray   NOTIFY systemTrayChanged)
    /*! Theme: 0 automatic (by clock), 1 light, 2 dark.  QML computes the
     *  actual palette in Theme.qml; the bridge only persists the choice. */
    Q_PROPERTY(int  themeMode    READ themeMode    WRITE setThemeMode    NOTIFY themeModeChanged)
    /*! 取词窗（CapWordDialog）跟主界面共用主题，QML 侧把 Theme.dark 同步过来。 */
    Q_PROPERTY(bool darkTheme    READ darkTheme    WRITE setDarkTheme    NOTIFY darkThemeChanged)
    Q_PROPERTY(bool cwsEnabled   READ cwsEnabled   WRITE setCwsEnabled   NOTIFY cwsChanged)
    Q_PROPERTY(bool cwsClipboard READ cwsClipboard WRITE setCwsClipboard NOTIFY cwsChanged)
    Q_PROPERTY(bool cwsSelection READ cwsSelection WRITE setCwsSelection NOTIFY cwsChanged)
    Q_PROPERTY(bool autoCloseEn  READ autoCloseEn  WRITE setAutoCloseEn  NOTIFY cwsChanged)
    Q_PROPERTY(int  shortcutIndex READ shortcutIndex WRITE setShortcutIndex NOTIFY cwsChanged)
    Q_PROPERTY(QStringList shortcutKeys READ shortcutKeys CONSTANT)
    Q_PROPERTY(int uiLanID READ uiLanID WRITE setUiLanID NOTIFY uiLanChanged)
    Q_PROPERTY(QStringList uiLans READ uiLans CONSTANT)

public:
    explicit QmlBridge(QObject *parent = NULL);
    virtual ~QmlBridge();

    DictIndexModel* indexModel() const { return m_indexModel; }
    DictListModel*  dictModel()  const { return m_dictModel; }

    QVariantList expl() const { return m_expl; }
    QString      query() const { return m_query; }
    QVariantList capWord() const { return m_capWord; }

    int indexCount() const;
    bool indexVisible() const { return m_indexVisible; }
    void setIndexVisible(bool visible);
    QString dataDir() const;
    int dictCount() const;

    QVariantList vbook() const { return m_vbook; }
    int          vBookMode() const;
    int          vBookRow() const;
    QString      vBookWord() const;
    QString      vBookExpl() const { return m_vBookExpl; }

    QString statusText() const { return m_statusText; }
    QString helpText() const { return m_helpText; }

    int     fontSize() const;
    int     minFontSize() const;
    int     maxFontSize() const;
    int     defaultFontSize() const;
    QString fontFamily() const;
    bool    hasDict() const;
    QString dictInfo() const;
    QString appVersion() const;

    bool systemTray() const;
    int  themeMode() const;
    bool darkTheme() const { return m_darkTheme; }
    bool cwsEnabled() const;
    bool cwsClipboard() const;
    bool cwsSelection() const;
    bool autoCloseEn() const;
    int  shortcutIndex() const;
    QStringList shortcutKeys() const;
    int  uiLanID() const;
    QStringList uiLans() const;

    /*! The raw list of the last captured-word lookup.  Ownership stays here,
     *  the caller must not delete it. */
    DictItemList* capWordItems() const { return m_capItems; }

    /*! Hand the captured-word list over to the caller (usually the popup
     *  dialog, which deletes it).  The bridge forgets it. */
    DictItemList* takeCapWordItems() { DictItemList *p = m_capItems; m_capItems = NULL; return p; }

public slots:
    /* Called by QtMessager (queued), never from a worker thread directly. */
    void onUpdateExplText(int which, void *v);
    void onUpdateCapWordExplText(void *v);
    /*! Called by QtMessager right after a (re)load rebuilt the index, i.e.
     *  the displayed results belong to a dictionary set that is gone.  Not
     *  driven by modelReset(): prefix search and paging reset the same model
     *  and must keep the results on screen. */
    void onDictReloaded();

private slots:
    void onVBookModelReset();

signals:
    void explChanged();
    void queryChanged();
    void capWordChanged();
    void indexCountChanged();
    void indexVisibleChanged();
    void dictCountChanged();
    void vbookChanged();
    void vBookModeChanged();
    void statusTextChanged();
    void helpTextChanged();
    void fontSizeChanged();
    void fontFamilyChanged();
    void dictStateChanged();
    void systemTrayChanged();
    void themeModeChanged();
    void darkThemeChanged();
    void cwsChanged();
    void uiLanChanged();

    /* window requests */
    void requestBeginWindowMove();
    /*! The index column was shown or hidden; widthDelta/heightDelta are the
     *  space it takes, so the window can grow and shrink by the same amount. */
    void requestIndexPane(bool open, int widthDelta, int heightDelta);
    void requestBeginWindowResize(int edges);
    void requestMinimizeToTray();
    /*! same path as the window manager's close button */
    void requestCloseWindow();
    void requestQuit();
    void capWordShown();
    void requestSystemTray(bool en);
    void requestCwsEnable(bool en);
    void requestShortcutKey(int oldIndex, int newIndex);

public:
    /*! Applies the index column's size to the window; the deltas come from
     *  the QML layout, which is the only side that knows them. */
    Q_INVOKABLE void fitIndexPane(bool open, int widthDelta, int heightDelta);

    Q_INVOKABLE void search(const QString &input);
    Q_INVOKABLE void prefixSearch(const QString &prefix);
    Q_INVOKABLE void clickIndex(int row);
    Q_INVOKABLE int  pageIndex(int pages);
    Q_INVOKABLE int  indexRowCount() const;

    Q_INVOKABLE void addToVBook(const QString &word);
    /*! Same as addToVBook() but tells the caller whether the word was
     *  really added (false when it is empty or already known). */
    bool addToVBookChecked(const QString &word);
    /*! True when the word is already in the vocabulary book. */
    Q_INVOKABLE bool inVBook(const QString &word) const;
    /*! Add the word, or drop it when it is already there. */
    Q_INVOKABLE void toggleVBook(const QString &word);
    Q_INVOKABLE void removeVBookRow(int row);
    Q_INVOKABLE void clearVBook();
    Q_INVOKABLE void selectVBookRow(int row);
    Q_INVOKABLE void vbookStep(int step);
    Q_INVOKABLE void vbookInput(const QString &input, int repeat);

    Q_INVOKABLE void moveDictRow(int row, int offset);
    Q_INVOKABLE void setDictEnabled(int row, bool en);

    /*! A frameless window has no title bar.  startSystemMove() is unreliable
     *  inside QQuickWidget, so the window follows the cursor from the event
     *  filter instead (see QmlMainWindow::eventFilter). */
    Q_INVOKABLE void beginWindowMove();
    Q_INVOKABLE void beginWindowResize(int edges);
    Q_INVOKABLE void minimizeToTray();
    Q_INVOKABLE void closeWindow();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void refreshHelpText();
    Q_INVOKABLE void setStatus(const QString &text);
    Q_INVOKABLE void setUiLanID(int id);
    Q_INVOKABLE void resetSettings();

    /* property writers */
    void setFontSize(int size);
    void setFontFamily(const QString &family);
    void setVBookMode(int mode);
    void setSystemTray(bool en);
    void setThemeMode(int mode);
    void setDarkTheme(bool dark);
    void setCwsEnabled(bool en);
    void setCwsClipboard(bool en);
    void setCwsSelection(bool en);
    void setAutoCloseEn(bool en);
    void setShortcutIndex(int index);

private:
    /*! apply the configured family and size application-wide */
    void applyUiFont();

    Configure *m_config;
    DictIndexModel *m_indexModel;
    DictListModel  *m_dictModel;
    VBookModel     *m_vbookModel;

    QVariantList m_expl;
    /*! last result of every dictionary, keyed by its index, so several
     *  dictionaries can be shown one after another in dictionary-list order. */
    QMap<int, QVariantList> m_explByDict;
    QVariantList m_capWord;
    QString      m_query;
    /*! Last prefix the QML side asked for; replayed once the index is shown. */
    QString      m_indexPrefix;
    bool         m_indexVisible;
    QVariantList m_vbook;
    QString      m_vBookExpl;
    QString      m_statusText;
    QString      m_helpText;

    /*! used when the configured family is empty ("system default") */
    QString      m_defaultFontFamily;

    DictItemList *m_capItems;
    bool m_darkTheme;

    void refreshVBook();
};

#endif // _QMLBRIDGE_H_
