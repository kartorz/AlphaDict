#ifndef _QTMESSAGER_H_
#define _QTMESSAGER_H_

#include "MessageQueue.h"
#include "DictIndexModel.h"

#include <QThread>

class MainWindow;

class QtMessager: public QObject
{
	Q_OBJECT

public slots:
	void doWork();
	void onExit();

public:
    QtMessager(MainWindow* owner, DictIndexModel* model);
    ~QtMessager();

	void start();
	void abort();

protected:
	MainWindow* m_owner;
	QThread *m_thread;
	int m_reqAbort;

private:
    DictIndexModel* m_indexListModel;
};

#endif
