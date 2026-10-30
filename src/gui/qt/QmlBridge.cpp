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

#include <QtCore/QVariantMap>
#include <QtCore/QAbstractItemModel>

#include "QmlBridge.h"
#include "Configure.h"
#include "DictIndexModel.h"
#include "DictListModel.h"
#include "VBookModel.h"
#include "DictManager.h"
#include "Application.h"
#include "MessageQueue.h"
#include "Util.h"

namespace {

QVariantMap dictItemToMap(const iDictItem &item)
{
    QVariantMap map;
    map.insert("word",     QString::fromStdString(item.word));
    map.insert("dictname", QString::fromStdString(item.dictname));
    map.insert("phonetic", QString::fromStdString(item.phonetic));
    map.insert("expl",     QString::fromStdString(item.expl));
    map.insert("found",    item.bfind);
    return map;
}

} // namespace

QmlBridge::QmlBridge(QObject *parent)
    : QObject(parent),
      m_config(NULL),
      m_indexModel(NULL),
      m_dictModel(NULL),
      m_vbookModel(NULL),
      m_capItems(NULL)
{
    m_config = g_application.m_configure;

    m_indexModel = new DictIndexModel();
    m_indexModel->setParent(this);
    m_dictModel = new DictListModel(this);
    m_vbookModel = new VBookModel(m_config->getVBPath());
    m_vbookModel->setParent(this);

    m_srcLan = QString::fromStdString(m_config->m_srcLan);
    m_detLan = QString::fromStdString(m_config->m_detLan);

    /* Counts and the vocabulary list are derived from the models, so keep them
     * in sync with every reset the models issue. */
    connect(m_indexModel, SIGNAL(modelReset()), this, SIGNAL(indexCountChanged()));
    connect(m_dictModel, SIGNAL(modelReset()), this, SIGNAL(dictCountChanged()));
    /* hasDict() reads DictManager, not the model, so the property needs its own
     * notification.  Every dictionary load or reorder resets the list model, so
     * that is the one place where the state can have changed. */
    connect(m_dictModel, SIGNAL(modelReset()), this, SIGNAL(dictStateChanged()));
    connect(m_vbookModel, SIGNAL(modelReset()), this, SLOT(onVBookModelReset()));

    refreshHelpText();
    refreshVBook();
    selectVBookRow(0);
}

QmlBridge::~QmlBridge()
{
    delete m_capItems;
}

/*---------------------------------------------------------------------------
 * messages from the backend
 *-------------------------------------------------------------------------*/

void QmlBridge::onUpdateExplText(void *v)
{
    DictItemList *itemList = (DictItemList *)v;
    if (itemList == NULL)
        return;

    QVariantList list;
    for (size_t i = 0; i < itemList->size(); i++)
        list.append(dictItemToMap((*itemList)[i]));

    m_expl = list;
    emit explChanged();

    delete itemList;
}

void QmlBridge::onUpdateCapWordExplText(void *v)
{
    DictItemList *itemList = (DictItemList *)v;
    if (itemList == NULL)
        return;

    delete m_capItems;
    m_capItems = itemList;

    QVariantList list;
    for (size_t i = 0; i < itemList->size(); i++)
        list.append(dictItemToMap((*itemList)[i]));

    m_capWord = list;
    emit capWordChanged();
    emit capWordShown();
}

void QmlBridge::onSetLanComboBox(const QString &src, const QString &det, void *v)
{
    /* The sender hands over a heap copy and this slot owns it (pArg1[@3]). */
    vector<string> *pVec = (vector<string> *)v;

    m_srcLans.clear();
    m_detLans.clear();
    m_srcLans << "any";
    m_detLans << "any";

    if (pVec != NULL) {
        for (size_t i = 0; i < pVec->size(); i++) {
            QString item(QString::fromStdString((*pVec)[i]));
            m_srcLans << item;
            m_detLans << item;
        }
        delete pVec;
    }

    m_srcLan = src;
    m_detLan = det;
    emit languagesChanged();
}

/*---------------------------------------------------------------------------
 * lookups
 *-------------------------------------------------------------------------*/

void QmlBridge::search(const QString &input)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty())
        return;

    std::string u8input(trimmed.toUtf8().constData());
    if (!Util::isValidInput(u8input)) {
        setStatus(tr("Invalid input"));
        return;
    }

    m_query = trimmed;
    m_expl.clear();
    emit queryChanged();
    emit explChanged();

    g_application.sysMessageQ()->push(MSG_DICT_QUERY, u8input);
}

void QmlBridge::prefixSearch(const QString &prefix)
{
    std::string u8prefix(prefix.trimmed().toUtf8().constData());
    m_indexModel->onResetIndexList(u8prefix);
}

void QmlBridge::clickIndex(int row)
{
    if (row < 0 || row >= m_indexModel->rowCount())
        return;

    /* Hand a detached copy to the consumer thread: the model may reset (and
     * free its items) before the queued message is handled. */
    iIndexItem *item = m_indexModel->itemClone(row);
    if (item == NULL)
        return;

    m_query = QString::fromStdString(item->index);
    m_expl.clear();
    emit queryChanged();
    emit explChanged();

    g_application.sysMessageQ()->push(MSG_DICT_INDEX_QUERY, row, (void *)item);
}

int QmlBridge::pageIndex(int pages)
{
    if (pages == 0)
        return -1;
    QModelIndex inx = m_indexModel->updateIndexList(pages);
    if (!inx.isValid())
        return -1;
    return inx.row();
}

int QmlBridge::indexRowCount() const
{
    return m_indexModel->rowCount();
}

int QmlBridge::indexCount() const
{
    return m_indexModel->rowCount();
}

int QmlBridge::dictCount() const
{
    return m_dictModel->rowCount();
}

/*---------------------------------------------------------------------------
 * vocabulary book
 *-------------------------------------------------------------------------*/

void QmlBridge::refreshVBook()
{
    m_vbook.clear();
    int rows = m_vbookModel->rowCount();
    for (int i = 0; i < rows; i++) {
        QVariantMap map;
        /* word(), not data(): in exam mode data() hides the word until the
         * row has been answered. */
        map.insert("word", m_vbookModel->word(i));
        map.insert("complexity", m_vbookModel->data(m_vbookModel->index(i, 1), Qt::DisplayRole));
        map.insert("date", m_vbookModel->data(m_vbookModel->index(i, 2), Qt::DisplayRole));
        m_vbook.append(map);
    }

    int cur = m_vbookModel->getCurrentRow();
    if (cur >= 0 && cur < rows && m_vbookModel->getMode() == StudyMode)
        m_vBookExpl = m_vbookModel->expl(cur);
}

void QmlBridge::onVBookModelReset()
{
    refreshVBook();
    if (m_vbookModel->rowCount() == 0)
        m_vBookExpl.clear();
    emit vbookChanged();
}

QString QmlBridge::vBookWord() const
{
    int row = m_vbookModel->getCurrentRow();
    if (row < 0 || row >= m_vbookModel->rowCount())
        return QString();
    return m_vbookModel->word(row);
}

void QmlBridge::selectVBookRow(int row)
{
    if (row < 0 || row >= m_vbookModel->rowCount()) {
        m_vBookExpl.clear();
        emit vbookChanged();
        return;
    }

    m_vbookModel->setCurrentRow(row);
    m_vBookExpl = m_vbookModel->expl(row);
    emit vbookChanged();
}

void QmlBridge::addToVBook(const QString &word)
{
    QString trimmed = word.trimmed();
    if (trimmed.isEmpty())
        return;

    if (m_vbookModel->add(trimmed))
        setStatus(tr("Added to the vocabulary book"));
    else
        setStatus(tr("Already in the vocabulary book"));

    refreshVBook();
    emit vbookChanged();
}

bool QmlBridge::addToVBookChecked(const QString &word)
{
    QString trimmed = word.trimmed();
    if (trimmed.isEmpty())
        return false;

    bool added = m_vbookModel->add(trimmed);
    setStatus(added ? tr("Added to the vocabulary book")
                    : tr("Already in the vocabulary book"));

    refreshVBook();
    emit vbookChanged();
    return added;
}

void QmlBridge::removeVBookRow(int row)
{
    if (row < 0 || row >= m_vbookModel->rowCount())
        return;

    m_vbookModel->remove(row);
    refreshVBook();
    if (row >= m_vbookModel->rowCount())
        selectVBookRow(m_vbookModel->rowCount() - 1);
    else
        selectVBookRow(row);
}

void QmlBridge::clearVBook()
{
    m_vbookModel->clear();
    m_vBookExpl.clear();
    refreshVBook();
    emit vbookChanged();
}

void QmlBridge::vbookStep(int step)
{
    if (m_vbookModel->rowCount() == 0)
        return;

    QModelIndex inx = step > 0 ? m_vbookModel->moveDown() : m_vbookModel->moveUp();
    selectVBookRow(inx.row());
}

void QmlBridge::vbookInput(const QString &input, int repeat)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty() || m_vbookModel->rowCount() == 0)
        return;

    if (m_vbookModel->getMode() == ExamMode) {
        QString score;
        bool ok = m_vbookModel->exam(trimmed, score);
        setStatus(score);
        if (ok) {
            /* the answered row now shows its word instead of the placeholder */
            refreshVBook();
            emit vbookChanged();
        }
        if (m_vbookModel->isLastRow())
            selectVBookRow(m_vbookModel->moveToFirst().row());
        else
            vbookStep(1);
    } else {
        int ret = m_vbookModel->study(trimmed, repeat);
        if (ret == 0) {
            setStatus(tr("Correct"));
            vbookStep(1);
        } else if (ret < 0) {
            setStatus(tr("Try again"));
        } else {
            setStatus(tr("Repeat %1 more time(s)").arg(ret));
        }
    }
}

void QmlBridge::setVBookMode(int mode)
{
    if ((int)m_vbookModel->getMode() == mode)
        return;
    m_vbookModel->setMode((enum VBookMode)mode);
    /* setMode() resets the model, which refreshes the list and emits
     * vbookChanged() through onVBookModelReset(). */
    emit vBookModeChanged();
}

int QmlBridge::vBookMode() const
{
    return (int)m_vbookModel->getMode();
}

int QmlBridge::vBookRow() const
{
    return m_vbookModel->getCurrentRow();
}

/*---------------------------------------------------------------------------
 * properties
 *-------------------------------------------------------------------------*/

int QmlBridge::fontSize() const
{
    return m_config->m_setting.fontsize;
}

QString QmlBridge::fontFamily() const
{
    return QString::fromStdString(m_config->m_setting.font);
}

QString QmlBridge::dataDir() const
{
    return QString::fromStdString(m_config->m_dataDir);
}

bool QmlBridge::hasDict() const
{
    return DictManager::getReference().hasDict();
}

bool QmlBridge::systemTray() const
{
    return m_config->m_setting.bsystemTray;
}

bool QmlBridge::cwsEnabled() const
{
    return m_config->m_cws.benable;
}

bool QmlBridge::cwsClipboard() const
{
    return m_config->m_cws.bclipboard;
}

bool QmlBridge::cwsSelection() const
{
    return m_config->m_cws.bselection;
}

bool QmlBridge::autoCloseEn() const
{
    return m_config->m_cws.autoCloseEn;
}

int QmlBridge::shortcutIndex() const
{
    int key = m_config->m_cws.shortcutKey;
    if (key < 0 || key > 25)
        key = 0;
    return key;
}

QStringList QmlBridge::shortcutKeys() const
{
    QStringList keys;
    for (char c = 'A'; c <= 'Z'; c++)
        keys << QString(QChar(c));
    return keys;
}

int QmlBridge::uiLanID() const
{
    return m_config->m_setting.uilanID;
}

QStringList QmlBridge::uiLans() const
{
    QStringList lans;
    lans << QString::fromUtf8("English") << QString::fromUtf8("简体中文");
    return lans;
}

void QmlBridge::setSrcLan(const QString &lan)
{
    if (m_srcLan == lan)
        return;
    m_srcLan = lan;
    std::string u8lan(lan.toUtf8().constData());
    g_application.sysMessageQ()->push(MSG_SET_SRCLAN, u8lan);
    emit languagesChanged();
}

void QmlBridge::setDetLan(const QString &lan)
{
    if (m_detLan == lan)
        return;
    m_detLan = lan;
    std::string u8lan(lan.toUtf8().constData());
    g_application.sysMessageQ()->push(MSG_SET_DETLAN, u8lan);
    emit languagesChanged();
}

void QmlBridge::setFontSize(int size)
{
    if (size < 9)
        size = 9;
    if (size > 32)
        size = 32;
    if (m_config->m_setting.fontsize == size)
        return;
    m_config->writeFontSize(size);
    emit fontSizeChanged();
}

void QmlBridge::setFontFamily(const QString &family)
{
    if (fontFamily() == family)
        return;
    m_config->writeFont(family.toStdString());
    emit fontFamilyChanged();
}

void QmlBridge::setSystemTray(bool en)
{
    if (systemTray() == en)
        return;
    m_config->writeSystemTray(en);
    emit systemTrayChanged();
    emit requestSystemTray(en);
}

void QmlBridge::setCwsEnabled(bool en)
{
    if (cwsEnabled() == en)
        return;
    m_config->writeCwsEnable(en);
    emit cwsChanged();
    emit requestCwsEnable(en);
}

void QmlBridge::setCwsClipboard(bool en)
{
    if (cwsClipboard() == en)
        return;
    m_config->writeCwsClipboard(en);
    emit cwsChanged();
}

void QmlBridge::setCwsSelection(bool en)
{
    if (cwsSelection() == en)
        return;
    m_config->writeCwsSelection(en);
    emit cwsChanged();
}

void QmlBridge::setAutoCloseEn(bool en)
{
    if (autoCloseEn() == en)
        return;
    m_config->writeCwsAutoCloseEn(en);
    emit cwsChanged();
}

void QmlBridge::setShortcutIndex(int index)
{
    if (index < 0 || index > 25 || index == shortcutIndex())
        return;
    int old = shortcutIndex();
    m_config->writeCwsShortcutKey(index);
    emit cwsChanged();
    emit requestShortcutKey(old, index);
}

void QmlBridge::setUiLanID(int id)
{
    if (uiLanID() == id)
        return;
    m_config->writeUILanID(id);
    refreshHelpText();
    emit uiLanChanged();
    setStatus(tr("UI language will be applied after restart"));
}

void QmlBridge::resetSettings()
{
    m_config->reset();
    refreshHelpText();
    emit fontSizeChanged();
    emit fontFamilyChanged();
    emit systemTrayChanged();
    emit cwsChanged();
    emit uiLanChanged();
    setStatus(tr("Settings have been reset"));
}

void QmlBridge::refreshHelpText()
{
    std::string suffix[] = {".en", ".cn"};
    int lanid = uiLanID();
    if (lanid < 0 || lanid > 1)
        lanid = 0;

    std::string helpPath = m_config->m_dataDir + "/help" + suffix[lanid];
    FILE *pHelpFile = fopen(helpPath.c_str(), "rb");
    if (pHelpFile == NULL) {
        m_helpText = tr("Help file not found: %1").arg(QString::fromStdString(helpPath));
        emit helpTextChanged();
        return;
    }

    util::ReadFile read;
    char *bytes = (char *)read(pHelpFile, -1);
    m_helpText = bytes != NULL ? QString::fromUtf8(bytes) : QString();
    emit helpTextChanged();
}

void QmlBridge::setStatus(const QString &text)
{
    m_statusText = text;
    emit statusTextChanged();
}

/*---------------------------------------------------------------------------
 * window requests and dictionary list editing
 *-------------------------------------------------------------------------*/

void QmlBridge::moveDictRow(int row, int offset)
{
    m_dictModel->moveRow(row, offset);
}

void QmlBridge::setDictEnabled(int row, bool en)
{
    m_dictModel->setEnabled(row, en);
}

void QmlBridge::moveWindowBy(int dx, int dy)
{
    emit requestMove(dx, dy);
}

void QmlBridge::minimizeToTray()
{
    emit requestMinimizeToTray();
}

void QmlBridge::quit()
{
    emit requestQuit();
}
