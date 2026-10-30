#ifndef _QTMESSAGER_H_
#define _QTMESSAGER_H_
#include <QtCore/QThread>

#include "MessageQueue.h"
#include "DictIndexModel.h"

class QtMessager: public QObject
{
	Q_OBJECT

public slots:
	void doWork();
	void onExit();

public:
    QtMessager(QObject* owner, DictIndexModel* model, MessageQueue* q);
    ~QtMessager();

	void start();
	void abort();

protected:
    QObject* m_owner;
    QThread *m_thread;
    int m_reqAbort;

private:
    DictIndexModel* m_indexListModel;
    MessageQueue* m_msgQ;
};

#endif
