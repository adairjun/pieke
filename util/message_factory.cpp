#include "pieke/message_factory.h"
#include <string.h>

MessageFactory::MessageFactory() {
}

MessageFactory::~MessageFactory() {
}

void MessageFactory::Dump() const {
  printf("\n=====MessageFactory Dump START ========== \n");
  printf("\n===MessageFactory DUMP END ============\n");
}

/*
 * singleton
 */
MessageFactory& MessageFactory::Instance() {
  static MessageFactory sInstance;
  return sInstance;
}

struct rapidMsg MessageFactory::CreateRapidMsg(long messageId_,
						string message_) {
	struct rapidMsg create_msg;
	create_msg.messageId = messageId_;
	create_msg.messageLength = message_.length();

	/*
	 * 写入message
	 */
    if (message_.length() <=  _MYMSG_BUFFER_) {
	  memcpy(create_msg.buffer, message_.c_str(), message_.length());
    } else {
      LOG(ERROR) << "buffer's space is not enough for message\n";
    }
	return std::move(create_msg);
}

void MessageFactory::ParseRapidMsg(const struct rapidMsg& myMsg_,
		          long& messageId_,
				  string& message_) {
  messageId_ = myMsg_.messageId;
  int messageLength = myMsg_.messageLength;

  message_ = string(myMsg_.buffer, messageLength);
}

