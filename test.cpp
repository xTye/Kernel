#include <stdio.h>
#include <sys/types.h>
#include <unistd.h> 
#include <stdlib.h>
#include <errno.h>  
#include <sys/wait.h>

#include <iostream>
#include <fstream>

int main()
{
  pid_t  pid;
  int ret = 1;
  int status;
  pid = fork();
  
  if (pid == -1)
  {
    printf("can't fork, error occured\n");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    char * argv_list[] = {"xterm", NULL};

    execv("/usr/bin/xterm",argv_list);
    exit(0);
  }
  else
  {
    if (waitpid(pid, &status, 0) > 0)
    {
      if (WIFEXITED(status) && !WEXITSTATUS(status)) 
        printf("program execution successful\n");
            
      else if (WIFEXITED(status) && WEXITSTATUS(status))
      {
        if (WEXITSTATUS(status) == 127)
          printf("execv failed\n");
        else 
          printf("program terminated normally, but returned a non-zero status\n");                
        }
      else 
        printf("program didn't terminate normally\n");            
    } 
    else
      printf("waitpid() failed\n");
  }

  std::string s;
  while(s != "q") { getline(std::cin, s); }
   
  return 0;
}