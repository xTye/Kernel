// Written by:    Robert "Tyler" Riley
// For:           COP 4600, Spring 2022

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

#define MAX_PROCESSES 1000

int i = 0;
vector<string> h;
pid_t processes[MAX_PROCESSES];
int pcnt = 0;

string parse(string s);
string terminateall();

// Delay function
void delay(int number_of_seconds)
{
    int milli_seconds = 1000 * number_of_seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds);
}

// Custom tokenize function
// Parses the first word in the in the string, and returns the rest in a vector
vector<string> tokenize(string s, string t)
{
  vector<string> tokens;

  int start = 0;
  int end = s.find(t);

  // Check if nothing to parse
  if (end == -1)
  {
    tokens.push_back(s);
    tokens.push_back("");
    return tokens;
  }

  tokens.push_back(s.substr(start, end));
  tokens.push_back(s.substr(end + 1, s.size()));

  return tokens;
}

// Returns the program name from the path
string getProgram(string s)
{
  string p;

  while(s != "")
  {
    vector<string> tokens = tokenize(s, "/");
    p = tokens.at(0);
    s = tokens.at(1);
  }

  return p;
}

// History function
// Returns error
string history(string s)
{
  vector<string> tokens = tokenize(s, " ");
  
  // No arguments
  // Cycle through the history in memory in reverse order
  if (s.empty())
  {
    if (i == 0)
      return "";
    int temp = i-1;
    while (temp >= 0) {cout << i-1 - temp << ": " << h.at(temp) << endl; temp--;}
  }
  // Command to clear | clear history
  else if (s == "-c")
  {
    i = 0;
    h.clear();
  }
  else
  {
    return "Command not recognizable.";
  }

  return "";
}

// Function to close program
void byebye()
{
  ofstream f;
  f.open("mysh.history");

  // Prints history in memory to file
  int temp = 0;
  while (temp < i)
  {
    (temp == i-1) ? f << h.at(temp) : f << h.at(temp) << endl;
    temp++;
  }

  f.close();
  
  // Kill all child processes.
  terminateall();

  exit(0);
}

// Replays a function from memory
// Returns error
string replay(string s)
{
  if (s.empty())
    return "The 'replay' commands needs a number to replay";

  if ((i - stoi(s) -2) < 0)
    return "Replay command out of range";

  parse(h.at(i - stoi(s) -2));
  return "";
}

// Creates a child and starts a program
// Returns error
string start(string s)
{
  char * argv_list[1024];
  string err = "";
  string directory = "";
  int n = 0;

  // Reads all arguments and converts it to type char *, for exec family.
  while (s != "" && i < 1024)
  {
    vector<string> tokens = tokenize(s, " ");
    string arg  = tokens.at(0);

    // Special directives for using directory.
    if (arg.at(0) == '/' && directory == "")
    {
      ifstream valid;
      valid.open(tokenize(s, " ").at(0));
      if (!valid)
        return "Directory '" + s + "' cannot be found";

      directory = arg;
      arg = getProgram(arg);
    }

    argv_list[n] = (char *)malloc(sizeof(char) * arg.size() + 1);
    strcpy(argv_list[n], &arg[0]);
    s = tokens.at(1);
    n++;
  }

  if (n >= 1024)
  {
    for (int it = 0; it < n; it++)
      free(argv_list[it]);
    
    return "Too manny command arguments";
  }

  argv_list[n] = NULL;

  
  pid_t pid = fork();
  int status;
  
  if (pid == -1)
  {
    err = "Can't fork, error occured";
  }
  // Child processes
  else if (pid == 0)
  {
    if (directory == "")
    {
      execvp(argv_list[0], argv_list);
    }
    else
    {
      execv(&directory[0], argv_list);
    }

    exit(0);
  }
  // Parent process
  else
  {
    // Wait for child process
    if (waitpid(pid, &status, 0) > 0)
    {
      if (WIFEXITED(status) && WEXITSTATUS(status))
      {
        if (WEXITSTATUS(status) == 127)
          err = "Execv failed";
      }
    }
  }

  for (int it = 0; it < n; it++)
    free(argv_list[it]);
  
  return err;
}

string background(string s)
{
  char * argv_list[1024];
  string err = "";
  string directory = "";
  int n = 0;

  // Reads all arguments and converts it to type char *, for exec family.
  while (s != "" && n < 1024)
  {
    vector<string> tokens = tokenize(s, " ");
    string arg  = tokens.at(0);

    // Special directives for using directory.
    if (arg.at(0) == '/' && directory == "")
    {
      ifstream valid;
      valid.open(tokenize(s, " ").at(0));
      if (!valid)
        return "Directory '" + s + "' cannot be found";

      directory = arg;
      arg = getProgram(arg);
    }

    argv_list[n] = (char *)malloc(sizeof(char) * arg.size() + 1);
    strcpy(argv_list[n], &arg[0]);
    s = tokens.at(1);
    n++;
  }

  if (n >= 1024)
  {
    for (int it = 0; it < n; it++)
      free(argv_list[it]);
    
    return "Too manny command arguments";
  }

  argv_list[n] = NULL;

  // CHILD PROCESS CREATED
  pid_t pid = fork();
  processes[pcnt] = pid;
  pcnt++;
  int status;
  
  if (pid == -1)
  {
    err = "Can't fork, error occured";
  }
  // Child process
  else if (pid == 0)
  {
    int exec = 1;
    cout << "Running process with process id: " << getpid() << endl;

    if (directory == "")
    {
      exec = execvp(argv_list[0], argv_list);
    }
    else
    {
      exec = execv(&directory[0], argv_list);
    }
    
    // Can't return error on child process because of exit
    if (exec == -1)
      cout << "Program could not be executed" << endl;

    exit(0);
  }

  delay(1);

  for (int it = 0; it < n; it++)
    free(argv_list[it]);
  
  return err;
}

// Terminates a program
// Returns error
string term(string s)
{
  pid_t pid = (pid_t)stoi(s);
  // KILL
  int result = kill(pid, 15);
  string ret = to_string(result);
  int index = -1;

  // Reorganize structure of process
  for (int it = 0; it < MAX_PROCESSES; it++)
    if (processes[it] == pid)
      index = it;
  for (int it = index+1; it < pcnt; it++)
    processes[it-1] = processes[it];

  pcnt--;

  // Can't return an error in terminate all
  cout << "Killed process with id: '" + s + "' wth exit code '" + ret + "'" << endl;
  return "";
}

// Repeat a command
// Returns error
string repeat(string s)
{
  vector<string> tokens = tokenize(s, " ");
  int n = stoi(tokens.at(0));
  s = tokens.at(1);

  // Check if a valid directory
  ifstream valid;
  valid.open(tokenize(s, " ").at(0));
  if (!valid && s.at(0) == '/')
    return "Directory '" + s + "' cannot be found";

  // Create children processes
  for (int it = 0; it < n; it++)
    background(s);

  return "";
}

// Terminates all child processes
// Returns error
string terminateall()
{
  string err = "";

  if (pcnt == 0)
    return "No processes are currently running";

  string num = to_string(pcnt);
  err = "Teminating " + num + " processes.";

  for (int it = pcnt-1; it >= 0; it--)
    term(to_string((int)processes[it]));

  return err;
}

// Parse functions that manages all commands
// Returns error truncated down from individual functions
string parse(string s)
{
  vector<string> tokens = tokenize(s, " ");
  string t = tokens.at(0);
  s = tokens.at(1);
  string err = "";

  if (t == "history")
  {
    err = history(s);
  }
  else if (t == "byebye")
  {
    byebye();
  }
  else if (t == "replay")
  {
    err = replay(s);
  }
  else if (t == "start")
  {
    err = start(s);
  }
  else if (t == "background")
  {
    err = background(s);
  }
  else if (t == "terminate")
  {
    err = term(s);
  }
  else if (t == "repeat")
  {
    err = repeat(s);
  }
  else if (t == "terminateall")
  {
    err = terminateall();
  }
  else
  {
    err = "'" + t + "' command not found";
  }

  return err;
}

// Parses the history file if it exists and adds the commands to history
void parse_file()
{
  ifstream f;
  f.open("mysh.history");

  if (f.is_open())
  {
    string temp;    

    while (getline(f, temp))
    {
      h.push_back(temp);
      i++;
    }
  }
  
  f.close();
}

int main()
{
  // Initialize processes array
  for (int it = 0; it < MAX_PROCESSES; it++)
    processes[it] = (pid_t)0;
  
  parse_file();
  string s;

  while (true)
  {
    cout << "# ";
    getline(cin, s);

    h.push_back(s);
    i++;

    string err = parse(s);
    if (err != "")
      cout << err << endl;
  }
}