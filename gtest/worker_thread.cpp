#include "worker_thread.h"
#include <unistd.h>
#include <cstring>
#include <thread>
#include "pieke/logobj.h"

using std::thread;

WorkerThread::WorkerThread() {
  last_thread_ = -1;
}

WorkerThread::~WorkerThread() {
  for(auto iter : vec_libevent_thread_) {
    delete iter;
  }
}

void WorkerThread::Dump() const {
  printf("\n=====WorkerThread Dump START ========== \n");
  printf("last_thread_=%d ", last_thread_);
  for (int i = 0; i < vec_libevent_thread_.size(); ++i) {
	printf("vec_libevent_thread_[%d]=%p ", i, vec_libevent_thread_[i]);
  }
  printf("\n===WorkerThread DUMP END ============\n");
}

bool WorkerThread::InitThreads() {
  LOG(INFO) << "Initializes worker threads...\n";

  for(unsigned int i=0; i< THREAD_NUM; ++i) {
	LibeventThread* libevent_thread_ptr = new LibeventThread;
	/* 建立每个worker线程和主监听线程通信的管道 */
	int fds[2];
	if (pipe(fds) != 0) {
	  LOG(ERROR) << "Thread::InitThreads:Can't create notify pipe\n";
	  return false;
	}

	// fds[0]用于读取管道,fds[1]用于写入管道
	libevent_thread_ptr->notify_receive_fd = fds[0];
    libevent_thread_ptr->notify_send_fd	   = fds[1];

    // 继续设定LIBEVENT_THREAD
    libevent_thread_ptr->base = event_base_new();
    /* 通过每个worker线程的读管道监听来自master的通知 */
    libevent_thread_ptr->notify_event = event_new(libevent_thread_ptr->base, libevent_thread_ptr->notify_receive_fd, EV_READ|EV_PERSIST, ReadPipeCb, (void*)libevent_thread_ptr);

    if (event_add(libevent_thread_ptr->notify_event, NULL) == -1) {
      int error_code = EVUTIL_SOCKET_ERROR();
      LOG(ERROR) << "WorkerThread::SetupThread:event_add errorCode = " << error_code
				<< ", description = " << evutil_socket_error_to_string(error_code);
      return false;
    }

    //把worker放进池里面
    vec_libevent_thread_.push_back(libevent_thread_ptr);

    // 将线程启动起来
    thread t(WorkerLibevent, libevent_thread_ptr);

    // 创建一个线程之后必须使用join或者detach来释放线程资源，这里不能使用join阻塞住，只能使用detach
    // 我以前写多线程代码使用join的目的是防止主线程提前结束，
    // 但是现在因为主线程是一个eventloop，或者直接说这个进程就是一个守护进程，是不会结束的
    t.detach();
  }

  // 这里不需要使用join来防止
  LOG(INFO) << "Create threads success. we hava done all the libevent setup.\n";

  return true;
}

void WorkerThread::ReadPipeCb(int notify_receive_fd, short event, void* arg) {
  LOG(INFO) << "WorkerThread::ReadPipeCb...\n";

  LibeventThread* libevent_thread_ptr = static_cast<LibeventThread*>(arg);

  /* read from master-thread had write, a byte 代表一个客户端连接 */
  char buf[1];
  if (read(notify_receive_fd, buf, 1) != 1) {
  	LOG(ERROR) << "WorkerThread::ThreadLibeventProcess:Can't read from libevent pipe.\n";
  	return;
  }

  // 由于libevent_thread_ptr是每个线程对应唯一的一个，那么它当下的list_conn也是只有这个线程来操作，其他的线程操作不了，所以不需要加锁
  int fd = libevent_thread_ptr->list_conn.front();
  libevent_thread_ptr->list_conn.pop_front();

  // 就是把sfd和LIBEVENT_THREAD的指针放入CONN当中,构建一个CONN
  if(fd != 0) {
	CONN* conn = new CONN;
	if (conn == NULL) {
	  LOG(ERROR) << "WorkerThread::InitNewConn:new conn error.\n";
	  return;
	}
	/*conn->buf = new char[DATA_BUFFER_SIZE];*/
	conn->sfd = fd;
	conn->thread = libevent_thread_ptr;

	/* 将新连接加入此线程libevent事件循环 */
	struct bufferevent *client_tcp_event = bufferevent_socket_new(libevent_thread_ptr->base, fd, BEV_OPT_CLOSE_ON_FREE);

	if (client_tcp_event == NULL) {
	   // 销毁conn
	   FreeConn(conn);
	   int error_code = EVUTIL_SOCKET_ERROR();
       LOG(ERROR) << "WorkerThread::conn_new:bufferevent_socket_new errorCode = " << error_code << ", description = " << evutil_socket_error_to_string(error_code);
       return;
	}
	bufferevent_setcb(client_tcp_event, ClientTcpReadCb, NULL, ClientTcpErrorCb, (void*)conn);

	  /* 利用客户端心跳超时机制处理半开连接 */
	struct timeval heartbeat_sec;
	heartbeat_sec.tv_sec = CLIENT_HEARTBEAT_TIEMOUT;
	heartbeat_sec.tv_usec= 0;
	bufferevent_set_timeouts(client_tcp_event, &heartbeat_sec, NULL);
	bufferevent_enable(client_tcp_event, EV_READ | EV_WRITE | EV_PERSIST);

	if (conn == NULL) {
	  LOG(ERROR) << "WorkerThread::ReadPipeCb:Can't listen for events on sfd = " << fd << "\n";
	  close(fd);
	  return;
	}
  }
}

void WorkerThread::WorkerLibevent(void *arg) {
  LOG(INFO) << "WorkerThread::WorkerLibevent...\n";

  LibeventThread* me = static_cast<LibeventThread*>(arg);
  event_base_dispatch(me->base);
}

void WorkerThread::ClientTcpReadCb(struct bufferevent *bev, void *arg) {
  LOG(INFO) << "WorkerThread::ClientTcpReadCb...\n";

  CONN* conn = static_cast<CONN*>(arg);
  int n;
  evutil_socket_t fd = bufferevent_getfd(bev);
  // 这里的buf就是连接conn的buf
  while (n=bufferevent_read(bev, conn->buf, DATA_BUFFER_SIZE), n > 0) {
    //回显给客户端
	bufferevent_write(bev, conn->buf, n);
	//将buf写回到客户端的时候一定要清空buf,否则的话下次read的时候buf里面会存在客户端上次发送的残留
	memset(conn->buf, 0, sizeof(conn->buf));
  }
}

void WorkerThread::ClientTcpErrorCb(struct bufferevent *bev, short event, void *arg) {
  LOG(ERROR) << "WorkerThread::ClientTcpErrorCb...\n";

  if (event & BEV_EVENT_TIMEOUT) {
    LOG(ERROR) << "WorkerThread::ClientTcpErrorCb:TimeOut.\n";
  } else if (event & BEV_EVENT_EOF) {
    LOG(ERROR) << "WorkerThread::ClientTcpErrorCb:BEV_EVENT_EOF.\n";
  } else if (event & BEV_EVENT_ERROR) {
	int error_code = EVUTIL_SOCKET_ERROR();
	LOG(ERROR) << "WorkerThread::ClientTcpErrorCb:some other errorCode = " << error_code
			<< ", description = " << evutil_socket_error_to_string(error_code);
  }

  // 出错之后要清理掉conn
  CONN* conn = static_cast<CONN*>(arg);
  CloseConn(conn, bev);
}

void WorkerThread::DispatchSfdToWorker(int sfd) {
  LOG(INFO) << "WorkerThread::DispatchSfdToWorker...\n";

  //round-robin, 目的是为了让线程的工作量平均分配
  int tid = (last_thread_ + 1) % THREAD_NUM;
  LibeventThread* libevent_thread_ptr = vec_libevent_thread_.at(tid);
  last_thread_ = tid;

  // 把accept得到的sfd放入list当中去，让ReadPipeCb能够获取到sfd
  libevent_thread_ptr->list_conn.push_back(sfd);

  /* 通知此worker线程有新连接到来，可以读取了 */
  char buf[1];
  buf[0] = 'c';
  if (write(libevent_thread_ptr->notify_send_fd, buf, 1) != 1) {
  	LOG(INFO) << "WorkerThread::DispatchSfdToWorker:Writing to thread notify pipe\n";
  }
}

void WorkerThread::FreeConn(CONN* conn) {
  LOG(INFO) << "WorkerThread::FreeConn...\n";

  if (conn != NULL) {
    delete conn;
  }
}

void WorkerThread::CloseConn(CONN* conn, struct bufferevent* bev) {
  LOG(INFO) << "WorkerThread::CloseConn...\n";

  if (conn != NULL) {
    /* 清理资源：the event, the socket and the conn */
    bufferevent_free(bev);
    FreeConn(conn);
  }
}
