#ifndef PIEKE_INCLUDE_MESSAGE_FACTORY_H_
#define PIEKE_INCLUDE_MESSAGE_FACTORY_H_

#include "logobj.h"
#include <string>

using std::string;


#define _MYMSG_BUFFER_ 8192

struct rapidMsg {
  long messageId;              //根据proto文件得到
  int messageLength;
  char buffer[_MYMSG_BUFFER_];
};

class MessageFactory {
 public:

  //和ClassFactory一样，这里也不能使用public的MessageFactory
  MessageFactory(const MessageFactory&) = delete;
  MessageFactory& operator=(const MessageFactory&) = delete;
  virtual ~MessageFactory();
  void Dump() const;

  static MessageFactory &Instance();

  /*
   * messageId是发送的消息id，定义在proto文件当中，比如在test.151000.153000.proto的 enum MessageType 当中
   * JUST_TEST_REQUEST = 151001;
   * 表明了JUST_TEST_REQUEST这个消息id是151001
   * message是要发送的消息
   */
  struct rapidMsg CreateRapidMsg(long messageId_,
						string message_);
  /*
   * 将myMsg给解析出来,使用引用参数来接收结果
   */
  void ParseRapidMsg(const struct rapidMsg& myMsg_,
		          long& messageId_,
				  string& message_);

 protected:
  //为了singleton模式，不能用public的构造函数，可以用protected和private
  MessageFactory();
};



#endif /* PIEKE_INCLUDE_MESSAGE_FACTORY_H_ */
