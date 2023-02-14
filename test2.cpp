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
  std::ifstream test;
  test.open("/usr/bin/xterm");
  if (test)
    std::cout << "file exists" << std::endl;
}