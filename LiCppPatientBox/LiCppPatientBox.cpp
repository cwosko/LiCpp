//============================================================================
// Name        : LiC++Test2.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "LiC++.h"
#include "Common.h"
#include "Simulation.h"

#include <stdio.h>

#include <sstream>
#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;
using namespace LiCpp;

typedef enum eAliases {
  DataCollector
} Aliases;

/* Chaos Monkey
   Chaos monkey to simulate random spontaneous process crashes
   Checks the list of active threads for killing random ones after a random amount of time */
class ChaosMonkey {
  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run, std::vector<ThreadId> spare_list) {
      ThreadId my_id = p_run->spawn_messaging<ChaosMonkey>(ChaosMonkey::msg_handler, ChaosMonkey(p_run, spare_list));
      std::cout << "Started ChaosMonkey with Pid=" << my_id << "\n";
      return my_id;
    }
  private:
    ChaosMonkey(pLiCppRuntime p_run_, std::vector<ThreadId> spare_list_) : p_run(p_run_), next_activation(), spare_list(spare_list_) { }

    static void msg_handler(pMsgBox incoming, std::shared_ptr<ChaosMonkey> me) {
      static const double life_expectancy_ms = 500.0;
      static const auto initial_activation = std::chrono::time_point<std::chrono::system_clock>();

      auto now = std::chrono::system_clock::now();

      if ((me->next_activation == initial_activation) || (me->next_activation < now)) {
        std::default_random_engine generator;
        generator.seed(time(0));
        std::uniform_real_distribution<double> distribution(0.0,1.0);

        if (me->next_activation != initial_activation) {

          /* Load and filter thread list ... */
          auto thread_list = me->p_run->list_threads();
          
          thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
          [&](ThreadId id) {
            if (id == me->p_run->current_thread_id())
              return true;
            return (std::find(me->spare_list.begin(), me->spare_list.end(), id) != me->spare_list.end());
          } ), thread_list.end());

          /* uniform distribution means factor 2, divided by the number of processes in order to
             have a constant life expectancy for each process, albeit with a non-constant variance
             after random:uniform(round(LifeExpectancy*2.0/Length)) */
          ThreadId random_thread = thread_list.at(ceil(thread_list.size() * distribution(generator))-1);

          std::stringstream txt;
          txt << "---Chaos Monkey plunges a process (Pid=" << random_thread << ") into chaos.---\n";
          std::cout << txt.str();
          me->p_run->kill_thread(random_thread);
        }
        me->next_activation = std::chrono::system_clock::now() + std::chrono::milliseconds((int)(life_expectancy_ms*2.0*distribution(generator)));
      }
    }

    pLiCppRuntime p_run;
    std::chrono::time_point<std::chrono::system_clock> next_activation;
    std::vector<ThreadId> spare_list;
};

/* Patient Data Collector
   receives sensor data in order to forward it to the hospital */
class PatientDataCollector {
  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run) {
      ThreadId my_id = p_run->spawn_messaging<PatientDataCollector>(PatientDataCollector::msg_handler, PatientDataCollector(p_run), DataCollector);
      std::cout << "Started data_collector with Pid=" << my_id << "\n";
      return my_id;
    }
  private:
    PatientDataCollector(pLiCppRuntime p_run_) : p_run(p_run_) { }

    static void msg_handler(pMsgBox incoming, std::shared_ptr<PatientDataCollector> me) {
      incoming->wait()
      .handle<msg_value>( [&](msg_value const& msg) {
        std::stringstream txt;
        txt << "Collector has received data update: ID=" << msg.src << ", Value=" << msg.value.at(0) << "\n";
        std::cout << txt.str();
        /*TODO: Forward data to hospital ... */
      } );
    }

    pLiCppRuntime p_run;
};


/* Sensor Driver
   Initiates the communication with the sensor, queries the sensor value,
   and evaluates this value before communicating it up. */
template <typename Driver>
class SensorDriver {
  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run, ThreadId sensor_id) {
      ThreadId my_id = p_run->spawn_messaging<Driver>(Driver::msg_handler, Driver(p_run, sensor_id));
      std::cout << "Started sensorDriver. (type=" << Driver::sensor_type::type_string << ", ID=" << sensor_id << ", PID=" << my_id << ").\n";
      return my_id;
    }

    explicit SensorDriver(pLiCppRuntime p_run_, ThreadId sensor_id_) :
      p_run(p_run_), sensor_id(sensor_id_), last_request(std::chrono::system_clock::now()), response_timeout() {}

  private:
    struct msg_100ms_tick { };

    static void msg_handler(pMsgBox incoming, std::shared_ptr<Driver> me) {

      static const auto max_response_delay_ms = std::chrono::milliseconds(3000);
      static const auto no_timeout = std::chrono::time_point<std::chrono::system_clock>();

      incoming->check()
      .template handle<msg_value>( [&](msg_value const& msg) {
        std::stringstream txt;
        me->response_timeout = no_timeout;
        if (me->is_valid(msg.value)) {
          txt << "Sensor driver (type=" << Driver::sensor_type::type_string
              << ") sending result=" << msg.value.at(0)
              << " to data_collector.\n";
          std::cout << txt.str();
          me->p_run->send_to_alias(DataCollector, msg);
        } else {
          txt << "Value validation has failed on sensor (type=" << Driver::sensor_type::type_string
              << ", ID=" << msg.src
              << ", value=" << msg.value.at(0)
              << ").\nDriver is shutting down.\n";
          std::cout << txt.str();
          throw std::runtime_error("Sensor invalid.");
        }

      } )
      .template handle<msg_crash>( [&](msg_crash const& msg) {
        std::cout << "Connected Supervisor died. Driver is shutting down.\n";
        throw std::runtime_error("Supervisor crash.");
      } );

      auto now = std::chrono::system_clock::now();
      if (me->response_timeout == no_timeout) { /* not waiting for response */
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - me->last_request).count() > Driver::request_interval_ms) {
          me->p_run->send_message(me->sensor_id, msg_query(me->p_run->current_thread_id()));
          me->last_request = now;
          me->response_timeout = now + max_response_delay_ms;
        }
      } else { /* waiting for response */
        if	(now > me->response_timeout) {
          std::cout << "Sensor driver (type=" << Driver::sensor_type::type_string << ") exiting upon timeout!\n";
          throw std::runtime_error("Sensor timeout.");
        }
      }
    };

    virtual bool is_valid(std::vector<double> value) = 0;
    pLiCppRuntime p_run;
    ThreadId sensor_id;
    std::chrono::time_point<std::chrono::system_clock> last_request;
    std::chrono::time_point<std::chrono::system_clock> response_timeout;
};

class TemperatureSensor : SensorDriver<TemperatureSensor> {
    friend class SensorDriver<TemperatureSensor>;
    using SensorDriver::SensorDriver;

  public:
    static const int request_interval_ms = 5000;
    typedef Temperature sensor_type;

  private:
    virtual bool is_valid(std::vector<double> value) {
      return (value.at(0)>=35.5 && value.at(0)<43.0);
    }
};

class PulseSensor : SensorDriver<PulseSensor> {
    friend class SensorDriver<PulseSensor>;
    using SensorDriver::SensorDriver;

  public:
    static const int request_interval_ms = 1600;
    typedef Pulse sensor_type;

  private:
    virtual bool is_valid(std::vector<double> value) {
      return (value.at(0)>=20.0 && value.at(0)<300.0);
    }
};

class BloodPressureSensor : SensorDriver<BloodPressureSensor> {
    friend class SensorDriver<BloodPressureSensor>;
    using SensorDriver::SensorDriver;

  public:
    static const int request_interval_ms = 4200;
    typedef BloodPressure sensor_type;

  private:
    virtual bool is_valid(std::vector<double> value) {
      return (value.at(0)>=50.0 && value.at(0)<250.0 && value.at(1)>=20.0 && value.at(1)<200.0 && value.at(0)>value.at(1));
    }
};

class BreathingSensor : SensorDriver<BreathingSensor> {
    friend class SensorDriver<BreathingSensor>;
    using SensorDriver::SensorDriver;

  public:
    static const int request_interval_ms = 2300;
    typedef Breathing sensor_type;

  private:
    virtual bool is_valid(std::vector<double> value) {
      return (value.at(0)>=5.0 && value.at(0)<100.0);
    }
};


typedef std::vector<ThreadId>SensorList;

template <typename SensorType>
SensorList startSensorSimulations(pLiCppRuntime p_run)
{
  SensorList initial_list;
  for (int i=0; i<SensorType::number_of_sensors; ++i)
    initial_list.push_back(SensorSim<typename SensorType::driver_type::sensor_type>::CreateAndRun(p_run));
  return initial_list;
}

/* Sensor Type Supervisor
   Sensor type supervisor comes in 4 variants, administrates sensor communication processes
   keeps sensor ID lists and keeps track of last used ID per type */
template <typename SensorType>
class SensorTypeSuperVisor {

  public:
    static ThreadId CreateRunAndLink(pLiCppRuntime p_run, SensorList initial_list) {

      ThreadId my_id = p_run->spawn_messaging<SensorType>(SensorType::msg_handler, SensorType(p_run, initial_list));
      std::cout << "Started sensorType. (type=" << SensorType::driver_type::sensor_type::type_string << ", PID=" << my_id << ")\n";

      ThreadId driver_id = SensorDriver<typename SensorType::driver_type>::CreateAndRun(p_run, initial_list.at(0));

      p_run->link_thread_to_supervisor(initial_list.at(0), driver_id);
      p_run->link_thread_to_supervisor(driver_id, my_id);
      p_run->link_thread_to_supervisor(my_id, driver_id);

      return my_id;
    }

    explicit SensorTypeSuperVisor(pLiCppRuntime p_run_, SensorList sensor_list_) : p_run(p_run_), sensor_list(sensor_list_), sensor_index(0) { }

  private:
    static void msg_handler(pMsgBox incoming, std::shared_ptr<SensorType> me) {
      incoming->wait()
      .template handle<msg_crash>( [&](msg_crash const& msg) {

        ++me->sensor_index;
        if (me->sensor_index >= SensorType::number_of_sensors)
          me->sensor_index = 0;

        ThreadId sensor_id = me->sensor_list.at(me->sensor_index);
        ThreadId driver_id = SensorDriver<typename SensorType::driver_type>::CreateAndRun(me->p_run, sensor_id);
        me->p_run->link_thread_to_supervisor(sensor_id, driver_id);
        me->p_run->link_thread_to_supervisor(driver_id, me->p_run->current_thread_id());

      } );
    };

    pLiCppRuntime p_run;
    SensorList sensor_list;
    int sensor_index; /* Iterator gets invalid, using index instead*/
};

struct TemperatureSensors : SensorTypeSuperVisor<TemperatureSensors> {
  friend class SensorTypeSuperVisor<TemperatureSensors>;
  using SensorTypeSuperVisor::SensorTypeSuperVisor;

  typedef TemperatureSensor driver_type;
  static const int number_of_sensors = 2;
};

struct PulseSensors : SensorTypeSuperVisor<PulseSensors> {
  friend class SensorTypeSuperVisor<PulseSensors>;
  using SensorTypeSuperVisor::SensorTypeSuperVisor;

  typedef PulseSensor driver_type;
  static const int number_of_sensors = 2;
};

struct BloodPressureSensors : SensorTypeSuperVisor<BloodPressureSensors> {
  friend class SensorTypeSuperVisor<BloodPressureSensors>;
  using SensorTypeSuperVisor::SensorTypeSuperVisor;

  typedef BloodPressureSensor driver_type;
  static const int number_of_sensors = 2;
};

struct BreathingSensors : SensorTypeSuperVisor<BreathingSensors> {
  friend class SensorTypeSuperVisor<BreathingSensors>;
  using SensorTypeSuperVisor::SensorTypeSuperVisor;

  typedef BreathingSensor driver_type;
  static const int number_of_sensors = 2;
};

/* Home Unit
   Central supervisor to create the sensor types */
class HomeUnit {

    struct Child {
      ThreadId id;
      std::function<ThreadId()> create_new;
    };
    typedef std::vector<Child> Children;
    typedef std::shared_ptr<Children> pChildren;

  public:
    static ThreadId CreateAndRun(pLiCppRuntime p_run, SensorList temp_sensors, SensorList pulse_sensors, SensorList blood_sensors, SensorList breath_sensors) {

      pChildren p_children = make_shared<Children>(Children {
        {
          ThreadId(),[=]() {
            return PatientDataCollector::CreateAndRun(p_run);
          }
        },
        {
          ThreadId(),[=]() {
            return SensorTypeSuperVisor<TemperatureSensors>::CreateRunAndLink(p_run, temp_sensors);
          }
        },
        {
          ThreadId(),[=]() {
            return SensorTypeSuperVisor<PulseSensors>::CreateRunAndLink(p_run, pulse_sensors);
          }
        },
        {
          ThreadId(),[=]() {
            return SensorTypeSuperVisor<BloodPressureSensors>::CreateRunAndLink(p_run, blood_sensors);
          }
        },
        {
          ThreadId(),[=]() {
            return SensorTypeSuperVisor<BreathingSensors>::CreateRunAndLink(p_run, breath_sensors);
          }
        }
      });

      ThreadId my_id = p_run->spawn_messaging<HomeUnit>(HomeUnit::msg_handler, HomeUnit(p_run, p_children));
      std::cout << "Started homeUnit with PID=" << my_id << "\n";

      for (Child& child : *p_children) {
        child.id = child.create_new();
        p_run->link_thread_to_supervisor(child.id, my_id);
      }

      return my_id;
    }
  private:
    HomeUnit(pLiCppRuntime p_run_, pChildren p_children_) : p_run(p_run_), p_children(p_children_) { }

    static void msg_handler(pMsgBox incoming, std::shared_ptr<HomeUnit> me) {
      incoming->wait()
      .template handle<msg_crash>( [&](msg_crash const& msg) {
        for (Child& child : *(me->p_children)) {
          if (msg.id == child.id) {
            std::cout << "Home Unit child (Pid=" << msg.id << ") has died. Restarting ...\n";
            child.id = child.create_new();
            me->p_run->link_thread_to_supervisor(child.id, me->p_run->current_thread_id());
          }
        }
      });
    }

    pLiCppRuntime p_run;
    pChildren p_children;
};

int main()
{
  pLiCppRuntime p_run = make_shared<LiCppRuntime>();

  SensorList temp_sensors   = startSensorSimulations<TemperatureSensors>(p_run);
  SensorList pulse_sensors  = startSensorSimulations<PulseSensors>(p_run);
  SensorList blood_sensors  = startSensorSimulations<BloodPressureSensors>(p_run);
  SensorList breath_sensors = startSensorSimulations<BreathingSensors>(p_run);

  ThreadId home_id = HomeUnit::CreateAndRun(p_run, temp_sensors, pulse_sensors, blood_sensors, breath_sensors);

  std::vector<ThreadId> spare_list {home_id};
  std::copy(temp_sensors.begin()  , temp_sensors.end()  , std::back_inserter(spare_list));
  std::copy(pulse_sensors.begin() , pulse_sensors.end() , std::back_inserter(spare_list));
  std::copy(blood_sensors.begin() , blood_sensors.end() , std::back_inserter(spare_list));
  std::copy(breath_sensors.begin(), breath_sensors.end(), std::back_inserter(spare_list));

  ThreadId chaos_monkey_id = ChaosMonkey::CreateAndRun(p_run, spare_list);

  bool quit_pressed=false;
  std::thread char_thread([&]() {
    while (!quit_pressed) {
      std::string in_data;
      std::cin >> in_data;
      if (in_data.empty())
        continue;
      if (in_data=="q")
        quit_pressed=true;
      if (in_data=="t")
      {
        auto thread_list = p_run->list_threads();
        for (auto const& id : thread_list) std::cout << id << ' ';
        std::cout << std::endl;
      }
      if (in_data.front() == 'k')
      {
        std::istringstream  ss(in_data);
        int to_kill;
        char tmp;
        ss >> tmp >> to_kill;
        std::cout << "Killing Thread: " << to_kill << std::endl;
        p_run->kill_thread(static_cast<ThreadId>(to_kill));
      }
    }
  });

  while(!quit_pressed) {
    p_run->exec();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  char_thread.join();
  return 0;
}
