
#include "Logger.hpp"
#include <time.h>

using namespace std;

Logger* Logger::logger = NULL;

/**
   getInstance : Maintain only single instance of Logger class
   
**/
Logger* Logger::getInstance(){
  if(logger == NULL){
    logger = new Logger;
    cout << "new instance created \n";
  }
  return logger;
}


/**
   addOutputStream : Register output streams with logger
**/
bool Logger::addOutputStream(ostream *s, enum LOG_LEVEL level, string time_format){
  
  if(s == NULL){
    return false;
  }
  streamLoggers.push_back( Logger::BaseLogger(s, level, time_format));
  cout << "Output stream resgitered sucessfully";
  return true;
}


/**
   getTimeStamp : Returns current timestamp in give format
 **/

string Logger::getTimeStamp(const char *timeFormat ){
  time_t current_time;
  struct tm* time_info;
  char buffer[100];

  //get current time
  time(&current_time);
  //convert it to local time
  time_info = localtime(&current_time);
  // Formated time stamp
  strftime(buffer, 100, timeFormat, time_info);

  return string(buffer);
}

/**
   LOG : Log the data, exposed to user
**/

void Logger::LOG(LOG_LEVEL level, string str){
  if(level == ERROR){
    printErr(level, str);
  }else{
    printLog(level, str);
  }
}


/**
   printLog: Actual implmentation of logger     //TODO:Make it thread safe 
**/

void Logger::printLog(enum LOG_LEVEL level, string str){
 
  string log;

  switch(level){
    
  case INFO : log = "[INFO] ";
    break;
    
  case WARNING : log = "[WARNING] ";
    break;

  case DEBUG : log = "[DEBUG] ";
    break;
    
  case ERROR:
    break;
  }
  
  // Now print log to actual file
  for (list<Logger::BaseLogger>::iterator it=streamLoggers.begin(); it != streamLoggers.end(); ++it){
    Logger::BaseLogger b = (Logger::BaseLogger) *it;
    log = "[" + getTimeStamp(b.timeFormat.c_str()) + "]" + log + str + "\n";
    b.stream->write(log.c_str(), log.length());
  }
}

/**
   printErr : Print error in file as well as stderr
 **/
void Logger::printErr(enum LOG_LEVEL level, string str){
  
  string log;

  // print log to error console
  log = "[" + getTimeStamp("%F %T") + "]" + "[ERROR] " + str + "\n";
  cerr << log;

  // Now print log to actual file
  for (list<Logger::BaseLogger>::iterator it=streamLoggers.begin(); it != streamLoggers.end(); ++it){
    Logger::BaseLogger b = (Logger::BaseLogger) *it;
    log = "[" + getTimeStamp(b.timeFormat.c_str()) + "]" + "[ERROR] " + str + "\n";
    b.stream->write(log.c_str(), log.length());
  }
}
