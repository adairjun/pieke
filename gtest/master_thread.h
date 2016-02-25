#ifndef _MASTER_THREAD_H_
#define _MASTER_THREAD_H_

#include "worker_thread.h"
#include "pieke/socket_obj.h"

class WorkerThread;

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
