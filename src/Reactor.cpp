#include "Reactor.hpp"
#include <iostream>

using namespace std;

// Intialise the global pointer to NULL
Reactor* Reactor::reactor = NULL;


/*
 Function Name: getInstance
 paramer : None
 Output  : Reactor global instance
*/
Reactor* Reactor::getInstance(){

  if(reactor == NULL){
    reactor = new Reactor;
    cout<<"Created new instance";
  }else{
    cout<<"Sending old instance";
  }
  return reactor;
}

void Reactor::handleEvent(){
  cout<<"handle event";
}