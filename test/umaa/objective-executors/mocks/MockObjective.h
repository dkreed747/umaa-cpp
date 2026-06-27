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

#ifndef  TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVE_H_
#define TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVE_H_

#include <memory>

#include "ObjectiveBase.h"

namespace arlcore::test {

class MockObjective : public arlcore::umaa::ObjectiveBase {
 public:
  explicit MockObjective(
    const arlcore::umaa::ObjectiveType& objective,
    std::shared_ptr<arlcore::io::SenderBase<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
      exeStatSender) :
    ObjectiveBase(objective, exeStatSender) {}

  bool onCycle() override { return onCycleReturnFlag;}
  bool onQueued() override { return onQueuedReturnFlag; }
  bool onCanceled() override { return onCanceledReturnFlag; }
  bool onCanceling() override { return onCancelingReturnFlag; }
  bool onCompleted() override { return onCompletedReturnFlag; }
  bool onExecuting() override { return onExecutingReturnFlag; }
  bool onFailed() override { return onFailedReturnFlag; }
  bool onModifying() override { return onModifyingReturnFlag; }
  bool onPaused() override { return onPausedReturnFlag; }
  bool onPausing() override { return onPausingReturnFlag; }
  bool onResuming() override { return onResumingReturnFlag; }

  bool isObjectiveValid() override { return isObjectiveValidFlag; }
  bool isObjectiveComplete() override { return isObjectiveCompleteFlag; }
  arlcore::umaa::ObjectiveExecutorStateReasonEnumType isObjectiveFailed() override {
    return objectiveFailReason;
  }
  arlcore::io::SendStatus sendExecutionStatus() override {
    return sendExecutionStatusReturnEnum;
  }

  std::optional<std::vector<std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>>> getActiveConstraints() {
    return this->activeConstraints_;
  }

  bool onCycleReturnFlag = true;
  bool onQueuedReturnFlag = true;
  bool onCanceledReturnFlag = true;
  bool onCancelingReturnFlag = true;
  bool onCompletedReturnFlag = true;
  bool onExecutingReturnFlag = true;
  bool onFailedReturnFlag = true;
  bool onModifyingReturnFlag = true;
  bool onPausedReturnFlag = true;
  bool onPausingReturnFlag = true;
  bool onResumingReturnFlag = true;

  bool isObjectiveValidFlag = true;
  bool isObjectiveCompleteFlag = true;
  arlcore::umaa::ObjectiveExecutorStateReasonEnumType objectiveFailReason =
    arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED;

  arlcore::io::SendStatus sendExecutionStatusReturnEnum = arlcore::io::SendStatus::NOT_IMPLEMENTED;
};

}  // namespace arlcore::test
#endif  // TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVE_H_
