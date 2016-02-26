#include "master_thread.h"
#include <string.h>
#include <thread>
#include <string>
#include "pieke/logobj.h"

const string HOST = "";
const unsigned PORT = 9999;
const int BACKLOG = 10;

MasterThread::MasterThread() : pWorkerThread_(new WorkerThread) {
  base_ = NULL;
  listenFd_ = 0;
  event_ = NULL;
}

MasterThread::~MasterThread() {
  event_base_free(base_);
}

void MasterThread::Dump() const {
  printf("\n=====MasterThread Dump START ========== \n");
  printf("base_=%p ", base_);
  printf("listenFd_=%d ", listenFd_);
  printf("event_=%p ", event_);
  printf("\n===MasterThread DUMP END ============\n");
}

bool MasterThread::InitMasterThread() {
  // 先检查一下libevent的版本，版本必须是2
  const char* libevent_version = event_get_version();
  LOG(INFO) << "LIBEVENT VERSION IS" << libevent_version << "\n";
  if (strncmp(libevent_version, "2", 1) != 0) {
	LOG(ERROR) << "LIBEVENT VERSION MUST BE 2.0.*\n";
	return false;
  }

  // 创建socket
  SocketObjPtr listener(new SocketObj(HOST, PORT, BACKLOG));
  if (listener->Listen() == false) {
	LOG(ERROR) << "listen error\n";
	return false;
  }

  listenFd_ = listener->Get();

  // 设置为非阻塞, 调用libevent的接口进行事件驱动
  evutil_make_socket_nonblocking(listenFd_);
  base_ = event_base_new();

  mybaseStruct* pointer = new mybaseStruct;
  pointer->socketPtr = listener;
  pointer->masterThreadPtr = shared_from_this();

  event_ = event_new(base_, listenFd_, EV_READ | EV_PERSIST, MasterThread::AccepCb, (void*)pointer);
  event_add(event_, NULL);

  if(!pWorkerThread_->InitThreads()) {
	LOG(ERROR) << "pWorkerThread->InitThreads() error.\n";
	return false;
  }
  return true;
}

void MasterThread::Run() {
  LOG(INFO) << "MasterThread::Run() start.........\n";

  int ret = event_base_dispatch(base_);
  if (ret == -1) {
    int error_code = EVUTIL_SOCKET_ERROR();
	LOG(ERROR) << "MasterThread::Run():event_base_dispatch error, description = "
			  << evutil_socket_error_to_string(error_code);
	return;
  } else if (ret == 1) {
	LOG(ERROR) << "MasterThread::Run():no events were registered.\n";
	return;
  }
}

void MasterThread::AccepCb(evutil_socket_t listen_socket, short event, void* arg) {
	LOG(INFO) << "MasterThread::Accept() start.........\n";
	mybaseStruct* pointer = static_cast<mybaseStruct*>(arg);
	SocketObjPtr listener = pointer->socketPtr;
	MasterThreadPtr masterThreadPtr = pointer->masterThreadPtr;
	evutil_socket_t fd = listener->Accept();
	if (fd == -1) {
	  LOG(ERROR) << "MasterThread::AccepCb:accept error!\n";
	  return;
	}

	//将客户端新连接分发到各个工作线程
	masterThreadPtr->pWorkerThread_->DispatchSfdToWorker(fd);
}
