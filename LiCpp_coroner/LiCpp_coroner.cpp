#include "LiC++.h"

#include <iostream>

using namespace std;
using namespace LiCpp;

struct msg_number {
  msg_number(uint8_t _number): number(_number) {}
  uint8_t number;
};

void roulette(pMsgBox incoming)
{
  incoming->wait()
  .handle_if<msg_number>(
  [&](msg_number const& msg) {
    return msg.number == 3;
  },
  [&]() {
    std::cout << "bang." << std::endl;
    throw std::runtime_error("Dead by bullet!");
  }
  )
  .handle_default(
  [&]() {
    std::cout << "click." << std::endl;
  }
  );
}

void coroner(pMsgBox incoming)
{
  incoming->wait()
  .handle<msg_crash>(
  [&](msg_crash const& msg) {
    std::cout << "Player died." << std::endl;
    if (msg.exc_ptr) {
      try {
        std::rethrow_exception(msg.exc_ptr);
      } catch (const std::runtime_error& err) {
        std::cout << "Reason: " << err.what() << std::endl;
      }
    }
  }
  );
}

#ifndef TEST

int main()
{
  pLiCppRuntime p_run = make_shared<LiCppRuntime>();

  ThreadId roulette_id = p_run->spawn_messaging(roulette);
  ThreadId coroner_id = p_run->spawn_messaging(coroner);
  p_run->link_thread_to_supervisor(roulette_id, coroner_id);

  bool quit_pressed=false;
  std::thread char_thread(
  [&]() {
    while (!quit_pressed) {
      char c = getchar();
      if (c=='q')
        quit_pressed=true;
      else if ((c>='0') && (c<='9')) {
        p_run->send_message(roulette_id, msg_number(c-'0'));
        if (!p_run->thread_registered(roulette_id))
          std::cout << "Player is dead!" << std::endl;
      }
    }
  }
  );

  while(!quit_pressed) {
    p_run->exec();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  char_thread.join();
  return 0;
}

#else

int main()
{
  pLiCppRuntime p_run = make_shared<LiCppRuntime>();

  ThreadId roulette_id = p_run->spawn_messaging(roulette);
  ThreadId coroner_id = p_run->spawn_messaging(coroner);
  p_run->link_thread_to_supervisor(roulette_id, coroner_id);

  bool test_done=false;
  std::thread test_thread(
  [&]() {
    p_run->send_message(roulette_id, msg_number(2));
    p_run->send_message(roulette_id, msg_number(4));
    p_run->send_message(roulette_id, msg_number(3));
    p_run->send_message(roulette_id, msg_number(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (!p_run->thread_registered(roulette_id))
      std::cout << "Player is dead!" << std::endl;
    test_done=true;
  }
  );

  while(!test_done) {
    p_run->exec();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  test_thread.join();
  return 0;
}

#endif
