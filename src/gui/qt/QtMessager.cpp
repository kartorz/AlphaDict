#include "MessageQueue.h"
#include "QtMessager.h"

QtMessager::QtMessager(MainWindow* owner,  DictIndexModel* inxModel)
:m_reqAbort(false)
{
	m_owner = owner;
    m_indexListModel = inxModel;
}

QtMessager::~QtMessager()
{
    if (!m_reqAbort)
        abort();
}

void QtMessager::start()
{
    m_thread = new QThread();
    connect(m_thread, SIGNAL(started()), this, SLOT(doWork()));
    connect(m_thread, SIGNAL(finished()), this,SLOT(onExit()));
    //connect(m_thread, SIGNAL(terminated()), this, SLOT()
    moveToThread(m_thread);
    m_thread->start();
}

void QtMessager::doWork()
{
	do{
        Message msg;
        if (g_uiMessagerQ.pop(msg)) {
            printf("{QtMessager} MSGID:%d\n", msg.id);
		    switch (msg.id) {
                case MSG_SET_INDEXLIST:{
                    QMetaObject::invokeMethod(m_indexListModel,
                                              "onUpdataList",
                                              Qt::QueuedConnection,
                                              Q_ARG(void*, msg.pArg1));
                }
                break;

			    case MSG_SET_DICTITEM:{
                    QMetaObject::invokeMethod((QObject *)m_owner,
                                              "onUpdateText",
                                              Qt::QueuedConnection,
                                              Q_ARG(void*, msg.pArg1));
                }
			    break;         
            }
			  //SysMessager::getInstance()->getMessageQ()->push(0);
        } else {
            break;
		}
	}while(!m_reqAbort);
}

void QtMessager::abort()
{
	if(m_thread && m_thread->isRunning())
	{
		m_reqAbort = true;
        g_uiMessagerQ.unblockAll();
		m_thread->quit();
		m_thread->wait();
	}
}

void QtMessager::onExit()
{
}
