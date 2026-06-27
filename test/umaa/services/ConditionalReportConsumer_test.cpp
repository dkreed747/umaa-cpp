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

#include <gtest/gtest.h>

#include "ConditionalReportConsumer.h"
#include "ConditionalFactory.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"
#include "LargeSetWriter.h"

using arlcore::io::LocalReaderSender;

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;
using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
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

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class ConditionalReportConsumerTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    globalPoseReader_ = std::make_shared<LocalReaderSender<GlobalPoseReportType>>();
    speedStatusReader_ = std::make_shared<LocalReaderSender<SpeedReportType>>();
    velocityReportReader_ = std::make_shared<LocalReaderSender<VelocityReportType>>();
    reportReader_ = std::make_shared<LocalReaderSender<ConditionalReportType>>();
    conditionalSetReader_ = std::make_shared<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>>();
    constraintViolatedConditionalReader_ = std::make_shared<LocalReaderSender<ConstraintViolatedConditionalType>>();
    depthConditionalReader_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();
    depthRateConditionalReader_ = std::make_shared<LocalReaderSender<DepthRateConditionalType>>();
    emitterPresetConditionalReader_ = std::make_shared<LocalReaderSender<EmitterPresetConditionalType>>();
    expConditionalReader_ = std::make_shared<LocalReaderSender<ExpConditionalType>>();
    headingSectorConditionalReader_ = std::make_shared<LocalReaderSender<HeadingSectorConditionalType>>();
    logicalANDConditionalReader_ = std::make_shared<LocalReaderSender<LogicalANDConditionalType>>();
    logicalNOTConditionalReader_ = std::make_shared<LocalReaderSender<LogicalNOTConditionalType>>();
    logicalORConditionalReader_ = std::make_shared<LocalReaderSender<LogicalORConditionalType>>();
    missionStateConditionalReader_ = std::make_shared<LocalReaderSender<MissionStateConditionalType>>();
    objectiveStateConditionalReader_ = std::make_shared<LocalReaderSender<ObjectiveStateConditionalType>>();
    pitchRateConditionalReader_ = std::make_shared<LocalReaderSender<PitchRateConditionalType>>();
    relativeSpeedConditionalReader_ = std::make_shared<LocalReaderSender<RelativeSpeedConditionalType>>();
    rollRateConditionalReader_ = std::make_shared<LocalReaderSender<RollRateConditionalType>>();
    speedConditionalReader_ = std::make_shared<LocalReaderSender<SpeedConditionalType>>();
    taskStateConditionalReader_ = std::make_shared<LocalReaderSender<TaskStateConditionalType>>();
    timeConditionalReader_ = std::make_shared<LocalReaderSender<TimeConditionalType>>();
    waterZoneConditionalReader_ = std::make_shared<LocalReaderSender<WaterZoneConditionalType>>();
    yawRateConditionalReader_ = std::make_shared<LocalReaderSender<YawRateConditionalType>>();
    auto gpReportConsumer = std::make_shared<arlcore::umaa::services::GlobalPoseReportConsumer>(globalPoseReader_);
    auto ssReportConsumer = std::make_shared<arlcore::umaa::services::SpeedReportConsumer>(speedStatusReader_);
    auto vsReportConsumer = std::make_shared<arlcore::umaa::services::VelocityReportConsumer>(velocityReportReader_);
    io_ = std::make_shared<arlcore::umaa::ConditionalFactoryIo>(gpReportConsumer, ssReportConsumer, vsReportConsumer, constraintViolatedConditionalReader_, depthConditionalReader_,
        depthRateConditionalReader_, emitterPresetConditionalReader_, expConditionalReader_, headingSectorConditionalReader_, logicalANDConditionalReader_,
        logicalNOTConditionalReader_, logicalORConditionalReader_, missionStateConditionalReader_, objectiveStateConditionalReader_, pitchRateConditionalReader_,
        relativeSpeedConditionalReader_, rollRateConditionalReader_, speedConditionalReader_, taskStateConditionalReader_, timeConditionalReader_,
        waterZoneConditionalReader_, yawRateConditionalReader_);
  }

  void SetUp() override {
    // Code to run before running each TEST_F()
    factory_ = std::make_shared<arlcore::umaa::conditional::ConditionalFactory>(io_);
  }

  void TearDown() override {
    factory_.reset();
    reportReader_->clear();
    conditionalSetReader_->clear();
    constraintViolatedConditionalReader_->clear();
    depthConditionalReader_->clear();
    depthRateConditionalReader_->clear();
    emitterPresetConditionalReader_->clear();
    expConditionalReader_->clear();
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
    taskStateConditionalReader_->clear();
    timeConditionalReader_->clear();
    waterZoneConditionalReader_->clear();
    yawRateConditionalReader_->clear();
  }

  std::shared_ptr<arlcore::umaa::conditional::ConditionalFactory> factory_;

  static std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> io_;
  static std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> globalPoseReader_;
  static std::shared_ptr<LocalReaderSender<SpeedReportType>> speedStatusReader_;
  static std::shared_ptr<LocalReaderSender<VelocityReportType>> velocityReportReader_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportType>> reportReader_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> conditionalSetReader_;
  static std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> constraintViolatedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<DepthConditionalType>> depthConditionalReader_;
  static std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> depthRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> emitterPresetConditionalReader_;
  static std::shared_ptr<LocalReaderSender<ExpConditionalType>> expConditionalReader_;
  static std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> headingSectorConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> logicalANDConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> logicalNOTConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> logicalORConditionalReader_;
  static std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> missionStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> objectiveStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> pitchRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> relativeSpeedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<RollRateConditionalType>> rollRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<SpeedConditionalType>> speedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> taskStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<TimeConditionalType>> timeConditionalReader_;
  static std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> waterZoneConditionalReader_;
  static std::shared_ptr<LocalReaderSender<YawRateConditionalType>> yawRateConditionalReader_;
  
};

std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> ConditionalReportConsumerTest::io_;
std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> ConditionalReportConsumerTest::globalPoseReader_;
std::shared_ptr<LocalReaderSender<SpeedReportType>> ConditionalReportConsumerTest::speedStatusReader_;
std::shared_ptr<LocalReaderSender<VelocityReportType>> ConditionalReportConsumerTest::velocityReportReader_;
std::shared_ptr<LocalReaderSender<ConditionalReportType>> ConditionalReportConsumerTest::reportReader_;
std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> ConditionalReportConsumerTest::conditionalSetReader_;
std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> ConditionalReportConsumerTest::constraintViolatedConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthConditionalType>> ConditionalReportConsumerTest::depthConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> ConditionalReportConsumerTest::depthRateConditionalReader_;
std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> ConditionalReportConsumerTest::emitterPresetConditionalReader_;
std::shared_ptr<LocalReaderSender<ExpConditionalType>> ConditionalReportConsumerTest::expConditionalReader_;
std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> ConditionalReportConsumerTest::headingSectorConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> ConditionalReportConsumerTest::logicalANDConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> ConditionalReportConsumerTest::logicalNOTConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> ConditionalReportConsumerTest::logicalORConditionalReader_;
std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> ConditionalReportConsumerTest::missionStateConditionalReader_;
std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> ConditionalReportConsumerTest::objectiveStateConditionalReader_;
std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> ConditionalReportConsumerTest::pitchRateConditionalReader_;
std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> ConditionalReportConsumerTest::relativeSpeedConditionalReader_;
std::shared_ptr<LocalReaderSender<RollRateConditionalType>> ConditionalReportConsumerTest::rollRateConditionalReader_;
std::shared_ptr<LocalReaderSender<SpeedConditionalType>> ConditionalReportConsumerTest::speedConditionalReader_;
std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> ConditionalReportConsumerTest::taskStateConditionalReader_;
std::shared_ptr<LocalReaderSender<TimeConditionalType>> ConditionalReportConsumerTest::timeConditionalReader_;
std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> ConditionalReportConsumerTest::waterZoneConditionalReader_;
std::shared_ptr<LocalReaderSender<YawRateConditionalType>> ConditionalReportConsumerTest::yawRateConditionalReader_;

TEST_F(ConditionalReportConsumerTest, getConditionals) {
  arlcore::umaa::conditional::ConditionalReportConsumer consumer(reportReader_, conditionalSetReader_, factory_);
  arlcore::NumericGuid id0 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
  arlcore::NumericGuid id1 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
  arlcore::umaa::LargeSetWriter<ConditionalType, ConditionalReportTypeConditionalsSetElement> writer(conditionalSetReader_);
  ConditionalType c0, c1;
  c0.name("C0");
  c1.name("C1");
  c0.conditionalID(id0.getGuid());
  c1.conditionalID(id1.getGuid());
  c0.specializationID(id0.getGuid());
  c1.specializationID(id1.getGuid());
  c0.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  c1.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  c0.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  c1.specializationTopic(UMAA::MM::Conditional::PitchRateConditionalTypeTopic);
  writer.insert(c0);
  writer.insert(c1);

  UMAA::MM::Conditional::DepthConditionalType d0(ConditionalOperatorEnumType::GREATER_THAN, 10, c0.specializationTimestamp(), c0.specializationID());
  UMAA::MM::Conditional::PitchRateConditionalType p0(ConditionalOperatorEnumType::LESS_THAN, 10, c1.specializationTimestamp(), c1.specializationID());

  depthConditionalReader_->send(d0);
  pitchRateConditionalReader_->send(p0);

  io_->DepthCache.update();
  io_->PitchRateCache.update();

  ConditionalReportType report;
  auto metadata = writer.getMetadata();
  report.conditionalsSetMetadata(metadata);
  report.source().id(arlcore::NIL_GUID.getGuid());
  report.timeStamp(metadata.updateElementTimestamp().value());
  
  
  EXPECT_FALSE(consumer.getReport().has_value());
  EXPECT_FALSE(consumer.getConditionals().has_value());
  reportReader_->send(report);
  consumer.cycle();

  ASSERT_TRUE(consumer.getReport().has_value());
  EXPECT_EQ(report, consumer.getReport().value());

  auto conditionals = consumer.getConditionals();
  ASSERT_TRUE(conditionals.has_value());

  for (auto it = conditionals->begin(); it != conditionals->end(); ++it) {
    EXPECT_TRUE((*it)->getSpecializationId() == id0.getGuid() || (*it)->getSpecializationId() == id1.getGuid());
    EXPECT_TRUE((*it)->getConditionalId() == id0.getGuid() || (*it)->getConditionalId() == id1.getGuid());
  }
}
