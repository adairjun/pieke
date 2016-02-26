#ifndef _MASTER_THREAD_H_
#define _MASTER_THREAD_H_

#include "worker_thread.h"
#include "pieke/socket_obj.h"

class WorkerThread;

/*
 * 这是一个master-worker模型，master启动一个eventLoop,在监听套接字上执行accept，并把得到的文件描述符传给worker
 * worker启动多个线程来处理文件描述符
 */
class MasterThread : public boost::enable_shared_from_this<MasterThread> {
 public:
  explicit MasterThread();
  MasterThread(const MasterThread&) = delete;
  MasterThread& operator=(const MasterThread&) = delete;
  virtual ~MasterThread();
  void Dump() const;

 public:
  bool InitMasterThread();
  void Run();

 private:
  static void AccepCb(evutil_socket_t listen_socket, short event, void* arg);

 private:
  struct event_base* base_;
  evutil_socket_t listenFd_;
  struct event* event_;
  WorkerThreadPtr pWorkerThread_;
};

typedef boost::shared_ptr<MasterThread>MasterThreadPtr;

typedef struct mybase {
  SocketObjPtr socketPtr;
  MasterThreadPtr masterThreadPtr;
}mybaseStruct;


#endif
