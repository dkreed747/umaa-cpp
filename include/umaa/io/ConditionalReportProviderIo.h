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

#ifndef INCLUDE_UMAA_IO_CONDITIONALREPORTPROVIDERIO_H_
#define INCLUDE_UMAA_IO_CONDITIONALREPORTPROVIDERIO_H_

#include <memory>

#include <UMAA/MM/ConditionalReport/ConditionalReportType.hpp>
#include <UMAA/MM/Conditional/ConstraintViolatedConditionalType.hpp>
#include <UMAA/MM/Conditional/DepthConditionalType.hpp>
#include <UMAA/MM/Conditional/DepthRateConditionalType.hpp>
#include <UMAA/MM/Conditional/EmitterPresetConditionalType.hpp>
#include <UMAA/MM/Conditional/ExpConditionalType.hpp>
#include <UMAA/MM/Conditional/HeadingSectorConditionalType.hpp>
#include <UMAA/MM/Conditional/LogicalANDConditionalType.hpp>
#include <UMAA/MM/Conditional/LogicalNOTConditionalType.hpp>
#include <UMAA/MM/Conditional/LogicalORConditionalType.hpp>
#include <UMAA/MM/Conditional/MissionStateConditionalType.hpp>
#include <UMAA/MM/Conditional/ObjectiveStateConditionalType.hpp>
#include <UMAA/MM/Conditional/PitchRateConditionalType.hpp>
#include <UMAA/MM/Conditional/RelativeSpeedConditionalType.hpp>
#include <UMAA/MM/Conditional/RollRateConditionalType.hpp>
#include <UMAA/MM/Conditional/SpeedConditionalType.hpp>
#include <UMAA/MM/Conditional/TaskStateConditionalType.hpp>
#include <UMAA/MM/Conditional/TimeConditionalType.hpp>
#include <UMAA/MM/Conditional/WaterZoneConditionalType.hpp>
#include <UMAA/MM/Conditional/YawRateConditionalType.hpp>

#include "SenderBase.h"

namespace arlcore::umaa {

using arlcore::io::SenderBase;

using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConstraintViolatedConditionalType;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::DepthRateConditionalType;
using UMAA::MM::Conditional::EmitterPresetConditionalType;
using UMAA::MM::Conditional::ExpConditionalType;
using UMAA::MM::Conditional::HeadingSectorConditionalType;
using UMAA::MM::Conditional::LogicalANDConditionalType;
using UMAA::MM::Conditional::LogicalNOTConditionalType;
using UMAA::MM::Conditional::LogicalORConditionalType;
using UMAA::MM::Conditional::MissionStateConditionalType;
using UMAA::MM::Conditional::ObjectiveStateConditionalType;
using UMAA::MM::Conditional::PitchRateConditionalType;
using UMAA::MM::Conditional::RelativeSpeedConditionalType;
using UMAA::MM::Conditional::RollRateConditionalType;
using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::MM::Conditional::TaskStateConditionalType;
using UMAA::MM::Conditional::TimeConditionalType;
using UMAA::MM::Conditional::WaterZoneConditionalType;
using UMAA::MM::Conditional::YawRateConditionalType;

class ConditionalReportProviderIo {
 public:
  //! \brief Structure holding all writers needed for the Conditional Report Publisher
  //! \param constraintViolatedWriter ConstraintViolatedConditionalTypeTopic
  //! \param depthWriter DepthConditionalTypeTopic
  //! \param depthRateWriter DepthRateConditionalTypeTopic
  //! \param emitterPresetWriter EmitterPresetConditionalTypeTopic
  //! \param expWriter ExpConditionalTypeTopic
  //! \param headingSectorWriter HeadingSectorConditionalTypeTopic
  //! \param logicalANDWriter LogicalANDConditionalTypeTopic
  //! \param logicalNOTWriter LogicalNOTConditionalTypeTopic
  //! \param logicalORWriter LogicalORConditionalTypeTopic
  //! \param missionStateWriter MissionStateConditionalTypeTopic
  //! \param objectiveStateWriter ObjectiveStateConditionalTypeTopic
  //! \param pitchRateWriter PitchRateConditionalTypeTopic
  //! \param relativeSpeedWriter RelativeSpeedConditionalTypeTopic
  //! \param rollRateWriter RollRateConditionalTypeTopic
  //! \param speedWriter SpeedConditionalTypeTopic
  //! \param taskStateWriter TaskStateConditionalTypeTopic
  //! \param timeWriter TimeConditionalTypeTopic
  //! \param waterZoneWriter WaterZoneConditionalTypeTopic
  //! \param yawRateWriter YawRateConditionalTypeTopic
  ConditionalReportProviderIo(
    std::shared_ptr<SenderBase<ConditionalReportType>> reportWriter,
    std::shared_ptr<SenderBase<ConditionalReportTypeConditionalsSetElement>> reportSetElementWriter,
    std::shared_ptr<SenderBase<ConstraintViolatedConditionalType>> constraintViolatedWriter,
    std::shared_ptr<SenderBase<DepthConditionalType>> depthWriter,
    std::shared_ptr<SenderBase<DepthRateConditionalType>> depthRateWriter,
    std::shared_ptr<SenderBase<EmitterPresetConditionalType>> emitterPresetWriter,
    std::shared_ptr<SenderBase<ExpConditionalType>> expWriter,
    std::shared_ptr<SenderBase<HeadingSectorConditionalType>> headingSectorWriter,
    std::shared_ptr<SenderBase<LogicalANDConditionalType>> logicalANDWriter,
    std::shared_ptr<SenderBase<LogicalNOTConditionalType>> logicalNOTWriter,
    std::shared_ptr<SenderBase<LogicalORConditionalType>> logicalORWriter,
    std::shared_ptr<SenderBase<MissionStateConditionalType>> missionStateWriter,
    std::shared_ptr<SenderBase<ObjectiveStateConditionalType>> objectiveStateWriter,
    std::shared_ptr<SenderBase<PitchRateConditionalType>> pitchRateWriter,
    std::shared_ptr<SenderBase<RelativeSpeedConditionalType>> relativeSpeedWriter,
    std::shared_ptr<SenderBase<RollRateConditionalType>> rollRateWriter,
    std::shared_ptr<SenderBase<SpeedConditionalType>> speedWriter,
    std::shared_ptr<SenderBase<TaskStateConditionalType>> taskStateWriter,
    std::shared_ptr<SenderBase<TimeConditionalType>> timeWriter,
    std::shared_ptr<SenderBase<WaterZoneConditionalType>> waterZoneWriter,
    std::shared_ptr<SenderBase<YawRateConditionalType>> yawRateWriter) :
    ReportWriter(reportWriter),
    ReportSetElementWriter(reportSetElementWriter),
    ConstraintViolatedWriter(constraintViolatedWriter),
    DepthWriter(depthWriter),
    DepthRateWriter(depthRateWriter),
    EmitterPresetWriter(emitterPresetWriter),
    ExpWriter(expWriter),
    HeadingSectorWriter(headingSectorWriter),
    LogicalANDWriter(logicalANDWriter),
    LogicalNOTWriter(logicalNOTWriter),
    LogicalORWriter(logicalORWriter),
    MissionStateWriter(missionStateWriter),
    ObjectiveStateWriter(objectiveStateWriter),
    PitchRateWriter(pitchRateWriter),
    RelativeSpeedWriter(relativeSpeedWriter),
    RollRateWriter(rollRateWriter),
    SpeedWriter(speedWriter),
    TaskStateWriter(taskStateWriter),
    TimeWriter(timeWriter),
    WaterZoneWriter(waterZoneWriter),
    YawRateWriter(yawRateWriter) {}

  std::shared_ptr<SenderBase<ConditionalReportType>> ReportWriter;
  std::shared_ptr<SenderBase<ConditionalReportTypeConditionalsSetElement>> ReportSetElementWriter;
  std::shared_ptr<SenderBase<ConstraintViolatedConditionalType>> ConstraintViolatedWriter;
  std::shared_ptr<SenderBase<DepthConditionalType>> DepthWriter;
  std::shared_ptr<SenderBase<DepthRateConditionalType>> DepthRateWriter;
  std::shared_ptr<SenderBase<EmitterPresetConditionalType>> EmitterPresetWriter;
  std::shared_ptr<SenderBase<ExpConditionalType>> ExpWriter;
  std::shared_ptr<SenderBase<HeadingSectorConditionalType>> HeadingSectorWriter;
  std::shared_ptr<SenderBase<LogicalANDConditionalType>> LogicalANDWriter;
  std::shared_ptr<SenderBase<LogicalNOTConditionalType>> LogicalNOTWriter;
  std::shared_ptr<SenderBase<LogicalORConditionalType>> LogicalORWriter;
  std::shared_ptr<SenderBase<MissionStateConditionalType>> MissionStateWriter;
  std::shared_ptr<SenderBase<ObjectiveStateConditionalType>> ObjectiveStateWriter;
  std::shared_ptr<SenderBase<PitchRateConditionalType>> PitchRateWriter;
  std::shared_ptr<SenderBase<RelativeSpeedConditionalType>> RelativeSpeedWriter;
  std::shared_ptr<SenderBase<RollRateConditionalType>> RollRateWriter;
  std::shared_ptr<SenderBase<SpeedConditionalType>> SpeedWriter;
  std::shared_ptr<SenderBase<TaskStateConditionalType>> TaskStateWriter;
  std::shared_ptr<SenderBase<TimeConditionalType>> TimeWriter;
  std::shared_ptr<SenderBase<WaterZoneConditionalType>> WaterZoneWriter;
  std::shared_ptr<SenderBase<YawRateConditionalType>> YawRateWriter;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_IO_CONDITIONALREPORTPROVIDERIO_H_
