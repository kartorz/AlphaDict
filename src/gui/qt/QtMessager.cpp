#include "MessageQueue.h"
#include "QtMessager.h"
#include "Application.h"
#include "Log.h"

QtMessager::QtMessager(QObject* owner,  DictIndexModel* inxModel, MessageQueue* q)
:m_thread(NULL), m_reqAbort(false)
{
    m_owner = owner;
    m_indexListModel = inxModel;
    m_msgQ = q;
}

QtMessager::~QtMessager()
{
    /* Unconditional, because 'm_reqAbort' only proves that doWork() left its
     * own loop: QThread::run() is the default exec(), so the thread is still
     * spinning until quit() and wait() have been called. abort() checks
     * isRunning() itself, so this is a no-op once the thread has stopped. */
    abort();
    delete m_thread;
    m_thread = NULL;
}

void QtMessager::start()
{
    /* Idempotent: a second start() would leak the first QThread and re-target
     * this object's thread affinity. */
    if (m_thread != NULL)
        return;

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
        if (m_msgQ->pop(msg)) {
            //printf("{QtMessager} MSGID:%d\n", msg.id);
            switch (msg.id) {
            case MSG_RESET_INDEXLIST:{
                QMetaObject::invokeMethod(m_indexListModel,
                                          "onResetIndexList",
                                          Qt::QueuedConnection);
                break;
            }

            case MSG_SET_DICTITEMS:{
                QMetaObject::invokeMethod(m_owner,
                                              "onUpdateExplText",
                                              Qt::QueuedConnection,
                                              Q_ARG(void*, msg.pArg1));
                break;
            }
                
            case MSG_SET_CAPWORD_DICTITEM: {
                QMetaObject::invokeMethod(m_owner,
                                              "onUpdateCapWordExplText",
                                              Qt::QueuedConnection,
                                              Q_ARG(void*, msg.pArg1));
                break;
            }

            case MSG_SET_LANLIST: {
                QMetaObject::invokeMethod(m_owner,
                                              "onSetLanComboBox",
                                              Qt::QueuedConnection,
                                              Q_ARG(QString, QString(msg.strArg1.c_str())),
                                              Q_ARG(QString, QString(msg.strArg2.c_str())),
                                              Q_ARG(void*, msg.pArg1));
                break;
            }

            case MSG_QUIT: {
                m_reqAbort = true;
                break;
            }
            default:
                break;
            }

        } else {
            m_reqAbort = true;
            //printf("{QtMessager} no message eixt\n");
            break;
        }
    }while(!m_reqAbort);
}

void QtMessager::abort()
{
	if(m_thread && m_thread->isRunning())
	{
	    m_reqAbort = true;
            m_msgQ->push(MSG_QUIT);
            //m_msgQ->unblockAll();
	    m_thread->quit();
	    m_thread->wait();
	}
}

void QtMessager::onExit()
{
}
