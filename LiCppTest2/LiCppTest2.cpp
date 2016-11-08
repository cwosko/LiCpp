//============================================================================
// Name        : LiC++Test2.cpp
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

class Worker {
  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run, int initial_count) {
      ThreadId my_id = p_run->spawn_messaging<Worker>(Worker::msg_handler, Worker());
      p_run->send_message(my_id, msg_tick(initial_count));
      return my_id;
    }

  private:
    Worker() {
    };
    static void msg_handler(pMsgBox incoming, std::shared_ptr<Worker> me) {
      incoming->wait()
      .handle<msg_tick>(
      [&](msg_tick const& msg) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (msg.counter < 1) {
          std::cout << "boom" << std::endl;
          throw std::runtime_error("boom");
        }
        std::cout << "tick" << std::endl;
        incoming->get_sender().send(msg_tick(msg.counter - 1));
      }
      );
    };

    struct msg_tick {
      int counter;
      msg_tick(int counter_):counter(counter_) {};
    };
};

class SuperVisor {
  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run) {
      ThreadId my_id = p_run->spawn_messaging<SuperVisor>(SuperVisor::msg_handler, SuperVisor(p_run));
      p_run->send_message(my_id, msg_init(my_id));
      return my_id;
    }

  private:
    SuperVisor(pLiCppRuntime p_run) : _p_run(p_run) {
    }

    static void msg_handler(pMsgBox incoming, std::shared_ptr<SuperVisor> me) {
      incoming->wait()
      .handle<msg_init>(
      [&](msg_init const& msg) {
        me->_own_id = msg.id;
        ThreadId worker_id = Worker::CreateAndRun(me->_p_run, 20);
        me->_p_run->link_thread_to_supervisor(worker_id, me->_own_id);
      }
      )
      .handle<msg_crash>(
      [&](msg_crash const& msg) {
        ThreadId worker_id = Worker::CreateAndRun(me->_p_run, 20);
        me->_p_run->link_thread_to_supervisor(worker_id, me->_own_id);
      }
      );
    };

    ThreadId _own_id;
    pLiCppRuntime _p_run;
    struct msg_init {
      ThreadId id;
      msg_init(ThreadId id_):id(id_) {};
    };
};


int main()
{
  try {
    pLiCppRuntime p_run = make_shared<LiCppRuntime>();
    ThreadId sv_id = SuperVisor::CreateAndRun(p_run);

    bool test_done=false;
    std::thread test_thread([&]() {
      std::this_thread::sleep_for(std::chrono::seconds(5));
      p_run->suspend_thread(sv_id);
      std::this_thread::sleep_for(std::chrono::seconds(3));
      p_run->resume_thread(sv_id);
      std::this_thread::sleep_for(std::chrono::seconds(3));
      p_run->kill_thread(sv_id);
      std::this_thread::sleep_for(std::chrono::seconds(5));
      test_done=true;
    });

    while(!test_done) {
      p_run->exec();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    test_thread.join();
    std::cout << "Game over!" << std::endl;
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
