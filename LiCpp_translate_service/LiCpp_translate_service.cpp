#include "LiC++.h"

#include <iostream>

using namespace std;
using namespace LiCpp;


pLiCppRuntime p_run = nullptr;
ThreadId messaging_id;

struct msg_request {
  msg_request(std::string _text, ThreadId _sender_id): text(_text), sender_id(_sender_id) {}
  std::string text;
  ThreadId sender_id;
};

struct msg_response {
  msg_response(std::string _text):text(_text) {}
  std::string text;
};

void msg_handler(pMsgBox incoming)
{
  incoming->wait()
  .handle_if<msg_request>(
  [&](msg_request const& msg) {
    return msg.text == "casa";
  },
  [&](msg_request const& msg) {
    p_run->send_message(msg.sender_id, msg_response("house"));
  }
  )
  .handle_if<msg_request>(
  [&](msg_request const& msg) {
    return msg.text == "blanca";
  },
  [&](msg_request const& msg) {
    p_run->send_message(msg.sender_id, msg_response("white"));
  }
  )
  .handle_default(
  [&](std::shared_ptr<messaging::message_base> const& msg) {
    auto wrapped_msg = dynamic_cast<messaging::wrapped_message<msg_request>*>(msg.get());
    if (wrapped_msg) {
      p_run->send_message(
        wrapped_msg->contents.sender_id,
        msg_response("Don't understand!")
      );
    }
  }
  );
}

void translate(ThreadId id, std::string text)
{
  ThreadId my_id = std::this_thread::get_id();
  p_run->send_message(id, msg_request(text, my_id));
  p_run->execute_blocking(
    my_id,
  [&](pMsgBox incoming) {
    incoming->wait()
    .handle<msg_response>(
    [&](msg_response const& msg) {
      std::cout << msg.text << std::endl;
    }
    );
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
    ThreadId my_id = std::this_thread::get_id();
    p_run->add_messaging_to_thread(my_id);

    while (!quit_pressed) {
      std::string in_data;
      std::cin >> in_data;
      if (in_data=="q") {
        p_run->remove_messaging_from_thread(my_id);
        quit_pressed=true;
      } else
        translate(messaging_id, in_data);
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
    ThreadId my_id = std::this_thread::get_id();
    p_run->add_messaging_to_thread(my_id);

    translate(messaging_id, "casa");
    translate(messaging_id, "blanca");
    translate(messaging_id, "humphrey");

    p_run->remove_messaging_from_thread(my_id);
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
