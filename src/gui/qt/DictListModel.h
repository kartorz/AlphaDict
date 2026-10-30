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

#ifndef _DICTLISTMODEL_H_
#define _DICTLISTMODEL_H_

#include <QtCore/QAbstractListModel>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

class Configure;

/*! The dictionary list of Configure::m_dictNodes exposed to QML.
 *
 *  Toggling a dictionary has to go through the message queue (SysMessager),
 *  because that is what schedules the dictionary reload.  The queue is
 *  asynchronous, so the value shown by the model is kept optimistically in
 *  m_pending until Configure reports the new state; otherwise the check box
 *  would flip back on the next data() call.
 */
class DictListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        EnabledRole,
        SummaryRole,
        SrcLanRole,
        DetLanRole,
        RowRole
    };

    explicit DictListModel(QObject *parent = NULL);
    virtual ~DictListModel();

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    Q_INVOKABLE void setEnabled(int row, bool en);
    Q_INVOKABLE void moveRow(int row, int offset);
    Q_INVOKABLE void refresh();

private slots:
    void confirmPending();

private:
    int dictCount() const;

    Configure *m_config;
    QHash<int, bool> m_pending;
    QHash<int, qint64> m_bGiveUpTime;
    QTimer m_timer;
};

#endif // _DICTLISTMODEL_H_
