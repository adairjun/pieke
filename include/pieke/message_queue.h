#ifndef PIEKE_INCLUDE_MESSAGE_QUEUE_H_
#define PIEKE_INCLUDE_MESSAGE_QUEUE_H_

#include "message_factory.h"
#include <string>

using std::string;

class MessageQueue {
 public:
  explicit MessageQueue();
  explicit MessageQueue(string msgFile);
  MessageQueue(const MessageQueue&) = delete;
  MessageQueue& operator=(const MessageQueue&) = delete;
  virtual ~MessageQueue();

  void Dump() const;

  int GetMsgID() const;

  string GetMsgFile() const;

  void SendMsg(struct rapidMsg* messagePtr);

  void RecvMsg(long type, struct rapidMsg* messagePtr);

  void DeleteMsgQue();

 private:
  int msgid_;

  //用于ftok的msgFile_
  string msgFile_;

};

#endif /* PIEKE_INCLUDE_MESSAGE_QUEUE_H_ */
