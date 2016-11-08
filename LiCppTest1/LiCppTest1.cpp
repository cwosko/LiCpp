//============================================================================
// Name        : LiC++Test.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <stdio.h>
#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include "LiC++.h"

using namespace std;
using namespace LiCpp;

void looping_func(std::function<void()> check_for_interruptions)
{
  for (;;) {
    check_for_interruptions();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "tack" << std::endl;
  }
}

void cyclic_func()
{
  static auto last = std::chrono::system_clock::now();
  auto now = std::chrono::system_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() > 100) {
    std::cout << "tick" << std::endl;
    last = now;
  }
}

pLiCppRuntime p_run = nullptr;
ThreadId looping_id;
ThreadId cyclic_id;
ThreadId messaging_id;

void dummy_crash_handler(ThreadId crashed_id, pException exc_ptr)
{
}

void crash_handler(ThreadId crashed_id, pException exc_ptr)
{
  if ((p_run) && (crashed_id == looping_id)) {
    std::cout << "Resurrecting tack ones ..." << std::endl;
    p_run->register_crash_handler(dummy_crash_handler);
    looping_id = p_run->spawn_looping(looping_func);
  }
};

struct Storage {
  std::string text;
  enum {init,ping,pong} state;
  std::vector<std::string> texts;
};

struct msg_ping {};
struct msg_pong {};

void msg_handler(pMsgBox incoming, std::shared_ptr<Storage> storage)
{
  incoming->wait()
  .handle<msg_ping>(
  [&](msg_ping const& msg) {
    std::cout << "ping" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    incoming->get_sender().send(msg_pong());
  }
  )
  .handle<msg_pong>(
  [&](msg_pong const& msg) {
    std::cout << "pong" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    incoming->get_sender().send(msg_ping());
  }
  );
}


int main()
{
  p_run = make_shared<LiCppRuntime>();
  p_run->register_crash_handler(crash_handler);

  Storage storage;
  looping_id = p_run->spawn_looping(looping_func);
  cyclic_id = p_run->spawn_cyclic(cyclic_func);
  messaging_id = p_run->spawn_messaging<Storage>(msg_handler, std::move(storage));

  p_run->send_message(messaging_id, msg_ping());

  bool test_done=false;
  std::thread test_thread([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Pausing all ..." << std::endl;
    p_run->suspend_thread(looping_id);
    p_run->suspend_thread(cyclic_id);
    p_run->suspend_thread(messaging_id);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Resuming all ..." << std::endl;
    p_run->resume_thread(looping_id);
    p_run->resume_thread(cyclic_id);
    p_run->resume_thread(messaging_id);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Killing all ..." << std::endl;
    p_run->kill_thread(looping_id);
    p_run->kill_thread(cyclic_id);
    p_run->kill_thread(messaging_id);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Killing tack ..." << std::endl;
    p_run->kill_thread(looping_id);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    test_done=true;
  });

  while (!test_done) {
    p_run->exec();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  test_thread.join();

  return 0;
}
