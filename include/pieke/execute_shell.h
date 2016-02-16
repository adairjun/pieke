#ifndef PIEKE_INCLUDE_EXECUTE_SHELL_H_
#define PIEKE_INCLUDE_EXECUTE_SHELL_H_

#include <string>

using std::string;
/*
 * 其实这个功能一个non-member函数就能做
 */
class ExecuteShell {
 public:
  ExecuteShell();
  virtual ~ExecuteShell();
  void Dump() const;

  int execute(const char* command, string& output);
  int execute(const string& command, string& output);

};

#endif /* PIEKE_INCLUDE_EXECUTE_SHELL_H_ */
