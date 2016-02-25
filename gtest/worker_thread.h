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

// 由于是IO多路复用,就是说一个线程要监听多个socket套接字,所以这里用队列来保存多个sfd
typedef struct {
    int 			    thread_id;        	 //unique ID of this thread
    struct event_base* 	base;    			 //libevent handle this thread uses
    struct event* 		notify_event;  		 //listen event for notify pipe
    int 				notify_receive_fd;   //receiving end of notify pipe
    int 				notify_send_fd;      //sending end of notify pipe
    list<int> 	        list_conn;	 //queue of new connections to handle
} LibeventThread;

typedef boost::shared_ptr<LibeventThread>LibeventThreadPtr;

// 这个是表示libevent正在执行的连接,这个CONN和LIBEVENT_THREAD是多对一的关系
// 写这个的目的就是为了每一个连接设定一个buffer，因为在多线程环境下如果使用全局变量char buf[DATA_BUFFER_SIZE]会有竞争问题
typedef struct {
    int    sfd;
    char  buf[DATA_BUFFER_SIZE];
    LibeventThread* thread;
}CONN;

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
