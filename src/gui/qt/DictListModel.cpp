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

#include "DictListModel.h"

#include "Configure.h"
#include "Application.h"
#include "MessageQueue.h"
#include "alphadict.h"

namespace {
    const int PENDING_POLL_MS = 100;
    const int PENDING_GIVEUP_MS = 3000;
}

DictListModel::DictListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_config(NULL)
{
    m_config = g_application.m_configure;
    m_timer.setInterval(PENDING_POLL_MS);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(confirmPending()));
}

DictListModel::~DictListModel()
{
}

int DictListModel::dictCount() const
{
    if (m_config == NULL)
        return 0;
    return (int)m_config->m_dictNodes.size();
}

int DictListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return dictCount();
}

QHash<int, QByteArray> DictListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole]    = "name";
    roles[EnabledRole] = "enabled";
    roles[SummaryRole] = "summary";
    roles[SrcLanRole]  = "srcLan";
    roles[DetLanRole]  = "detLan";
    roles[RowRole]     = "rowIndex";
    return roles;
}

QVariant DictListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= dictCount())
        return QVariant();

    const int row = index.row();
    const DictNode &node = m_config->m_dictNodes[row];
    bool en = node.en;
    if (m_pending.contains(row))
        en = m_pending.value(row);

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return QString::fromStdString(node.name);
    case Qt::CheckStateRole:
        return en ? Qt::Checked : Qt::Unchecked;
    case EnabledRole:
        return en;
    case SummaryRole:
        return QString::fromStdString(node.summary);
    case SrcLanRole:
        return QString::fromStdString(node.srclan);
    case DetLanRole:
        return QString::fromStdString(node.detlan);
    case RowRole:
        return row;
    default:
        return QVariant();
    }
}

Qt::ItemFlags DictListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

bool DictListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::CheckStateRole) {
        setEnabled(index.row(), value.toInt() == Qt::Checked);
        return true;
    }
    if (role == EnabledRole) {
        setEnabled(index.row(), value.toBool());
        return true;
    }
    return false;
}

void DictListModel::setEnabled(int row, bool en)
{
    if (row < 0 || row >= dictCount())
        return;

    if (m_pending.contains(row) ? m_pending.value(row) == en
                                : m_config->m_dictNodes[row].en == en)
        return;

    m_pending.insert(row, en);
    m_bGiveUpTime.insert(row, QDateTime::currentMSecsSinceEpoch() + PENDING_GIVEUP_MS);
    if (!m_timer.isActive())
        m_timer.start();

    emit dataChanged(index(row), index(row), QVector<int>() << EnabledRole
                                                           << Qt::CheckStateRole);

    // SysMessager turns this into configure->enableDict() plus a dictionary
    // reload, so the switch must not be applied locally.
    g_application.sysMessageQ()->push(MSG_SET_DICTEN, row, en ? 1 : 0);
}

void DictListModel::moveRow(int row, int offset)
{
    if (offset == 0 || row < 0 || row >= dictCount())
        return;
    int target = row + offset;
    if (target < 0 || target >= dictCount())
        return;

    // moveDictItem() swaps neighbouring entries only.
    int step = offset > 0 ? 1 : -1;
    while (row != target) {
        g_application.sysMessageQ()->push(MSG_MOVE_DICTITEM, row, step > 0 ? 1 : 0);
        row += step;
    }

    QTimer::singleShot(PENDING_POLL_MS * 2, this, SLOT(refresh()));
}

void DictListModel::refresh()
{
    beginResetModel();
    endResetModel();
}

void DictListModel::confirmPending()
{
    if (m_pending.isEmpty()) {
        m_timer.stop();
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QList<int> rows = m_pending.keys();
    for (int i = 0; i < rows.size(); i++) {
        int row = rows.at(i);
        if (row < 0 || row >= dictCount()) {
            m_pending.remove(row);
            m_bGiveUpTime.remove(row);
            continue;
        }
        bool wanted = m_pending.value(row);
        bool actual = m_config->m_dictNodes[row].en;
        bool giveUp = now > m_bGiveUpTime.value(row, now);
        if (actual == wanted || giveUp) {
            m_pending.remove(row);
            m_bGiveUpTime.remove(row);
            emit dataChanged(index(row), index(row), QVector<int>() << EnabledRole
                                                                   << Qt::CheckStateRole);
        }
    }

    if (m_pending.isEmpty())
        m_timer.stop();
}
