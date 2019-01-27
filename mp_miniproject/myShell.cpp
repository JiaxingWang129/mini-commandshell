//Citing from man page
//Citing from assignment 097

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <map>
#include <locale>
#include <fcntl.h>

void replaceEscape(std::string & cmd);

std::string listDir(const char * path, std::string cmdName){
  DIR * d;
  struct dirent * dir;
  std::string childpath;
  d = opendir(path);
  if(d == NULL){
    return "";
  }
  std::string filename = "";
  while ((dir = readdir(d)) != NULL){
    if((dir -> d_type & DT_DIR) == DT_DIR){
      if(strcmp(dir -> d_name, ".") == 0 || strcmp(dir -> d_name, "..") == 0){
	continue;
      }
      else{
	childpath = std::string(path) + "/" + std::string(dir -> d_name);
	listDir(childpath.c_str(), cmdName);
      }
    }
    else{
      if((dir -> d_name) == cmdName){
	filename = std::string(path) + "/" + std::string(dir -> d_name);
      }
    }
  }
  closedir(d);
  return filename;
}

std::string path(std::string cmdName){
  std::string str;
  str = getenv("PATH");
  std::stringstream ss(str);
  std::string fullPath;
  while(getline(ss, str, ':')){
    fullPath = listDir(str.c_str(), cmdName);
    if(fullPath != ""){
      return fullPath;
    }
  }
  return fullPath;
}

void divArg(std::vector<std::string> & arguments, std::string & arg, int & argCount){
  std::string temp;
  for(size_t j = 0; j < arg.length(); j++){
    if(arg[j] == ' '){
      if(arg[j - 1] != '\\' && arg[j + 1] != ' '){
	replaceEscape(temp);
	arguments.push_back(temp);
	argCount++;
	temp.erase(temp.begin(), temp.end());
      }
      if(arg[j - 1] == '\\'){
	temp.push_back(arg[j]);
      }
      continue;
    }
    else{
      temp.push_back(arg[j]);
    }
    if(j == arg.length() - 1){
      replaceEscape(temp);
      arguments.push_back(temp);
      argCount++;
    }
  }
}

void exec_withoutArg(std::string str, char ** env, std::string cmdName){
  char *newargv[] = {(char *)str.c_str(), NULL};
  /*if(str == ""){
    std::cerr << cmdName << ": command not found" << std::endl;
    exit(EXIT_FAILURE);
    }*/
  execve(str.c_str(), newargv, env);
  exit(EXIT_FAILURE);
}

void dealRedir(std::string & argWithRe, std::string & arg){
  //deal with >
  //  std::cout << argWithRe << std::endl;
  std::string filename_out;
  if(argWithRe.find('>') != std::string::npos){
    for(size_t i = argWithRe.length() - 1; i >= 0; i--){
      if(argWithRe[i] == '>' && argWithRe[i - 1] != '\\'){
	size_t j = i + 1;
	while(arg[j] == ' '){
	  j++;
	}
	size_t begin = j;
	//std::cout << begin << std::endl;
	while(!((arg[j] == '<' && arg[j - 1] !='\\') || (arg[j] == '2' && arg[j + 1] == '>' && arg[j - 1] !='\\') || (arg[j] == ' ' && arg[j - 1] != '\\')) && j != argWithRe.length() - 1){
	  j++;
	}
	if(j != argWithRe.length() - 1){
	  j--;
	}
	//	std::cout << j << std::endl;
	if(argWithRe[begin - 1] == ' '){
	  filename_out = argWithRe.substr(begin, j - i - 1);
	}
	else{
	  filename_out = argWithRe.substr(begin, j - i);
	}
	replaceEscape(filename_out);
	break;
      }
    }
    //std::cout << filename_out << std::endl;
    if(filename_out != ""){
      int newfd_out = open((char *)filename_out.c_str(), O_CREAT|O_RDWR|O_TRUNC, S_IRWXU | S_IRGRP | S_IWGRP | S_IWUSR);
      dup2(newfd_out, 1);
    }
  }
  //deal with <
  std::string filename_in;
  if(argWithRe.find('<') != std::string::npos){
    for(size_t i = argWithRe.length() - 1; i >= 0; i--){
      if(argWithRe[i] == '<' && argWithRe[i - 1] != '\\'){
	size_t j = i + 1;
	while(arg[j] == ' '){
	  j++;
	}
	size_t begin = j;
	//	std::cout << begin << std::endl;
	while(!((arg[j] == '>' && arg[j - 1]!='\\') || (arg[j] == '2' && arg[j + 1] == '>' && arg[j - 1] !='\\') || (arg[j] == ' ' && arg[j - 1] != '\\')) && j != argWithRe.length() - 1){
	  j++;
	}
	if(j != argWithRe.length() - 1){
	  j--;
	}
	//std::cout << j << std::endl;
	if(argWithRe[begin - 1] == ' '){
	  filename_in = argWithRe.substr(begin, j - i - 1);
	}
	else{
	  filename_in = argWithRe.substr(begin, j - i);
	}
	replaceEscape(filename_in);
	break;
      }
    }
    //std::cout << filename_in << std::endl;
    if(filename_in != ""){
      int newfd_in = open((char *)filename_in.c_str(), O_RDONLY, S_IRWXU | S_IRGRP | S_IWGRP | S_IWUSR);
      if(newfd_in == -1){
	std::cerr << "No such file." << std::endl;
	exit(EXIT_FAILURE);
      }
      dup2(newfd_in, 0);
    }
  }
  //deal with 2>
  if(argWithRe.find("2>") != std::string::npos){
    std::string filename_err;
    for(size_t i = argWithRe.length() - 1; i >= 0; i--){
      if(arg[i] == '2' && arg[i + 1] == '>' && arg[i - 1] !='\\'){
	size_t j = i + 2;
	while(arg[j] == ' '){
	  j++;
	}
	size_t begin = j;
	while(!((arg[j] == '>' && arg[j - 1] !='\\') || (arg[j] == '<' && arg[j - 1] !='\\') || (arg[j] == ' ' && arg[j - 1] != '\\')) && j != argWithRe.length() - 1){
	  j++;
	}
	if(j != argWithRe.length() - 1){
	  j--;
	}
	//std::cout << j << std::endl;
	if(argWithRe[begin - 1] == ' '){
	  filename_err = argWithRe.substr(begin, j - i - 1);
	}
	else{
	  filename_err = argWithRe.substr(begin, j - i);
	}
	replaceEscape(filename_err);
	break;
      }
    }
    if(filename_err != ""){
      close(2);
      int newfd = open((char *)filename_err.c_str(), O_RDWR, S_IRWXU | S_IRGRP | S_IWGRP | S_IWUSR);
      dup2(newfd, 2);
    }
  }
}

void exec_withArg(std::string str, std::string arg, char ** env, std::string cmdName){
  std::vector<std::string> arguments;
  std::string argWithoutRe;
  std::string argWithRe;
  char * newargv[] = {(char*)str.c_str()};
  int argCount = 1;
  for(size_t i = 0; i < arg.length(); i++){
    if((arg[i] == '>' && arg[i - 1] !='\\' && arg[i - 1] != '2') || (arg[i] == '<' && arg[i - 1] !='\\') || (arg[i] == '2' && arg[i + 1] == '>' && arg[i - 1] !='\\')){
      argWithoutRe = arg.substr(0, i);
      argWithRe = arg.substr(i);
      break;
    }
    else{
      argWithoutRe = arg;
    }
  }
  dealRedir(argWithRe, arg);
  divArg(arguments, argWithoutRe, argCount);
  for(int k = 1; k < argCount; k++){
    //because newargv[0] is filename, so it has to begin from newargv[1]
    newargv[k] = (char *)arguments[k - 1].c_str();
  }
  newargv[argCount] = NULL;
  /*if(str == ""){
    std::cerr << cmdName << ": command not found" << std::endl;
    exit(EXIT_FAILURE);
    }*/
  execve(str.c_str(), newargv, env);
  exit(EXIT_FAILURE);
}

void wait(pid_t cpid){
  int status;
  int ret = waitpid(cpid, &status, 0);
  if(ret){
    if(WIFEXITED(status)){
      std::cout << "Program exited with status " << WEXITSTATUS(status) << std::endl;
    }
    else if(WIFSIGNALED(status)){
      std::cout << "Program was killed by signal " << WTERMSIG(status) << std::endl;
    }
  }
}

std::string parseFullPath(std::string str){
  std::string fullPath;
  std::size_t found = str.find("/");
  //command without "/"
  if(found == std::string::npos){
    fullPath = path(str);
  }
  //command with "/"
  else{
    std::size_t path_file = str.find_last_of("/");
    fullPath = listDir(str.substr(0, path_file).c_str(), str.substr(path_file + 1));
  }
  return fullPath;
}

void divCmdArg(std::string &cmd, std::string &cmdName, std::string &arg){
  size_t i = 1;
  while(true){
    if(((cmd[i] == ' ' || cmd[i] == '<' || cmd[i] == '>' || (cmd[i] == '2' && cmd[i + 1] == '>')) && cmd[i - 1] != '\\') || i == cmd.length()){
      break;
    }
    i++;
  }
  if(i != cmd.length()){
    cmdName = cmd.substr(0, i);
    while(cmd[i] == ' '){
      i++;
    }
    arg = cmd.substr(i);
  }
  else{
    cmdName = cmd;
  }
}

void changeDir(std::string arg){
  int argCount = 0;
  std::vector<std::string> arguments;
  divArg(arguments, arg, argCount);
  int dir_err = chdir((char *)arguments[0].c_str());
  if(dir_err == -1){
    perror("cd");
  }
  char * currDirName = get_current_dir_name();
  setenv("PWD", currDirName, 1);
  free(currDirName);
}

void setEnv(std::string & arg, std::map<std::string, std::string> & envVar){
  int argCount = 0;
  std::vector<std::string> arguments;
  divArg(arguments, arg, argCount);
  for(size_t i = 0; i < arguments[0].length(); i++){
    if(!(arguments[0][i] == '_' || isalpha(arguments[0][i]) || isdigit(arguments[0][i]))){
      std::cout << "Invalid var name." << std::endl;
      return;
    }
  }
  std::string varValue;
  if(argCount == 1){
    varValue = "";
  }
  else{
    std::size_t found = arg.find(arguments[0]);
    varValue = arg.substr(found + arguments[0].length() + 1);
  }
  std::map<std::string, std::string>::iterator it = envVar.find(arguments[0]);
  if(it == envVar.end()){
    envVar.insert(std::pair<std::string, std::string>(arguments[0], varValue));
  }
  else{
    envVar[arguments[0]] = varValue;
  }
}

void replaceVarname(std::map<std::string, std::string> & envVar, std::string & cmd){
  std::string tempVarName;
  std::string tempCmd = cmd;
  std::string ans;
  size_t i = 0;
  while(i != tempCmd.length()){
    if(tempCmd[i] == '$'){
      size_t k = i + 1;
      if(!(tempCmd[k] == '_' || isalpha(tempCmd[k]) || isdigit(tempCmd[k]))){
	ans.push_back('$');
	i++;
      }
      else{
	while(tempCmd[k] == '_' || isalpha(tempCmd[k]) || isdigit(tempCmd[k])){
	  k++;
	}
	std::map<std::string, std::string>::iterator it = envVar.find(tempCmd.substr(i+1, k-i-1));
	if(it != envVar.end()){
	  ans += it -> second;
	}
	i = k;
      }
    }
    else{
      ans.push_back(tempCmd[i]);
      i++;
    }
  }
  cmd = ans;
}

void replaceEscape(std::string & cmd){
  std::string ans;
  size_t i = 0;
  while(i != cmd.length()){
    if(cmd[i] == '\\' && i != cmd.length() - 1){
      ans.push_back(cmd[i + 1]);
      i += 2;
    }
    else{
      if(i == cmd.length() - 1 && cmd[i] == '\\'){
	i++;
	continue;
      }
      ans.push_back(cmd[i]);
      i++;
    }
  }
  cmd = ans;
}

void exportEnv(std::string & arg, std::map<std::string, std::string> & envVar){
  int argCount = 0;
  std::vector<std::string> arguments;
  divArg(arguments, arg, argCount);
  for(int i = 0; i < argCount; i++){
    std::map<std::string, std::string>::iterator it = envVar.find(arguments[i]);
    if(it != envVar.end()){
      setenv((char *)it -> first.c_str(), (char *)it -> second.c_str(), 1);
    }
  }
}

void skipHeadSpace(std::string & str){
  std::size_t pos = str.find_first_not_of(" ");
  if(pos != std::string::npos){
    str = str.substr(pos);
  }
  else{
    str = "";
  }
}

int main(){
  std::map<std::string, std::string> envVar;
  while(true){
    std::string fullPath;
    std::string cmdName;
    std::string arg;
    char * currPath = getenv("PWD");
    std::cout << "myShell:" << currPath << "$ ";
    std::string cmd;
    if(!std::getline(std::cin, cmd)){
      return 0;
    }
    if(cmd == ""){
      continue;
    }
    skipHeadSpace(cmd);
    divCmdArg(cmd, cmdName, arg);
    replaceEscape(cmdName);
    replaceVarname(envVar, cmdName);
    replaceVarname(envVar, arg);
    if(cmdName == "exit"){
      return EXIT_SUCCESS;
    }
    if(cmdName == "cd"){
      if(arg == ""){
	continue;
      }
      changeDir(arg);
    }
    if(cmdName == "set"){
      setEnv(arg, envVar);
      }
    if(cmdName == "export"){
      exportEnv(arg, envVar);
    }
    fullPath = parseFullPath(cmdName);
    if(fullPath == "" && cmdName != "cd" && cmdName != "set" && cmdName != "export"){
      std::cerr << cmdName << ": command not found" << std::endl;
      continue;
    }
    if(cmdName != "cd" && cmdName != "set" && cmdName != "export"){
      pid_t cpid = fork();
      //fork failure
      if(cpid < 0){
	return EXIT_FAILURE;
      }
      //child process
      else if (cpid == 0){
	//execute without arguments
	if(arg == ""){
	  exec_withoutArg(fullPath, environ, cmdName);
	}
	//execute with arguments
	else{
	  exec_withArg(fullPath, arg, environ, cmdName);
	}
      }
      else{
	wait(cpid);
      }
    }
  }
  return EXIT_SUCCESS;
}
