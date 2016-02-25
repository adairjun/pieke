#include <iostream>
#include "master_thread.h"

using namespace std;

int main() {

  //在主函数main中，MasterThread的实例是在栈上构造，没有使用boost::shared_ptr<MasterThread> 的构造方式，
  //所以boost::enable_shared_from_this<MasterThread>中的weak_ptr所指的函数对象也就没有被赋值，
  // 如果使用MasterThread masterThread的话将会抛出错误
  boost::shared_ptr<MasterThread> masterThreadPtr(new MasterThread);
  if (!masterThreadPtr->InitMasterThread()) {
	LOG(ERROR) << "InitNetCore failed.\n";
    return -1;
  }
  masterThreadPtr->Run();
  return 0;
}
