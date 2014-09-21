#ifndef REACTOR_H
#define REACTOR_H

#include<vector>

class Reactor{

public:
  void registerEvent();
  void handleEvent();
  void start_reactor();
  static Reactor* getInstance();

private:
  static Reactor* reactor;
  
};

#endif



