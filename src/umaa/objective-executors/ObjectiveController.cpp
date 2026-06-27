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

#include "ObjectiveController.h"

namespace arlcore::umaa {

std::mutex ObjectiveController::signalReceived_;
std::condition_variable ObjectiveController::cv_;
std::atomic<bool> ObjectiveController::isObjExeRunning_ = ATOMIC_VAR_INIT(false);

ObjectiveController::ObjectiveController(
  const std::string& objectiveTopic,
  std::shared_ptr<services::GlobalPoseReportConsumer> gpRptConsumer,
  std::shared_ptr<services::SpeedReportConsumer> spRptConsumer,
  std::shared_ptr<services::VelocityReportConsumer> velRptConsumer,
  std::shared_ptr<ObjectiveExecutorControlServiceProvider> objExeProvider,
  std::shared_ptr<ObjectiveExecutorStateControlServiceProvider> objExeStateProvider,
  std::shared_ptr<conditional::ActiveConstraintsControlProvider> activeConstraintsProvider,
  std::shared_ptr<conditional::ConditionalReportConsumer> conditionalReportConsumer,
  const int64_t& cycleRateMilliseconds) :
  objectiveTopic_(objectiveTopic),
  gpRptConsumer_(gpRptConsumer),
  spRptConsumer_(spRptConsumer),
  velRptConsumer_(velRptConsumer),
  objExeProvider_(move(objExeProvider)),
  objExeStateProvider_(move(objExeStateProvider)),
  activeConstraintsProvider_(activeConstraintsProvider),
  conditionalReportConsumer_(conditionalReportConsumer),
  cycleRateMilliseconds_(cycleRateMilliseconds) {
    conditionalReportConsumer->registerObserver(activeConstraintsProvider_);
    objExeProvider_->setActiveConstraintsProvider(activeConstraintsProvider_);
}

ObjectiveController::~ObjectiveController() {
}

bool ObjectiveController::cycle() const {
  // Cycle ObjectiveExecutorControlServiceProvider
  if (!objExeProvider_->cycle()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error during cycle of Objective Executor Control Provider, exiting...")
    return false;
  }

  // Cycle ObjectiveExecutorStateControlProvider
  if (!objExeStateProvider_->cycle()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error during cycle of Objective Executor State Control Provider, exiting...")
    return false;
  }

  // Cycle ConditionalReportConsumer
  if (!conditionalReportConsumer_->cycle()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error during cycle of Conditional Report Consumer, exiting...")
    return false;
  }

  // Cycle ActiveConstraintsControlProvider
  if (!activeConstraintsProvider_->cycle()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error during cycle of Active Constraints Control Provider, exiting...")
    return false;
  }

  //! Cycle GlobalPoseReportConsumer
  // logReadStatus("Global Pose Report", gpRptConsumer_->cycle());
//
  // ! Cycle SpeedReportConsumer
  // logReadStatus("Speed Report", spRptConsumer_->cycle());
//
  // ! Cycle VelocityReportConsumer
  // logReadStatus("Velocity Report", velRptConsumer_->cycle());

  bool isCycleSuccessful = true;

  // Check ObjectiveExecutorControlProvider for an active objective, if nullopt then the executor is in an IDLE state
  if (auto activeObjOpt = ObjectiveExecutorControlServiceProvider::getActiveObjective()) {
    std::shared_ptr<ObjectiveBase> activeObj = *activeObjOpt;
    ObjectiveExecutorStateEnumType currentState;
    auto constraints = activeConstraintsProvider_->getConstraintConditionals();
    bool constraintsFailed = false;

    // Execute objective defined logic depending on the current objective state. If the objective state moves,
    // keep executing, otherwise wait till next cycle.
    do {
      currentState = activeObj->getObjectiveState();
      if (isCycleSuccessful = activeObj->onCycle(); !isCycleSuccessful) {
        break;
      }

      switch (currentState) {
        case ObjectiveExecutorStateEnumType::CANCELED:
          isCycleSuccessful = activeObj->onCanceled();
          break;
        case ObjectiveExecutorStateEnumType::CANCELING:
          isCycleSuccessful = activeObj->onCanceling();
          break;
        case ObjectiveExecutorStateEnumType::COMPLETED:
          isCycleSuccessful = activeObj->onCompleted();
          break;
        case ObjectiveExecutorStateEnumType::EXECUTING:
          isCycleSuccessful = activeObj->onExecuting();
          if (constraints.has_value() && !constraints->empty()) {
            constraintsFailed = std::any_of(constraints->begin(), constraints->end(),
              [](std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> c) {
              return !c->evaluateConditional().value_or(true);
            });
          }
          if (constraintsFailed) {
            UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Active constraint evaluated false during execution")
            activeObj->transitionObjectiveStateToFailed(
              UMAA::Common::MaritimeEnumeration::ObjectiveExecutorStateReasonEnumModule::
              ObjectiveExecutorStateReasonEnumType::CANNOT_PERFORM_UNDER_CONSTRAINTS);
          } else if (activeObj->isObjectiveComplete()) {
            activeObj->advanceObjectiveState();
          } else if (auto reason = activeObj->isObjectiveFailed();
              reason != ObjectiveExecutorStateReasonEnumType::SUCCEEDED) {
            activeObj->transitionObjectiveStateToFailed(reason);
          }
          break;
        case ObjectiveExecutorStateEnumType::FAILED:
          isCycleSuccessful = activeObj->onFailed();
          break;
        case ObjectiveExecutorStateEnumType::MODIFYING:
          isCycleSuccessful = activeObj->onModifying();
          break;
        case ObjectiveExecutorStateEnumType::PAUSED:
          isCycleSuccessful = activeObj->onPaused();
          break;
        case ObjectiveExecutorStateEnumType::PAUSING:
          isCycleSuccessful = activeObj->onPausing();
          break;
        case ObjectiveExecutorStateEnumType::QUEUED:
          isCycleSuccessful = activeObj->onQueued();
          break;
        case ObjectiveExecutorStateEnumType::RESUMING:
          isCycleSuccessful = activeObj->onResuming();
          break;
        default:
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unrecognized Objective State Enum Reached")
      }
    } while (currentState != activeObj->getObjectiveState() && isCycleSuccessful);
  }

  if (!isCycleSuccessful) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to execute objective logic")
  }

  return isCycleSuccessful;
}

int32_t ObjectiveController::run() const {
  std::unique_lock lk(signalReceived_);
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Starting Objective Executor...")

  isObjExeRunning_ = true;

  while (isObjExeRunning_) {
    auto start = std::chrono::system_clock::now();

    if (!cycle()) {
      return 1;
    }

    auto end = std::chrono::system_clock::now();

    if (end - start <= cycleRateMilliseconds_) {
      cv_.wait_until(lk, start + cycleRateMilliseconds_, []{ return !isObjExeRunning_; });
    } else {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Main loop failed to keep up with cycle rate...")
    }
  }

  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Exiting Objective Executor...")
  return 0;
}

void ObjectiveController::signalHandler(int signalNumber) {
  std::string signalName = "";
  switch (signalNumber) {
    case SIGINT:
      signalName = "SIGINT";
      break;
    case SIGTERM:
      signalName = "SIGTERM";
      break;
    default:
      signalName = "Unknown Signal";
      break;
  }

  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Objective Executor received signal: " << signalName)
  isObjExeRunning_ = false;
  cv_.notify_all();
}

void ObjectiveController::logReadStatus(const std::string& consumer, const arlcore::io::ReadStatus& status) const {
  switch (status) {
    case ReadStatus::DISPOSED:
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, consumer << " Consumer received disposed instance")
      break;
    case ReadStatus::ERROR:
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, consumer << " Consumer error during cycle")
      break;
    case ReadStatus::INVALID_DATA:
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, consumer << " Consumer received invalid data")
      break;
    case ReadStatus::NO_DATA:
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, consumer << " Consumer received no new data")
      break;
    case ReadStatus::NOT_IMPLEMENTED:
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, consumer << " Consumer tried to use reader with unimplemented functions")
      break;
    case ReadStatus::SUCCESS:
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, consumer << " Consumer received new data")
      break;
  }
}

}  // namespace arlcore::umaa
