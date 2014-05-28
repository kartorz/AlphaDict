#include "MessageQueue.h"

MessageQueue::MessageQueue(std::string identi)
:m_identify(identi)
{
}

MessageQueue::~MessageQueue()
{
    flush();
}

void MessageQueue::push(Message& msg)
{
    produce(&msg);
}

void MessageQueue::push(int id)
{
	Message msg;
    msg.id = id;
    produce(&msg);
}

void MessageQueue::push(int id, int arg1, int arg2)
{
	Message msg;
    msg.id = id;
    msg.iArg1 = arg1;
    msg.iArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, float arg1, float arg2)
{
	Message msg;
    msg.id = id;
    msg.fArg1 = arg1;
    msg.fArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, double arg1, double arg2)
{
	Message msg;
    msg.id = id;
    msg.dArg1 = arg1;
    msg.dArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, std::string arg1, std::string arg2)
{
	Message msg;
    msg.id = id;
    msg.strArg1 = arg1;
    msg.strArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, std::string& arg1, std::string& arg2)
{
	Message msg;
    msg.id = id;
    msg.strArg1 = arg1;
    msg.strArg2 = arg2;
    produce(&msg);
}

void MessageQueue::push(int id, int arg1, void *arg2, void *arg3)
{
    Message msg;
    msg.id = id;
    msg.iArg1 = arg1;
    msg.pArg1 = arg2;
    msg.pArg2 = arg3;
    produce(&msg);
}

void MessageQueue::onProduce(void *v)
{
    SpinLock m_lock(m_crs);
    m_queue.push_back(*((Message *)v));
}

bool MessageQueue::pop(Message& msg, bool bwait)
{
    bool ret = false;
    if (bwait) {
        ret = (consume(&msg) == 0);    
    } else {
        SpinLock m_lock(m_crs);
    	if (!m_queue.empty()) {
            msg = m_queue.back();
            m_queue.pop_back();
            ret = true;
        }
    }
    //printf("pop a message(%s, %d)\n", m_identify.c_str(), ret);
    return ret;
}

void MessageQueue::onConsume(void* v)
{
    SpinLock m_lock(m_crs);
    //printf("onConsume(%s)\n", m_identify.c_str());
    *((Message *)v)  = m_queue.back();
    m_queue.pop_back();
}

void MessageQueue::flush()
{
   SpinLock m_lock(m_crs);
   m_queue.clear();
}

void MessageQueue::flush(int id)
{
   SpinLock m_lock(m_crs);
   std::list<Message>::iterator it;
   for (it = m_queue.begin(); it != m_queue.end(); it++) {       
       if (it->id == id) {
           m_queue.erase(it);
       }
   }
}
