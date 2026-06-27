//---------------------------------------------------------------------------
// Copyright 2025 Pennsylvania State University
//
// Applied Research Laboratory
// Pennsylvania State University
// P.O. Box 30
// State College, PA 16804-0030
//
// DISTRIBUTION STATEMENT A. Approved for public release.
// Distribution is unlimited.
// This software was developed by the Department of the Navy,
// NAVSEA Unmanned and Small Combatants. It is provided under the terms of
// use found in the LICENSE file at the source code root directory.
//
//---------------------------------------------------------------------------

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_OBJECTIVECONTROLLER_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_OBJECTIVECONTROLLER_H_

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "ActiveConstraintsControlProvider.h"
#include "ConditionalReportConsumer.h"
#include "ObjectiveExecutorConfig.h"
#include "ObjectiveExecutorControlServiceProvider.h"
#include "ObjectiveExecutorStateControlServiceProvider.h"
#include "UmaaUtils.h"
#include "GlobalPoseReportConsumer.h"
#include "SpeedReportConsumer.h"
#include "VelocityReportConsumer.h"

namespace arlcore::umaa {

class ObjectiveController {
 public:
  //! \brief Constructor for an Objective Executor
  //! \param objectiveTopic UMAA topic string of the specialization for the objective executor to register to
  //! \param gpRptConsumer Global pose report consumer
  //! \param spRptConsumer Speed report consumer
  //! \param velRptConsumer Velocity report consumer
  //! \param objExeProvider Objective Executor Control Service Provider pointer
  //! \param objExeStateProvider Objective Executor State Control Service Provider pointer
  //! \param cycleRateMilliseconds The constant cycle rate (milliseconds) at which to run the main loop
  ObjectiveController(
    const std::string& objectiveTopic,
    std::shared_ptr<services::GlobalPoseReportConsumer> gpRptConsumer,
    std::shared_ptr<services::SpeedReportConsumer> spRptConsumer,
    std::shared_ptr<services::VelocityReportConsumer> velRptConsumer,
    std::shared_ptr<ObjectiveExecutorControlServiceProvider> objExeProvider,
    std::shared_ptr<ObjectiveExecutorStateControlServiceProvider> objExeStateProvider,
    std::shared_ptr<conditional::ActiveConstraintsControlProvider> activeConstraintsProvider,
    std::shared_ptr<conditional::ConditionalReportConsumer> conditionalReportConsumer,
    const int64_t& cycleRateMilliseconds = 200);

  // Delete Copy constructor and assignment operators
  ObjectiveController(const ObjectiveController&) = delete;
  ObjectiveController& operator=(const ObjectiveController&) = delete;

  //! \brief Unregister objectives
  ~ObjectiveController();

  //! \brief Code to be called every loop of the run() function
  bool cycle() const;

  //! \brief Run the Objective Executor main loop, calls code in cycle() every cycleRateMilliseconds
  //! \return returns unix error code. 0 for normal status and 1 for errors. Other non-zero numbers can be used, but
  //! containerized applications are recommended to only use 1 since the others are used by the docker engine
  int32_t run() const;

  //! \brief Signal handler to stop service provider gracefully
  //! \param signalNumber
  static void signalHandler(int signalNumber);

  //! \brief Helper function to log the result of a report consumer cycle
  //! \param consumer String name of the consumer
  //! \param status ReadStatus of the cycle
  void logReadStatus(const std::string& consumer, const arlcore::io::ReadStatus& status) const;

 private:
  const std::string objectiveTopic_;
  const std::shared_ptr<services::GlobalPoseReportConsumer> gpRptConsumer_;
  const std::shared_ptr<services::SpeedReportConsumer> spRptConsumer_;
  const std::shared_ptr<services::VelocityReportConsumer> velRptConsumer_;
  const std::shared_ptr<ObjectiveExecutorControlServiceProvider> objExeProvider_;
  const std::shared_ptr<ObjectiveExecutorStateControlServiceProvider> objExeStateProvider_;
  const std::shared_ptr<conditional::ActiveConstraintsControlProvider> activeConstraintsProvider_;
  const std::shared_ptr<conditional::ConditionalReportConsumer> conditionalReportConsumer_;
  const std::chrono::milliseconds cycleRateMilliseconds_;

  static std::mutex signalReceived_;
  static std::condition_variable cv_;
  static std::atomic<bool> isObjExeRunning_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_OBJECTIVECONTROLLER_H_
