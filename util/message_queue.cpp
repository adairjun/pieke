#include "pieke/message_queue.h"
#include "pieke/logobj.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

MessageQueue::MessageQueue()
    : msgFile_("key"){
	/*
	 * 这个key文件可不能由程序删除之后再创建，因为每次创建文件的时候，由linux系统赋予的索引节点很可能不一样，那样的话就会得到不用的key_t的值
	 */
	key_t key = ftok(msgFile_.c_str(),'a');
	msgid_ = msgget(key,S_IRUSR|S_IWUSR|IPC_CREAT|IPC_EXCL);
	if (msgid_ == -1) {
	  LOG(INFO) << msgFile_ << " is Exist";
	}
	msgid_ = msgget(key,S_IRUSR|S_IWUSR|IPC_CREAT);
}

MessageQueue::MessageQueue(string msgFile)
    : msgFile_(msgFile) {
	key_t key = ftok(msgFile_.c_str(),'a');
	msgid_ = msgget(key,S_IRUSR|S_IWUSR|IPC_CREAT|IPC_EXCL);
	if (msgid_ == -1) {
	  LOG(INFO) << msgFile_ << " is Exist";
	}
	msgid_ = msgget(key,S_IRUSR|S_IWUSR|IPC_CREAT);
}

MessageQueue::~MessageQueue() {
  //DeleteMsgQue();
}

void MessageQueue::Dump() const {
  printf("\n=====MessageQueue Dump START ========== \n");
  printf("msgid_=%d \n", msgid_);
  printf("msgFile_=%s ", msgFile_.c_str());
  printf("\n=====MessageQueue DUMP END ============\n");
}

int MessageQueue::GetMsgID() const {
  return msgid_;
}

string MessageQueue::GetMsgFile() const {
  return msgFile_;
}

void MessageQueue::SendMsg(struct rapidMsg* messagePtr) {
  //这里的第三个参数是char数组的长度，不能是sizeof(struct rapidMsg)
  int result = msgsnd(msgid_, messagePtr, _MYMSG_BUFFER_, 0);
  if (result == -1) {
	  LOG(ERROR) << "SendMsg Error!\n";
  }
}

void MessageQueue::RecvMsg(long type, struct rapidMsg* messagePtr) {
   int result= msgrcv(msgid_, messagePtr, _MYMSG_BUFFER_, type, 0);
   if (result == -1) {
	   LOG(ERROR) << "RecvMsg Error!\n";
   }
}

void MessageQueue::DeleteMsgQue() {
  msgctl(msgid_, IPC_RMID, NULL);
}
