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

#include "ConditionalFactory.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"
#include "MockConditional.h"

using arlcore::UuidFactory;
using arlcore::io::LocalReaderSender;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;
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

class ConditionalFactoryTest : public ::testing::Test {
 protected:

  static void SetUpTestSuite() {
    globalPoseReader_ = std::make_shared<LocalReaderSender<GlobalPoseReportType>>();
    speedStatusReader_ = std::make_shared<LocalReaderSender<SpeedReportType>>();
    velocityReportReader_ = std::make_shared<LocalReaderSender<VelocityReportType>>();
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
    factory_ = std::make_shared<arlcore::umaa::conditional::ConditionalFactory>(io_);
  }

  void TearDown() override {
    factory_.reset();
    globalPoseReader_->clear();
    speedStatusReader_->clear();
    velocityReportReader_->clear();
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

  static std::shared_ptr<arlcore::umaa::conditional::ConditionalFactory> factory_;
  static std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> io_;
  static std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> globalPoseReader_;
  static std::shared_ptr<LocalReaderSender<SpeedReportType>> speedStatusReader_;
  static std::shared_ptr<LocalReaderSender<VelocityReportType>> velocityReportReader_;
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

std::shared_ptr<arlcore::umaa::conditional::ConditionalFactory> ConditionalFactoryTest::factory_;
std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> ConditionalFactoryTest::io_;
std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> ConditionalFactoryTest::globalPoseReader_;
std::shared_ptr<LocalReaderSender<SpeedReportType>> ConditionalFactoryTest::speedStatusReader_;
std::shared_ptr<LocalReaderSender<VelocityReportType>> ConditionalFactoryTest::velocityReportReader_;
std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> ConditionalFactoryTest::constraintViolatedConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthConditionalType>> ConditionalFactoryTest::depthConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> ConditionalFactoryTest::depthRateConditionalReader_;
std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> ConditionalFactoryTest::emitterPresetConditionalReader_;
std::shared_ptr<LocalReaderSender<ExpConditionalType>> ConditionalFactoryTest::expConditionalReader_;
std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> ConditionalFactoryTest::headingSectorConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> ConditionalFactoryTest::logicalANDConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> ConditionalFactoryTest::logicalNOTConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> ConditionalFactoryTest::logicalORConditionalReader_;
std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> ConditionalFactoryTest::missionStateConditionalReader_;
std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> ConditionalFactoryTest::objectiveStateConditionalReader_;
std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> ConditionalFactoryTest::pitchRateConditionalReader_;
std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> ConditionalFactoryTest::relativeSpeedConditionalReader_;
std::shared_ptr<LocalReaderSender<RollRateConditionalType>> ConditionalFactoryTest::rollRateConditionalReader_;
std::shared_ptr<LocalReaderSender<SpeedConditionalType>> ConditionalFactoryTest::speedConditionalReader_;
std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> ConditionalFactoryTest::taskStateConditionalReader_;
std::shared_ptr<LocalReaderSender<TimeConditionalType>> ConditionalFactoryTest::timeConditionalReader_;
std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> ConditionalFactoryTest::waterZoneConditionalReader_;
std::shared_ptr<LocalReaderSender<YawRateConditionalType>> ConditionalFactoryTest::yawRateConditionalReader_;

TEST_F(ConditionalFactoryTest, testHasDependencies) {
  ConditionalType conditional;

  // Conditional types with dependencies
  conditional.specializationTopic(UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic);
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasDependencies(conditional));

  conditional.specializationTopic(UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasDependencies(conditional));

  conditional.specializationTopic(UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasDependencies(conditional));

  conditional.specializationTopic(UMAA::MM::Conditional::LogicalORConditionalTypeTopic);
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasDependencies(conditional));

  // All others should evaluate to false
  conditional.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  EXPECT_FALSE(arlcore::umaa::conditional::ConditionalFactory::hasDependencies(conditional));
}

TEST_F(ConditionalFactoryTest, testCircularDependencyDetection) {
  auto map = std::make_shared<std::map<UMAA::Common::Measurement::NumericGUID, std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>>>();

  // Generic conditionals to use as dependencies
  ConditionalType cBase1(UuidFactory::getInstance().generateGuid(), "c1", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), "Test");
  ConditionalType cBase2(UuidFactory::getInstance().generateGuid(), "c2", UuidFactory::getInstance().generateGuid(), DateTime(1, 0), "Test");
  ConditionalType cBase3(UuidFactory::getInstance().generateGuid(), "c3", UuidFactory::getInstance().generateGuid(), DateTime(2, 0), "Test");
  auto c1 = std::make_shared<arlcore::test::MockConditional>(cBase1, true);
  auto c2 = std::make_shared<arlcore::test::MockConditional>(cBase2, true);
  auto c3 = std::make_shared<arlcore::test::MockConditional>(cBase3, false);
  map->insert_or_assign(c1->getConditionalId(), c1);
  map->insert_or_assign(c2->getConditionalId(), c2);
  map->insert_or_assign(c3->getConditionalId(), c3);
  
  // a1: AND of c1 and c2
  ConditionalType a1Base(UuidFactory::getInstance().generateGuid(), "a1", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalANDConditionalType a1Specialized(cBase1.conditionalID(), cBase2.conditionalID(), a1Base.specializationTimestamp(), a1Base.specializationID());
  auto a1Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalANDConditional, arlcore::umaa::conditional::LogicalANDConditionalType>(a1Base, a1Specialized, a1Base.specializationTopic());
  ASSERT_TRUE(a1Opt.has_value());
  auto a1Conditional = a1Opt.value();
  map->insert_or_assign(a1Conditional->getConditionalId(), a1Conditional);

  // o1: OR of a1 and c3
  ConditionalType o1Base(UuidFactory::getInstance().generateGuid(), "o1", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalORConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalORConditionalType o1Specialized(a1Base.conditionalID(), cBase3.conditionalID(), o1Base.specializationTimestamp(), o1Base.specializationID());
  auto o1Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalORConditional, arlcore::umaa::conditional::LogicalORConditionalType>(o1Base, o1Specialized, o1Base.specializationTopic());
  ASSERT_TRUE(o1Opt.has_value());
  auto o1Conditional = o1Opt.value();
  map->insert_or_assign(o1Conditional->getConditionalId(), o1Conditional);

  // a2: AND of a1 and o1
  ConditionalType a2(UuidFactory::getInstance().generateGuid(), "a2", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalANDConditionalType a2Specialized(a1Base.conditionalID(), o1Base.conditionalID(), a2.specializationTimestamp(), a2.specializationID());
  auto a2Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalANDConditional, arlcore::umaa::conditional::LogicalANDConditionalType>(a2, a2Specialized, a2.specializationTopic());
  ASSERT_TRUE(a2Opt.has_value());
  auto a2Conditional = a2Opt.value();
  map->insert_or_assign(a2Conditional->getConditionalId(), a2Conditional);

  // n1: NOT of n1 (Circular Dependency)
  ConditionalType n1(UuidFactory::getInstance().generateGuid(), "n1", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalNOTConditionalType n1Specialized(n1.conditionalID(), n1.specializationTimestamp(), n1.specializationID());
  auto n1Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalNOTConditional, arlcore::umaa::conditional::LogicalNOTConditionalType>(n1, n1Specialized, n1.specializationTopic());
  ASSERT_TRUE(n1Opt.has_value());
  auto n1Conditional = n1Opt.value();
  map->insert_or_assign(n1Conditional->getConditionalId(), n1Conditional);

  // n2: NOT of n3 (Circular Dependency)
  auto n3Id = UuidFactory::getInstance().generateGuid();
  ConditionalType n2(UuidFactory::getInstance().generateGuid(), "n2", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalNOTConditionalType n2Specialized(n3Id, n2.specializationTimestamp(), n2.specializationID());
  auto n2Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalNOTConditional, arlcore::umaa::conditional::LogicalNOTConditionalType>(n2, n2Specialized, n2.specializationTopic());
  ASSERT_TRUE(n2Opt.has_value());
  auto n2Conditional = n2Opt.value();
  map->insert_or_assign(n2Conditional->getConditionalId(), n2Conditional);

  // n3: NOT of n2 (Circular Dependency)
  ConditionalType n3(n3Id, "n3", UuidFactory::getInstance().generateGuid(), DateTime(0, 0), UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  UMAA::MM::Conditional::LogicalNOTConditionalType n3Specialized(n2.conditionalID(), n3.specializationTimestamp(), n3.specializationID());
  auto n3Opt = factory_->createConditional<arlcore::umaa::conditional::LogicalNOTConditional, arlcore::umaa::conditional::LogicalNOTConditionalType>(n3, n3Specialized, n3.specializationTopic());
  ASSERT_TRUE(n3Opt.has_value());
  auto n3Conditional = n3Opt.value();
  map->insert_or_assign(n3Conditional->getConditionalId(), n3Conditional);

  EXPECT_FALSE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(c1->getConditionalId(), map));
  EXPECT_FALSE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(a1Conditional->getConditionalId(), map));
  EXPECT_FALSE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(o1Conditional->getConditionalId(), map));
  EXPECT_FALSE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(a2Conditional->getConditionalId(), map));
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(n1Conditional->getConditionalId(), map));
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(n2Conditional->getConditionalId(), map));
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalFactory::hasConflict(n3Conditional->getConditionalId(), map));  
}

TEST_F(ConditionalFactoryTest, testCreateConstraintViolatedConditional) {
  ConditionalType conditional;
  ConstraintViolatedConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  constraintViolatedConditionalReader_->send(specialization);

  auto c = factory_->createConstraintViolatedConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateDepthConditional) {
  ConditionalType conditional;
  DepthConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  depthConditionalReader_->send(specialization);

  auto c = factory_->createDepthConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateDepthRateConditional) {
  ConditionalType conditional;
  DepthRateConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::DepthRateConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  depthRateConditionalReader_->send(specialization);

  auto c = factory_->createDepthRateConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateHeadingSectorConditional) {
  ConditionalType conditional;
  HeadingSectorConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  headingSectorConditionalReader_->send(specialization);

  auto c = factory_->createHeadingSectorConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateLogicalANDConditional) {
  ConditionalType conditional;
  LogicalANDConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  logicalANDConditionalReader_->send(specialization);

  auto c = factory_->createLogicalANDConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateLogicalNOTConditional) {
  ConditionalType conditional;
  LogicalNOTConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  logicalNOTConditionalReader_->send(specialization);

  auto c = factory_->createLogicalNOTConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateLogicalORConditional) {
  ConditionalType conditional;
  LogicalORConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::LogicalORConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  logicalORConditionalReader_->send(specialization);

  auto c = factory_->createLogicalORConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreatePitchRateConditional) {
  ConditionalType conditional;
  PitchRateConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::PitchRateConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  pitchRateConditionalReader_->send(specialization);

  auto c = factory_->createPitchRateConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateRelativeSpeedConditional) {
  ConditionalType conditional;
  RelativeSpeedConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  relativeSpeedConditionalReader_->send(specialization);

  auto c = factory_->createRelativeSpeedConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateRollRateConditional) {
  ConditionalType conditional;
  RollRateConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::RollRateConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  rollRateConditionalReader_->send(specialization);

  auto c = factory_->createRollRateConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateSpeedConditional) {
  ConditionalType conditional;
  SpeedConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::SpeedConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  speedConditionalReader_->send(specialization);

  auto c = factory_->createSpeedConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateTimeConditional) {
  ConditionalType conditional;
  TimeConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::TimeConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  timeConditionalReader_->send(specialization);

  auto c = factory_->createTimeConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateWaterZoneConditional) {
  ConditionalType conditional;
  WaterZoneConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::WaterZoneConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  waterZoneConditionalReader_->send(specialization);

  auto c = factory_->createWaterZoneConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateYawRateConditional) {
  ConditionalType conditional;
  YawRateConditionalType specialization;

  auto id = UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.conditionalID(arlcore::NIL_GUID);
  conditional.specializationID(id);
  conditional.specializationTimestamp(sync);
  conditional.specializationTopic(UMAA::MM::Conditional::YawRateConditionalTypeTopic);
  specialization.specializationReferenceID(id);
  specialization.specializationReferenceTimestamp(sync);
  yawRateConditionalReader_->send(specialization);

  auto c = factory_->createYawRateConditional(conditional);
  EXPECT_TRUE(c.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateConditionalsValid) {
  std::vector<ConditionalType> conditionals;
  ConditionalType conditional;
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.specializationTimestamp(sync);

  // Depth Conditional
  auto depthId = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(depthId);
  conditional.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  DepthConditionalType depth;
  depth.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN);
  depth.specializationReferenceID(conditional.specializationID());
  depth.specializationReferenceTimestamp(sync);
  depth.depth(20);
  depthConditionalReader_->send(depth);
  conditionals.push_back(conditional);

  // Constraint Violated Conditional
  auto constraintViolatedId = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(constraintViolatedId);
  conditional.specializationTopic(UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic);
  ConstraintViolatedConditionalType cv;
  cv.specializationReferenceID(conditional.specializationID());
  cv.specializationReferenceTimestamp(sync);
  cv.constraintConditionalID(depthId);
  cv.duration(10);
  constraintViolatedConditionalReader_->send(cv);
  conditionals.push_back(conditional);

  // Depth Rate Conditional
  auto depthRateId = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(depthRateId);
  conditional.specializationTopic(UMAA::MM::Conditional::DepthRateConditionalTypeTopic);
  DepthRateConditionalType depthRate;
  depthRate.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  depthRate.specializationReferenceID(conditional.specializationID());
  depthRate.specializationReferenceTimestamp(sync);
  depthRate.depthRate(20);
  depthRateConditionalReader_->send(depthRate);
  conditionals.push_back(conditional);

  // Logical AND conditional
  auto logicalAndId = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(logicalAndId);
  conditional.specializationTopic(UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
  LogicalANDConditionalType logicalAnd;
  logicalAnd.specializationReferenceID(conditional.specializationID());
  logicalAnd.specializationReferenceTimestamp(sync);
  logicalAnd.conditionalID1(depthId);
  logicalAnd.conditionalID2(depthRateId);
  logicalANDConditionalReader_->send(logicalAnd);
  conditionals.push_back(conditional);

  auto result = factory_->createConditionals(conditionals);
  ASSERT_TRUE(result.has_value());

  GlobalPoseReportType gp;
  gp.depth(40);
  VelocityReportType vr;
  vr.velocity().downSpeed(10);

  // Cast these to specialized types to validate dependencies
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> constraintViolatedConditional;
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> depthConditional;
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> depthRateConditional;
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> logicalAndConditional;
  for (auto result : result.value()) {
    if (result->getConditionalId() == constraintViolatedId) {
      constraintViolatedConditional = result;
    } else if (result->getConditionalId() == depthId) {
      depthConditional = result;
    } else if (result->getConditionalId() == depthRateId) {
      depthRateConditional = result;
    } else if (result->getConditionalId() == logicalAndId) {
      logicalAndConditional = result;
    }
  }

  globalPoseReader_->send(gp);
  velocityReportReader_->send(vr);
  ASSERT_EQ(io_->globalPoseReportConsumer_->cycle(), ReadStatus::SUCCESS);
  ASSERT_EQ(io_->velocityReportConsumer_->cycle(), ReadStatus::SUCCESS);

  // Should evaluate to true if registered as an observer properly
  auto depthResult = depthConditional->evaluateConditional();
  ASSERT_TRUE(depthResult.has_value());
  EXPECT_TRUE(depthResult.value());

  // Should evaluate to false if dependency is linked properly
  auto constraintViolatedResult = constraintViolatedConditional->evaluateConditional();
  ASSERT_TRUE(constraintViolatedResult.has_value());
  EXPECT_FALSE(constraintViolatedResult.value());

  // Should evaluate to true if registered as an observer properly
  auto depthRateResult = depthRateConditional->evaluateConditional();
  ASSERT_TRUE(depthRateResult.has_value());
  EXPECT_TRUE(depthRateResult.value());

  // Should evaluate to true if dependencies are linked properly
  auto logicalAndResult = logicalAndConditional->evaluateConditional();
  ASSERT_TRUE(logicalAndResult.has_value());
  EXPECT_TRUE(logicalAndResult.value());
}

TEST_F(ConditionalFactoryTest, testCreateConditionalsNoSpecialization) {
  std::vector<ConditionalType> conditionals;
  ConditionalType conditional;
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.specializationTimestamp(sync);

  auto id = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(id);
  conditional.specializationTopic(UMAA::MM::Conditional::PitchRateConditionalTypeTopic);
  conditionals.push_back(conditional);

  auto result = factory_->createConditionals(conditionals);
  EXPECT_FALSE(result.has_value());
}

TEST_F(ConditionalFactoryTest, testCreateConditionalsCircularDependency) {
  std::vector<ConditionalType> conditionals;
  ConditionalType conditional;
  UMAA::Common::Measurement::DateTime sync(5, 100);
  conditional.specializationTimestamp(sync);

  auto id = UuidFactory::getInstance().generateGuid();
  conditional.specializationID(UuidFactory::getInstance().generateGuid());
  conditional.conditionalID(id);
  conditional.specializationTopic(UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
  LogicalNOTConditionalType notConditional;
  notConditional.notConditionalID(id);
  notConditional.specializationReferenceID(conditional.specializationID());
  notConditional.specializationReferenceTimestamp(sync);
  logicalNOTConditionalReader_->send(notConditional);
  conditionals.push_back(conditional);

  auto result = factory_->createConditionals(conditionals);
  EXPECT_FALSE(result.has_value());
}