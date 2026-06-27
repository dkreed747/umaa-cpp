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

#ifndef TEST_UMAA_CONDITIONALS_UTIL_LOCALCONDITIONALFACTORYIO_H_
#define TEST_UMAA_CONDITIONALS_UTIL_LOCALCONDITIONALFACTORYIO_H_

#include "ConditionalFactoryIo.h"
#include "LocalReaderSender.h"

namespace arlcore::test {

class LocalConditionalFactoryIo {
 public:
  LocalConditionalFactoryIo() :
      globalPoseReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::SA::GlobalPoseStatus::GlobalPoseReportType>>()),
      speedReportReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::SA::SpeedStatus::SpeedReportType>>()),
      velocityReportReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::SA::VelocityStatus::VelocityReportType>>()),
      constraintViolatedReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ConstraintViolatedConditionalType>>()),
      depthConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::DepthConditionalType>>()),
      depthRateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::DepthRateConditionalType>>()),
      emitterPresetConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::EmitterPresetConditionalType>>()),
      expConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ExpConditionalType>>()),
      headingSectorConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::HeadingSectorConditionalType>>()),
      logicalANDConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalANDConditionalType>>()),
      logicalNOTConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalNOTConditionalType>>()),
      logicalORConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalORConditionalType>>()),
      missionStateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::MissionStateConditionalType>>()),
      objectiveStateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ObjectiveStateConditionalType>>()),
      pitchRateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::PitchRateConditionalType>>()),
      relativeSpeedConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::RelativeSpeedConditionalType>>()),
      rollRateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::RollRateConditionalType>>()),
      speedConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::SpeedConditionalType>>()),
      taskStateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::TaskStateConditionalType>>()),
      timeConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::TimeConditionalType>>()),
      waterZoneConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::WaterZoneConditionalType>>()),
      yawRateConditionalReader_(std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::YawRateConditionalType>>()) {
        io_ = std::make_shared<arlcore::umaa::ConditionalFactoryIo>(
          std::make_shared<arlcore::umaa::services::GlobalPoseReportConsumer>(globalPoseReader_),
          std::make_shared<arlcore::umaa::services::SpeedReportConsumer>(speedReportReader_),
          std::make_shared<arlcore::umaa::services::VelocityReportConsumer>(velocityReportReader_),
          constraintViolatedReader_,
          depthConditionalReader_,
          depthRateConditionalReader_,
          emitterPresetConditionalReader_,
          expConditionalReader_,
          headingSectorConditionalReader_,
          logicalANDConditionalReader_,
          logicalNOTConditionalReader_,
          logicalORConditionalReader_,
          missionStateConditionalReader_,
          objectiveStateConditionalReader_,
          pitchRateConditionalReader_,
          relativeSpeedConditionalReader_,
          rollRateConditionalReader_,
          speedConditionalReader_,
          taskStateConditionalReader_,
          timeConditionalReader_,
          waterZoneConditionalReader_,
          yawRateConditionalReader_
        );
      }

  std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> io_;

  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::SA::GlobalPoseStatus::GlobalPoseReportType>> globalPoseReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::SA::SpeedStatus::SpeedReportType>> speedReportReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::SA::VelocityStatus::VelocityReportType>> velocityReportReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ConstraintViolatedConditionalType>> constraintViolatedReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::DepthConditionalType>> depthConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::DepthRateConditionalType>> depthRateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::EmitterPresetConditionalType>> emitterPresetConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ExpConditionalType>> expConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::HeadingSectorConditionalType>> headingSectorConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalANDConditionalType>> logicalANDConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalNOTConditionalType>> logicalNOTConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::LogicalORConditionalType>> logicalORConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::MissionStateConditionalType>> missionStateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::ObjectiveStateConditionalType>> objectiveStateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::PitchRateConditionalType>> pitchRateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::RelativeSpeedConditionalType>> relativeSpeedConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::RollRateConditionalType>> rollRateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::SpeedConditionalType>> speedConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::TaskStateConditionalType>> taskStateConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::TimeConditionalType>> timeConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::WaterZoneConditionalType>> waterZoneConditionalReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::Conditional::YawRateConditionalType>> yawRateConditionalReader_;

  void clearAll() {
    constraintViolatedReader_->clear();
    depthConditionalReader_->clear();
    depthRateConditionalReader_->clear();
    emitterPresetConditionalReader_->clear();
    expConditionalReader_->clear();
    globalPoseReader_->clear();
    headingSectorConditionalReader_->clear();
    logicalANDConditionalReader_->clear();
    logicalNOTConditionalReader_->clear();
    logicalORConditionalReader_->clear();
    missionStateConditionalReader_->clear();
    objectiveStateConditionalReader_->clear();
    pitchRateConditionalReader_->clear();
    relativeSpeedConditionalReader_->clear();
    rollRateConditionalReader_->clear();
    speedConditionalReader_->clear();
    speedReportReader_->clear();
    taskStateConditionalReader_->clear();
    timeConditionalReader_->clear();
    velocityReportReader_->clear();
    waterZoneConditionalReader_->clear();
    yawRateConditionalReader_->clear();
  }
};

}  // namespace arlcore::test
#endif  // TEST_UMAA_CONDITIONALS_UTIL_LOCALCONDITIONALFACTORYIO_H_
