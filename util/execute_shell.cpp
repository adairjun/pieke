#include "pieke/execute_shell.h" 
#include <string.h>

ExecuteShell::ExecuteShell() {

}

ExecuteShell::~ExecuteShell() {

}

void ExecuteShell::Dump() const {
  printf("\n=====ExecuteShell Dump START ========== \n");

  printf("\n=====ExecuteShell DUMP END ============ \n");
}

int ExecuteShell::execute(const char* cmd, string& output) {
  FILE *pf = popen(cmd, "r");
  if (!pf) {
	return -1;
  }

  output.clear();
  char tmp[1024];
  while (fgets(tmp, sizeof(tmp), pf) != NULL) {
  //为了输出方便些，把命令返回的换行符去掉
    if(tmp[strlen(tmp)-1] == '\n') {
      tmp[strlen(tmp)-1] = '\0';
    }
    output += tmp;
  }

  int status = pclose(pf);
  if(WIFEXITED(status)) {
	status = WEXITSTATUS(status);
  } else {
	status = -1;
  }
  return status;
}

int ExecuteShell::execute(const string& cmd, string& output) {
  return execute(cmd.c_str(), output);
}
