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

#ifndef INCLUDE_UMAA_IO_CONDITIONALFACTORYIO_H_
#define INCLUDE_UMAA_IO_CONDITIONALFACTORYIO_H_

#include <memory>

#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>
#include <UMAA/SA/SpeedStatus/SpeedReportType.hpp>
#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>
#include <UMAA/MM/ConditionalReport/ConditionalReportType.hpp>
#include "UMAA/MM/Conditional/ConstraintViolatedConditionalType.hpp"
#include "UMAA/MM/Conditional/DepthConditionalType.hpp"
#include "UMAA/MM/Conditional/DepthRateConditionalType.hpp"
#include "UMAA/MM/Conditional/EmitterPresetConditionalType.hpp"
#include "UMAA/MM/Conditional/ExpConditionalType.hpp"
#include "UMAA/MM/Conditional/HeadingSectorConditionalType.hpp"
#include "UMAA/MM/Conditional/LogicalANDConditionalType.hpp"
#include "UMAA/MM/Conditional/LogicalNOTConditionalType.hpp"
#include "UMAA/MM/Conditional/LogicalORConditionalType.hpp"
#include "UMAA/MM/Conditional/MissionStateConditionalType.hpp"
#include "UMAA/MM/Conditional/ObjectiveStateConditionalType.hpp"
#include "UMAA/MM/Conditional/PitchRateConditionalType.hpp"
#include "UMAA/MM/Conditional/RelativeSpeedConditionalType.hpp"
#include "UMAA/MM/Conditional/RollRateConditionalType.hpp"
#include "UMAA/MM/Conditional/SpeedConditionalType.hpp"
#include "UMAA/MM/Conditional/TaskStateConditionalType.hpp"
#include "UMAA/MM/Conditional/TimeConditionalType.hpp"
#include "UMAA/MM/Conditional/WaterZoneConditionalType.hpp"
#include "UMAA/MM/Conditional/YawRateConditionalType.hpp"

#include "ReaderBase.h"
#include "SenderBase.h"
#include "SpecializationCache.h"
#include "ObservableReader.h"
#include "GlobalPoseReportConsumer.h"
#include "SpeedReportConsumer.h"
#include "VelocityReportConsumer.h"

namespace arlcore::umaa {

using arlcore::io::ReaderBase;

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;
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

class ConditionalFactoryIo {
 public:
  //! \brief Structure holding all readers needed for the ConditionalFactory
  //! \param globalPoseReader GlobalPoseReportTypeTopic
  //! \param speedStatusReader SpeedReportTypeTopic
  //! \param velocityStatusReader VelocityReportTypeTopic
  //! \param constraintViolatedReader ConstraintViolatedConditionalTypeTopic
  //! \param depthReader DepthConditionalTypeTopic
  //! \param depthRateReader DepthRateConditionalTypeTopic
  //! \param emitterPresetReader EmitterPresetConditionalTypeTopic
  //! \param expReader ExpConditionalTypeTopic
  //! \param headingSectorReader HeadingSectorConditionalTypeTopic
  //! \param logicalANDReader LogicalANDConditionalTypeTopic
  //! \param logicalNOTReader LogicalNOTConditionalTypeTopic
  //! \param logicalORReader LogicalORConditionalTypeTopic
  //! \param missionStateReader MissionStateConditionalTypeTopic
  //! \param objectiveStateReader ObjectiveStateConditionalTypeTopic
  //! \param pitchRateReader PitchRateConditionalTypeTopic
  //! \param relativeSpeedReader RelativeSpeedConditionalTypeTopic
  //! \param rollRateReader RollRateConditionalTypeTopic
  //! \param speedReader SpeedConditionalTypeTopic
  //! \param taskStateReader TaskStateConditionalTypeTopic
  //! \param timeReader TimeConditionalTypeTopic
  //! \param waterZoneReader WaterZoneConditionalTypeTopic
  //! \param yawRateReader YawRateConditionalTypeTopic
  ConditionalFactoryIo(
    std::shared_ptr<arlcore::umaa::services::GlobalPoseReportConsumer> globalPoseReportConsumer,
    std::shared_ptr<arlcore::umaa::services::SpeedReportConsumer> speedReportConsumer,
    std::shared_ptr<arlcore::umaa::services::VelocityReportConsumer> velocityReportConsumer,
    std::shared_ptr<ReaderBase<ConstraintViolatedConditionalType>> constraintViolatedReader,
    std::shared_ptr<ReaderBase<DepthConditionalType>> depthReader,
    std::shared_ptr<ReaderBase<DepthRateConditionalType>> depthRateReader,
    std::shared_ptr<ReaderBase<EmitterPresetConditionalType>> emitterPresetReader,
    std::shared_ptr<ReaderBase<ExpConditionalType>> expReader,
    std::shared_ptr<ReaderBase<HeadingSectorConditionalType>> headingSectorReader,
    std::shared_ptr<ReaderBase<LogicalANDConditionalType>> logicalANDReader,
    std::shared_ptr<ReaderBase<LogicalNOTConditionalType>> logicalNOTReader,
    std::shared_ptr<ReaderBase<LogicalORConditionalType>> logicalORReader,
    std::shared_ptr<ReaderBase<MissionStateConditionalType>> missionStateReader,
    std::shared_ptr<ReaderBase<ObjectiveStateConditionalType>> objectiveStateReader,
    std::shared_ptr<ReaderBase<PitchRateConditionalType>> pitchRateReader,
    std::shared_ptr<ReaderBase<RelativeSpeedConditionalType>> relativeSpeedReader,
    std::shared_ptr<ReaderBase<RollRateConditionalType>> rollRateReader,
    std::shared_ptr<ReaderBase<SpeedConditionalType>> speedReader,
    std::shared_ptr<ReaderBase<TaskStateConditionalType>> taskStateReader,
    std::shared_ptr<ReaderBase<TimeConditionalType>> timeReader,
    std::shared_ptr<ReaderBase<WaterZoneConditionalType>> waterZoneReader,
    std::shared_ptr<ReaderBase<YawRateConditionalType>> yawRateReader) :
    globalPoseReportConsumer_(globalPoseReportConsumer),
    speedReportConsumer_(speedReportConsumer),
    velocityReportConsumer_(velocityReportConsumer),
    ConstraintViolatedCache(constraintViolatedReader),
    DepthCache(depthReader),
    DepthRateCache(depthRateReader),
    EmitterPresetCache(emitterPresetReader),
    ExpCache(expReader),
    HeadingSectorCache(headingSectorReader),
    LogicalANDCache(logicalANDReader),
    LogicalNOTCache(logicalNOTReader),
    LogicalORCache(logicalORReader),
    MissionStateCache(missionStateReader),
    ObjectiveStateCache(objectiveStateReader),
    PitchRateCache(pitchRateReader),
    RelativeSpeedCache(relativeSpeedReader),
    RollRateCache(rollRateReader),
    SpeedCache(speedReader),
    TaskStateCache(taskStateReader),
    TimeCache(timeReader),
    WaterZoneCache(waterZoneReader),
    YawRateCache(yawRateReader) {}

  std::shared_ptr<arlcore::umaa::services::GlobalPoseReportConsumer> globalPoseReportConsumer_;
  std::shared_ptr<arlcore::umaa::services::SpeedReportConsumer> speedReportConsumer_;
  std::shared_ptr<arlcore::umaa::services::VelocityReportConsumer> velocityReportConsumer_;
  SpecializationCache<ConstraintViolatedConditionalType> ConstraintViolatedCache;
  SpecializationCache<DepthConditionalType> DepthCache;
  SpecializationCache<DepthRateConditionalType> DepthRateCache;
  SpecializationCache<EmitterPresetConditionalType> EmitterPresetCache;
  SpecializationCache<ExpConditionalType> ExpCache;
  SpecializationCache<HeadingSectorConditionalType> HeadingSectorCache;
  SpecializationCache<LogicalANDConditionalType> LogicalANDCache;
  SpecializationCache<LogicalNOTConditionalType> LogicalNOTCache;
  SpecializationCache<LogicalORConditionalType> LogicalORCache;
  SpecializationCache<MissionStateConditionalType> MissionStateCache;
  SpecializationCache<ObjectiveStateConditionalType> ObjectiveStateCache;
  SpecializationCache<PitchRateConditionalType> PitchRateCache;
  SpecializationCache<RelativeSpeedConditionalType> RelativeSpeedCache;
  SpecializationCache<RollRateConditionalType> RollRateCache;
  SpecializationCache<SpeedConditionalType> SpeedCache;
  SpecializationCache<TaskStateConditionalType> TaskStateCache;
  SpecializationCache<TimeConditionalType> TimeCache;
  SpecializationCache<WaterZoneConditionalType> WaterZoneCache;
  SpecializationCache<YawRateConditionalType> YawRateCache;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_IO_CONDITIONALFACTORYIO_H_
