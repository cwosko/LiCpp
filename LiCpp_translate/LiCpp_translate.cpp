#include "helper.h"
#include "LiC++.h"

#include <iostream>

using namespace std;
using namespace LiCpp;


pLiCppRuntime p_run = nullptr;
ThreadId messaging_id;

template <int crc> struct msg_hash {	};

void msg_handler(pMsgBox incoming)
{
  incoming->wait()
  .handle<msg_hash<CRC("casa")> >(
  [&]() {
    std::cout << "house" << std::endl;
  }
  )
  .handle<msg_hash<CRC("blanca")> >(
  [&]() {
    std::cout << "white" << std::endl;
  }
  )
  .handle_default(
  [&]() {
    std::cout << "I don't understand." << std::endl;
  }
  );
}

#ifndef TEST

int main()
{
  p_run = make_shared<LiCppRuntime>();

  messaging_id = p_run->spawn_messaging(msg_handler);

  bool quit_pressed=false;
  std::thread char_thread(
  [&]() {
    while (!quit_pressed) {
      char c = getchar();
      if (c=='q')
        quit_pressed=true;
      if (c=='c')
        p_run->send_message(messaging_id, msg_hash<CRC("casa")>());
      if (c=='b')
        p_run->send_message(messaging_id, msg_hash<CRC("blanca")>());
      if (c=='h')
        p_run->send_message(messaging_id, msg_hash<CRC("humphrey")>());
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
  p_run = make_shared<LiCppRuntime>();

  messaging_id = p_run->spawn_messaging(msg_handler);

  bool test_done=false;
  std::thread test_thread(
  [&]() {
    p_run->send_message(messaging_id, msg_hash<CRC("casa")>());
    p_run->send_message(messaging_id, msg_hash<CRC("blanca")>()	);
    p_run->send_message(messaging_id, msg_hash<CRC("humphrey")>());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
