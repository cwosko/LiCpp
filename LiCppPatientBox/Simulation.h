#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "Common.h"
#include "LiC++.h"

#include <random>
#include <sstream>
#include <iostream>

using namespace std;
using namespace LiCpp;

/* SimpleSensor
   A mock for a sensor HW, generating some random numeric values. */
template <typename Sensor>
class SensorSim {
    friend class Temperature;
    friend class Pulse;
    friend class BloodPressure;
    friend class Breathing;

  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run) {
      ThreadId my_id = p_run->spawn_messaging<Sensor>(Sensor::msg_handler, Sensor(p_run));
      std::cout << "Started sensorSim. (type=" << Sensor::type_string << ", PID=" << my_id << ").\n";
      return my_id;
    }

    typedef enum eState {
      working,
      broken
    } State;

  private:
    explicit SensorSim(pLiCppRuntime p_run_) : p_run(p_run_), state(working), generator(), distribution(0.0,1.0) {
    }

    static void msg_handler(pMsgBox incoming, std::shared_ptr<Sensor> me) {
      incoming->wait()
      .template handle<msg_query>( [&](msg_query const& msg) {
        me->state_change();
        me->send_value(msg.src);
      } );
    };


    void state_change() {
      static const double BREAK = 0.01;
      static const double REPAIR = 0.005;
      std::stringstream txt;

      generator.seed(time(0));
      double number = distribution(generator);
      if ((state == working) && (number < BREAK)) {
        txt << "Sensor (type=" << Sensor::type_string
            << ", ID=" << p_run->current_thread_id()
            << ") is now broken.\n";
        std::cout << txt.str();
        state = broken;
      } else if ((state == broken) && (number < REPAIR)) {
        txt << "Sensor (type=" << Sensor::type_string
            << ", ID=" << p_run->current_thread_id()
            << ") is now working again.\n";
        std::cout << txt.str();
        state = working;
      }
    }

    virtual void send_value(ThreadId dst) = 0;

    pLiCppRuntime p_run;
    State state;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution;
};


class Temperature : SensorSim<Temperature> {
    friend class SensorSim<Temperature>;
    using SensorSim::SensorSim;

  public:
    static constexpr const char* type_string = "Temperature";
  private:
    virtual void send_value(ThreadId dst) {
      static const double base = 36;
      generator.seed(time(0));
      double variation = 4*distribution(generator);
      if (state == working)
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {base+variation-1}));
      else
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {0}));
    }
};

class Pulse : SensorSim<Pulse> {
    friend class SensorSim<Pulse>;
    using SensorSim::SensorSim;

  public:
    static constexpr const char* type_string = "Pulse";
  private:
    virtual void send_value(ThreadId dst) {
      static const double base = 100;
      generator.seed(time(0));
      double variation = 40*distribution(generator);
      if (state == working)
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {base+variation-20}));
      else
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {0}));
    }
};

class BloodPressure : SensorSim<BloodPressure> {
    friend class SensorSim<BloodPressure>;
    using SensorSim::SensorSim;

  public:
    static constexpr const char* type_string = "Bloodpressure";
  private:
    virtual void send_value(ThreadId dst) {
      static const double base_sys = 100;
      static const double base_dia = 80;
      generator.seed(time(0));
      double variation_sys = 20*distribution(generator);
      double variation_dia = 20*distribution(generator);
      if (state == working)
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {base_sys+variation_sys-10, base_dia+variation_dia-10}));
      else
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {0,0}));
    }
};

class Breathing : SensorSim<Breathing> {
    friend class SensorSim<Breathing>;
    using SensorSim::SensorSim;

  public:
    static constexpr const char* type_string = "Breathing";
  private:
    virtual void send_value(ThreadId dst) {
      static const double base = 90;
      generator.seed(time(0));
      double variation = 30*distribution(generator);
      if (state == working)
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {base+variation-15}));
      else
        p_run->send_message(dst, msg_value(p_run->current_thread_id(), {0}));
    }
};


#endif /* SIMULATION_H_ */
