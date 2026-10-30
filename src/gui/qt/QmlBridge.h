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

    Q_PROPERTY(QStringList srcLans READ srcLans NOTIFY languagesChanged)
    Q_PROPERTY(QStringList detLans READ detLans NOTIFY languagesChanged)
    Q_PROPERTY(QString srcLan READ srcLan WRITE setSrcLan NOTIFY languagesChanged)
    Q_PROPERTY(QString detLan READ detLan WRITE setDetLan NOTIFY languagesChanged)

    Q_PROPERTY(QVariantList vbook     READ vbook     NOTIFY vbookChanged)
    Q_PROPERTY(int          vBookMode READ vBookMode WRITE setVBookMode NOTIFY vBookModeChanged)
    Q_PROPERTY(int          vBookRow  READ vBookRow  NOTIFY vbookChanged)
    Q_PROPERTY(QString      vBookWord READ vBookWord NOTIFY vbookChanged)
    Q_PROPERTY(QString      vBookExpl READ vBookExpl NOTIFY vbookChanged)

    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString helpText   READ helpText   NOTIFY helpTextChanged)

    Q_PROPERTY(int     fontSize   READ fontSize   WRITE setFontSize   NOTIFY fontSizeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(bool    hasDict    READ hasDict    NOTIFY dictStateChanged)
    /* data directory actually in use, shown by the settings page */
    Q_PROPERTY(QString dataDir    READ dataDir    CONSTANT)

    /* settings page */
    Q_PROPERTY(bool systemTray   READ systemTray   WRITE setSystemTray   NOTIFY systemTrayChanged)
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
    QString dataDir() const;
    int dictCount() const;

    QStringList srcLans() const { return m_srcLans; }
    QStringList detLans() const { return m_detLans; }
    QString srcLan() const { return m_srcLan; }
    QString detLan() const { return m_detLan; }

    QVariantList vbook() const { return m_vbook; }
    int          vBookMode() const;
    int          vBookRow() const;
    QString      vBookWord() const;
    QString      vBookExpl() const { return m_vBookExpl; }

    QString statusText() const { return m_statusText; }
    QString helpText() const { return m_helpText; }

    int     fontSize() const;
    QString fontFamily() const;
    bool    hasDict() const;

    bool systemTray() const;
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
    void onUpdateExplText(void *v);
    void onUpdateCapWordExplText(void *v);
    void onSetLanComboBox(const QString &src, const QString &det, void *v);

private slots:
    void onVBookModelReset();

signals:
    void explChanged();
    void queryChanged();
    void capWordChanged();
    void indexCountChanged();
    void dictCountChanged();
    void languagesChanged();
    void vbookChanged();
    void vBookModeChanged();
    void statusTextChanged();
    void helpTextChanged();
    void fontSizeChanged();
    void fontFamilyChanged();
    void dictStateChanged();
    void systemTrayChanged();
    void cwsChanged();
    void uiLanChanged();

    /* window requests */
    void requestMove(int dx, int dy);
    void requestMinimizeToTray();
    void requestQuit();
    void capWordShown();
    void requestSystemTray(bool en);
    void requestCwsEnable(bool en);
    void requestShortcutKey(int oldIndex, int newIndex);

public:
    Q_INVOKABLE void search(const QString &input);
    Q_INVOKABLE void prefixSearch(const QString &prefix);
    Q_INVOKABLE void clickIndex(int row);
    Q_INVOKABLE int  pageIndex(int pages);
    Q_INVOKABLE int  indexRowCount() const;

    Q_INVOKABLE void addToVBook(const QString &word);
    /*! Same as addToVBook() but tells the caller whether the word was
     *  really added (false when it is empty or already known). */
    bool addToVBookChecked(const QString &word);
    Q_INVOKABLE void removeVBookRow(int row);
    Q_INVOKABLE void clearVBook();
    Q_INVOKABLE void selectVBookRow(int row);
    Q_INVOKABLE void vbookStep(int step);
    Q_INVOKABLE void vbookInput(const QString &input, int repeat);

    Q_INVOKABLE void moveDictRow(int row, int offset);
    Q_INVOKABLE void setDictEnabled(int row, bool en);

    Q_INVOKABLE void moveWindowBy(int dx, int dy);
    Q_INVOKABLE void minimizeToTray();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void refreshHelpText();
    Q_INVOKABLE void setStatus(const QString &text);
    Q_INVOKABLE void setUiLanID(int id);
    Q_INVOKABLE void resetSettings();

    /* property writers */
    void setSrcLan(const QString &lan);
    void setDetLan(const QString &lan);
    void setFontSize(int size);
    void setFontFamily(const QString &family);
    void setVBookMode(int mode);
    void setSystemTray(bool en);
    void setCwsEnabled(bool en);
    void setCwsClipboard(bool en);
    void setCwsSelection(bool en);
    void setAutoCloseEn(bool en);
    void setShortcutIndex(int index);

private:
    Configure *m_config;
    DictIndexModel *m_indexModel;
    DictListModel  *m_dictModel;
    VBookModel     *m_vbookModel;

    QVariantList m_expl;
    QVariantList m_capWord;
    QString      m_query;
    QStringList  m_srcLans;
    QStringList  m_detLans;
    QString      m_srcLan;
    QString      m_detLan;
    QVariantList m_vbook;
    QString      m_vBookExpl;
    QString      m_statusText;
    QString      m_helpText;

    DictItemList *m_capItems;

    void refreshVBook();
};

#endif // _QMLBRIDGE_H_
