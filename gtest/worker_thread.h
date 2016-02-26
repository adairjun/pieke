#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <vector>
#include <list>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using std::vector;
using std::list;

#define DATA_BUFFER_SIZE 2048

// 一个worker线程上就对应了一个LibeventThread
typedef struct {
    int 			    thread_id;        	 //线程id
    struct event_base* 	base;    			 //当前线程上的event_base
    struct event* 		notify_event;  		 //当前线程上的event，这里也只需要一个event，就是绑定在管道上的event
    int 				notify_receive_fd;   // worker线程接收master线程的指令
    int 				notify_send_fd;      // master线程发送指令给worker线程
    list<int> 	        list_conn;	         // master线程分配给worker线程的描述符
} LibeventThread;

typedef boost::shared_ptr<LibeventThread>LibeventThreadPtr;

// 这个是表示libevent正在执行的连接
// 一个LibeventThread对应多个CONN
// 写这个的目的就是为了每一个连接设定一个buffer，因为在多线程环境下如果使用全局变量char buf[DATA_BUFFER_SIZE]会有竞争问题
typedef struct {
    int  sfd;
    char buf[DATA_BUFFER_SIZE];
    LibeventThread* thread;
}CONN;

typedef boost::shared_ptr<CONN>CONNPtr;

class WorkerThread : public boost::enable_shared_from_this<WorkerThread> {
 public:
  explicit WorkerThread();
  WorkerThread(const WorkerThread&) = delete;
  WorkerThread& operator=(const WorkerThread&) = delete;
  virtual ~WorkerThread();
  void Dump() const;

 public:
  bool InitThreads();
  void DispatchSfdToWorker(int sfd);

 private:
  // 监听notify_receive_fd这个管道
  static void ReadPipeCb(int notify_receive_fd, short event, void* arg);
  static void WorkerLibevent(void *arg);

  static void ClientTcpReadCb(struct bufferevent *bev, void *arg);
  static void ClientTcpErrorCb(struct bufferevent *bev, short event, void *arg);
  static void FreeConn(CONN *conn);
  static void CloseConn(CONN *conn, struct bufferevent *bev);

 private:
  //enum hack
  enum { THREAD_NUM = 4 };
  enum { CLIENT_HEARTBEAT_TIEMOUT = 10 };

 private:
  vector<LibeventThread*> vec_libevent_thread_;
  int last_thread_;
};

typedef boost::shared_ptr<WorkerThread>WorkerThreadPtr;

#endif
