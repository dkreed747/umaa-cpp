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

#include "ConditionalReportProvider.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"
#include "LargeSetReader.h"

using arlcore::io::LocalReaderSender;

using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::DepthConditionalTypeTopic;
using UMAA::MM::Conditional::DepthRateConditionalType;
using UMAA::MM::Conditional::DepthRateConditionalTypeTopic;
using UMAA::MM::Conditional::EmitterPresetConditionalType;
using UMAA::MM::Conditional::EmitterPresetConditionalTypeTopic;
using UMAA::MM::Conditional::ExpConditionalType;
using UMAA::MM::Conditional::ExpConditionalTypeTopic;
using UMAA::MM::Conditional::HeadingSectorConditionalType;
using UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalANDConditionalType;
using UMAA::MM::Conditional::LogicalANDConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalNOTConditionalType;
using UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalORConditionalType;
using UMAA::MM::Conditional::LogicalORConditionalTypeTopic;
using UMAA::MM::Conditional::MissionStateConditionalType;
using UMAA::MM::Conditional::MissionStateConditionalTypeTopic;
using UMAA::MM::Conditional::ObjectiveStateConditionalType;
using UMAA::MM::Conditional::ObjectiveStateConditionalTypeTopic;
using UMAA::MM::Conditional::PitchRateConditionalType;
using UMAA::MM::Conditional::PitchRateConditionalTypeTopic;
using UMAA::MM::Conditional::RelativeSpeedConditionalType;
using UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic;
using UMAA::MM::Conditional::RollRateConditionalType;
using UMAA::MM::Conditional::RollRateConditionalTypeTopic;
using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::MM::Conditional::SpeedConditionalTypeTopic;
using UMAA::MM::Conditional::TaskStateConditionalType;
using UMAA::MM::Conditional::TaskStateConditionalTypeTopic;
using UMAA::MM::Conditional::TimeConditionalType;
using UMAA::MM::Conditional::TimeConditionalTypeTopic;
using UMAA::MM::Conditional::WaterZoneConditionalType;
using UMAA::MM::Conditional::WaterZoneConditionalTypeTopic;
using UMAA::MM::Conditional::YawRateConditionalType;
using UMAA::MM::Conditional::YawRateConditionalTypeTopic;

class ConditionalReportProviderTest : public ::testing::Test {
 protected:

  static void SetUpTestSuite() {
    reportWriter_ = std::make_shared<LocalReaderSender<ConditionalReportType>>();
    conditionalSetWriter_ = std::make_shared<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>>();
    constraintViolatedConditionalWriter_ = std::make_shared<LocalReaderSender<ConstraintViolatedConditionalType>>();
    depthConditionalWriter_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();
    depthRateConditionalWriter_ = std::make_shared<LocalReaderSender<DepthRateConditionalType>>();
    emitterPresetConditionalWriter_ = std::make_shared<LocalReaderSender<EmitterPresetConditionalType>>();
    expConditionalWriter_ = std::make_shared<LocalReaderSender<ExpConditionalType>>();
    headingSectorConditionalWriter_ = std::make_shared<LocalReaderSender<HeadingSectorConditionalType>>();
    logicalANDConditionalWriter_ = std::make_shared<LocalReaderSender<LogicalANDConditionalType>>();
    logicalNOTConditionalWriter_ = std::make_shared<LocalReaderSender<LogicalNOTConditionalType>>();
    logicalORConditionalWriter_ = std::make_shared<LocalReaderSender<LogicalORConditionalType>>();
    missionStateConditionalWriter_ = std::make_shared<LocalReaderSender<MissionStateConditionalType>>();
    objectiveStateConditionalWriter_ = std::make_shared<LocalReaderSender<ObjectiveStateConditionalType>>();
    pitchRateConditionalWriter_ = std::make_shared<LocalReaderSender<PitchRateConditionalType>>();
    relativeSpeedConditionalWriter_ = std::make_shared<LocalReaderSender<RelativeSpeedConditionalType>>();
    rollRateConditionalWriter_ = std::make_shared<LocalReaderSender<RollRateConditionalType>>();
    speedConditionalWriter_ = std::make_shared<LocalReaderSender<SpeedConditionalType>>();
    taskStateConditionalWriter_ = std::make_shared<LocalReaderSender<TaskStateConditionalType>>();
    timeConditionalWriter_ = std::make_shared<LocalReaderSender<TimeConditionalType>>();
    waterZoneConditionalWriter_ = std::make_shared<LocalReaderSender<WaterZoneConditionalType>>();
    yawRateConditionalWriter_ = std::make_shared<LocalReaderSender<YawRateConditionalType>>();
    io_ = std::make_shared<arlcore::umaa::ConditionalReportProviderIo>(reportWriter_, conditionalSetWriter_, constraintViolatedConditionalWriter_, depthConditionalWriter_,
        depthRateConditionalWriter_, emitterPresetConditionalWriter_, expConditionalWriter_, headingSectorConditionalWriter_, logicalANDConditionalWriter_,
        logicalNOTConditionalWriter_, logicalORConditionalWriter_, missionStateConditionalWriter_, objectiveStateConditionalWriter_, pitchRateConditionalWriter_,
        relativeSpeedConditionalWriter_, rollRateConditionalWriter_, speedConditionalWriter_, taskStateConditionalWriter_, timeConditionalWriter_,
        waterZoneConditionalWriter_, yawRateConditionalWriter_);
  }

  void SetUp() override {
    // Code to run before running each TEST_F()
  }

  void TearDown() override {
    reportWriter_->clear();
    conditionalSetWriter_->clear();
    constraintViolatedConditionalWriter_->clear();
    depthConditionalWriter_->clear();
    depthRateConditionalWriter_->clear();
    emitterPresetConditionalWriter_->clear();
    expConditionalWriter_->clear();
    headingSectorConditionalWriter_->clear();
    logicalANDConditionalWriter_->clear();
    logicalNOTConditionalWriter_->clear();
    logicalORConditionalWriter_->clear();
    missionStateConditionalWriter_->clear();
    objectiveStateConditionalWriter_->clear();
    pitchRateConditionalWriter_->clear();
    relativeSpeedConditionalWriter_->clear();
    rollRateConditionalWriter_->clear();
    speedConditionalWriter_->clear();
    taskStateConditionalWriter_->clear();
    timeConditionalWriter_->clear();
    waterZoneConditionalWriter_->clear();
    yawRateConditionalWriter_->clear();
  }

  static std::shared_ptr<arlcore::umaa::ConditionalReportProviderIo> io_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportType>> reportWriter_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> conditionalSetWriter_;
  static std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> constraintViolatedConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<DepthConditionalType>> depthConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> depthRateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> emitterPresetConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<ExpConditionalType>> expConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> headingSectorConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> logicalANDConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> logicalNOTConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> logicalORConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> missionStateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> objectiveStateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> pitchRateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> relativeSpeedConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<RollRateConditionalType>> rollRateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<SpeedConditionalType>> speedConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> taskStateConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<TimeConditionalType>> timeConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> waterZoneConditionalWriter_;
  static std::shared_ptr<LocalReaderSender<YawRateConditionalType>> yawRateConditionalWriter_;
  
};

std::shared_ptr<arlcore::umaa::ConditionalReportProviderIo> ConditionalReportProviderTest::io_;
std::shared_ptr<LocalReaderSender<ConditionalReportType>> ConditionalReportProviderTest::reportWriter_;
std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> ConditionalReportProviderTest::conditionalSetWriter_;
std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> ConditionalReportProviderTest::constraintViolatedConditionalWriter_;
std::shared_ptr<LocalReaderSender<DepthConditionalType>> ConditionalReportProviderTest::depthConditionalWriter_;
std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> ConditionalReportProviderTest::depthRateConditionalWriter_;
std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> ConditionalReportProviderTest::emitterPresetConditionalWriter_;
std::shared_ptr<LocalReaderSender<ExpConditionalType>> ConditionalReportProviderTest::expConditionalWriter_;
std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> ConditionalReportProviderTest::headingSectorConditionalWriter_;
std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> ConditionalReportProviderTest::logicalANDConditionalWriter_;
std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> ConditionalReportProviderTest::logicalNOTConditionalWriter_;
std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> ConditionalReportProviderTest::logicalORConditionalWriter_;
std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> ConditionalReportProviderTest::missionStateConditionalWriter_;
std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> ConditionalReportProviderTest::objectiveStateConditionalWriter_;
std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> ConditionalReportProviderTest::pitchRateConditionalWriter_;
std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> ConditionalReportProviderTest::relativeSpeedConditionalWriter_;
std::shared_ptr<LocalReaderSender<RollRateConditionalType>> ConditionalReportProviderTest::rollRateConditionalWriter_;
std::shared_ptr<LocalReaderSender<SpeedConditionalType>> ConditionalReportProviderTest::speedConditionalWriter_;
std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> ConditionalReportProviderTest::taskStateConditionalWriter_;
std::shared_ptr<LocalReaderSender<TimeConditionalType>> ConditionalReportProviderTest::timeConditionalWriter_;
std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> ConditionalReportProviderTest::waterZoneConditionalWriter_;
std::shared_ptr<LocalReaderSender<YawRateConditionalType>> ConditionalReportProviderTest::yawRateConditionalWriter_;

TEST_F(ConditionalReportProviderTest, testGetTopicAndWriter) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  ConstraintViolatedConditionalType cv;
  auto [cvTopic, cvWriter] = provider.getTopicAndWriter(cv);
  EXPECT_EQ(cvTopic, ConstraintViolatedConditionalTypeTopic);
  EXPECT_EQ(cvWriter, constraintViolatedConditionalWriter_);

  DepthConditionalType dc;
  auto [dcTopic, dcWriter] = provider.getTopicAndWriter(dc);
  EXPECT_EQ(dcTopic, DepthConditionalTypeTopic);
  EXPECT_EQ(dcWriter, depthConditionalWriter_);

  DepthRateConditionalType dr;
  auto [drTopic, drWriter] = provider.getTopicAndWriter(dr);
  EXPECT_EQ(drTopic, DepthRateConditionalTypeTopic);
  EXPECT_EQ(drWriter, depthRateConditionalWriter_);

  EmitterPresetConditionalType ep;
  auto [epTopic, epWriter] = provider.getTopicAndWriter(ep);
  EXPECT_EQ(epTopic, EmitterPresetConditionalTypeTopic);
  EXPECT_EQ(epWriter, emitterPresetConditionalWriter_);

  ExpConditionalType ex;
  auto [exTopic, exWriter] = provider.getTopicAndWriter(ex);
  EXPECT_EQ(exTopic, ExpConditionalTypeTopic);
  EXPECT_EQ(exWriter, expConditionalWriter_);

  HeadingSectorConditionalType hs;
  auto [hsTopic, hsWriter] = provider.getTopicAndWriter(hs);
  EXPECT_EQ(hsTopic, HeadingSectorConditionalTypeTopic);
  EXPECT_EQ(hsWriter, headingSectorConditionalWriter_);

  LogicalANDConditionalType la;
  auto [laTopic, laWriter] = provider.getTopicAndWriter(la);
  EXPECT_EQ(laTopic, LogicalANDConditionalTypeTopic);
  EXPECT_EQ(laWriter, logicalANDConditionalWriter_);

  LogicalNOTConditionalType ln;
  auto [lnTopic, lnWriter] = provider.getTopicAndWriter(ln);
  EXPECT_EQ(lnTopic, LogicalNOTConditionalTypeTopic);
  EXPECT_EQ(lnWriter, logicalNOTConditionalWriter_);

  LogicalORConditionalType lo;
  auto [loTopic, loWriter] = provider.getTopicAndWriter(lo);
  EXPECT_EQ(loTopic, LogicalORConditionalTypeTopic);
  EXPECT_EQ(loWriter, logicalORConditionalWriter_);

  MissionStateConditionalType ms;
  auto [msTopic, msWriter] = provider.getTopicAndWriter(ms);
  EXPECT_EQ(msTopic, MissionStateConditionalTypeTopic);
  EXPECT_EQ(msWriter, missionStateConditionalWriter_);

  ObjectiveStateConditionalType os;
  auto [osTopic, osWriter] = provider.getTopicAndWriter(os);
  EXPECT_EQ(osTopic, ObjectiveStateConditionalTypeTopic);
  EXPECT_EQ(osWriter, objectiveStateConditionalWriter_);

  PitchRateConditionalType pr;
  auto [prTopic, prWriter] = provider.getTopicAndWriter(pr);
  EXPECT_EQ(prTopic, PitchRateConditionalTypeTopic);
  EXPECT_EQ(prWriter, pitchRateConditionalWriter_);

  RelativeSpeedConditionalType rs;
  auto [rsTopic, rsWriter] = provider.getTopicAndWriter(rs);
  EXPECT_EQ(rsTopic, RelativeSpeedConditionalTypeTopic);
  EXPECT_EQ(rsWriter, relativeSpeedConditionalWriter_);

  RollRateConditionalType rr;
  auto [rrTopic, rrWriter] = provider.getTopicAndWriter(rr);
  EXPECT_EQ(rrTopic, RollRateConditionalTypeTopic);
  EXPECT_EQ(rrWriter, rollRateConditionalWriter_);

  SpeedConditionalType sp;
  auto [spTopic, spWriter] = provider.getTopicAndWriter(sp);
  EXPECT_EQ(spTopic, SpeedConditionalTypeTopic);
  EXPECT_EQ(spWriter, speedConditionalWriter_);

  TaskStateConditionalType ts;
  auto [tsTopic, tsWriter] = provider.getTopicAndWriter(ts);
  EXPECT_EQ(tsTopic, TaskStateConditionalTypeTopic);
  EXPECT_EQ(tsWriter, taskStateConditionalWriter_);

  TimeConditionalType tc;
  auto [tcTopic, tcWriter] = provider.getTopicAndWriter(tc);
  EXPECT_EQ(tcTopic, TimeConditionalTypeTopic);
  EXPECT_EQ(tcWriter, timeConditionalWriter_);

  WaterZoneConditionalType wz;
  auto [wzTopic, wzWriter] = provider.getTopicAndWriter(wz);
  EXPECT_EQ(wzTopic, WaterZoneConditionalTypeTopic);
  EXPECT_EQ(wzWriter, waterZoneConditionalWriter_);

  YawRateConditionalType yr;
  auto [yrTopic, yrWriter] = provider.getTopicAndWriter(yr);
  EXPECT_EQ(yrTopic, YawRateConditionalTypeTopic);
  EXPECT_EQ(yrWriter, yawRateConditionalWriter_);
}

TEST_F(ConditionalReportProviderTest, testAddConditionals) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  std::array<ConditionalType, 3> conditionals;
  std::for_each(conditionals.begin(), conditionals.end(), [&](ConditionalType& c) {
    c.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
    EXPECT_EQ(provider.addConditional(c), SendStatus::SUCCESS);
  });

  auto c1 = provider.getConditionalById(arlcore::NumericGuid(conditionals[0].conditionalID()));
  ASSERT_TRUE(c1.has_value());
  EXPECT_EQ(c1.value(), conditionals[0]);

  auto list = provider.getConditionals();
  EXPECT_EQ(list.size(), conditionals.size());
}

TEST_F(ConditionalReportProviderTest, testAddSpecializations) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  ConstraintViolatedConditionalType cv;
  auto [cvStatus, cvId] = provider.addSpecialization(&cv);
  EXPECT_EQ(cvStatus, arlcore::io::SendStatus::SUCCESS);
  std::optional<ConditionalType> conditional = provider.getConditionalById(cvId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), cv.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), cv.specializationReferenceTimestamp());
  ConstraintViolatedConditionalType cvRecieved;
  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(cv, cvRecieved);

  DepthConditionalType dc;
  auto [dcStatus, dcId] = provider.addSpecialization(&dc);
  EXPECT_EQ(dcStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(dcId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), dc.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), dc.specializationReferenceTimestamp());
  DepthConditionalType dcRecieved;
  EXPECT_EQ(depthConditionalWriter_->read(&dcRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(dc, dcRecieved);

  DepthRateConditionalType dr;
  auto [drStatus, drId] = provider.addSpecialization(&dr);
  EXPECT_EQ(drStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(drId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), dr.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), dr.specializationReferenceTimestamp());
  DepthRateConditionalType drRecieved;
  EXPECT_EQ(depthRateConditionalWriter_->read(&drRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(dr, drRecieved);

  EmitterPresetConditionalType ep;
  auto [epStatus, epId] = provider.addSpecialization(&ep);
  EXPECT_EQ(epStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(epId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), ep.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), ep.specializationReferenceTimestamp());
  EmitterPresetConditionalType epRecieved;
  EXPECT_EQ(emitterPresetConditionalWriter_->read(&epRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(ep, epRecieved);

  ExpConditionalType ex;
  auto [exStatus, exId] = provider.addSpecialization(&ex);
  EXPECT_EQ(exStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(exId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), ex.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), ex.specializationReferenceTimestamp());
  ExpConditionalType exRecieved;
  EXPECT_EQ(expConditionalWriter_->read(&exRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(ex, exRecieved);

  HeadingSectorConditionalType hs;
  auto [hsStatus, hsId] = provider.addSpecialization(&hs);
  EXPECT_EQ(hsStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(hsId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), hs.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), hs.specializationReferenceTimestamp());
  HeadingSectorConditionalType hsRecieved;
  EXPECT_EQ(headingSectorConditionalWriter_->read(&hsRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(hs, hsRecieved);

  LogicalANDConditionalType la;
  auto [laStatus, laId] = provider.addSpecialization(&la);
  EXPECT_EQ(laStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(laId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), la.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), la.specializationReferenceTimestamp());
  LogicalANDConditionalType laRecieved;
  EXPECT_EQ(logicalANDConditionalWriter_->read(&laRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(la, laRecieved);

  LogicalNOTConditionalType ln;
  auto [lnStatus, lnId] = provider.addSpecialization(&ln);
  EXPECT_EQ(lnStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(lnId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), ln.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), ln.specializationReferenceTimestamp());
  LogicalNOTConditionalType lnRecieved;
  EXPECT_EQ(logicalNOTConditionalWriter_->read(&lnRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(ln, lnRecieved);

  LogicalORConditionalType lo;
  auto [loStatus, loId] = provider.addSpecialization(&lo);
  EXPECT_EQ(loStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(loId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), lo.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), lo.specializationReferenceTimestamp());
  LogicalORConditionalType loRecieved;
  EXPECT_EQ(logicalORConditionalWriter_->read(&loRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(lo, loRecieved);

  MissionStateConditionalType ms;
  auto [msStatus, msId] = provider.addSpecialization(&ms);
  EXPECT_EQ(msStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(msId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), ms.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), ms.specializationReferenceTimestamp());
  MissionStateConditionalType msRecieved;
  EXPECT_EQ(missionStateConditionalWriter_->read(&msRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(ms, msRecieved);

  ObjectiveStateConditionalType os;
  auto [osStatus, osId] = provider.addSpecialization(&os);
  EXPECT_EQ(osStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(osId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), os.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), os.specializationReferenceTimestamp());
  ObjectiveStateConditionalType osRecieved;
  EXPECT_EQ(objectiveStateConditionalWriter_->read(&osRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(os, osRecieved);

  PitchRateConditionalType pr;
  auto [prStatus, prId] = provider.addSpecialization(&pr);
  EXPECT_EQ(prStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(prId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), pr.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), pr.specializationReferenceTimestamp());
  PitchRateConditionalType prRecieved;
  EXPECT_EQ(pitchRateConditionalWriter_->read(&prRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(pr, prRecieved);

  RelativeSpeedConditionalType rs;
  auto [rsStatus, rsId] = provider.addSpecialization(&rs);
  EXPECT_EQ(rsStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(rsId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), rs.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), rs.specializationReferenceTimestamp());
  RelativeSpeedConditionalType rsRecieved;
  EXPECT_EQ(relativeSpeedConditionalWriter_->read(&rsRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(rs, rsRecieved);

  RollRateConditionalType rr;
  auto [rrStatus, rrId] = provider.addSpecialization(&rr);
  EXPECT_EQ(rrStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(rrId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), rr.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), rr.specializationReferenceTimestamp());
  RollRateConditionalType rrRecieved;
  EXPECT_EQ(rollRateConditionalWriter_->read(&rrRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(rr, rrRecieved);

  SpeedConditionalType sp;
  auto [spStatus, spId] = provider.addSpecialization(&sp);
  EXPECT_EQ(spStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(spId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), sp.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), sp.specializationReferenceTimestamp());
  SpeedConditionalType spRecieved;
  EXPECT_EQ(speedConditionalWriter_->read(&spRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(sp, spRecieved);

  TaskStateConditionalType ts;
  auto [tsStatus, tsId] = provider.addSpecialization(&ts);
  EXPECT_EQ(tsStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(tsId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), ts.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), ts.specializationReferenceTimestamp());
  TaskStateConditionalType tsRecieved;
  EXPECT_EQ(taskStateConditionalWriter_->read(&tsRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(ts, tsRecieved);

  TimeConditionalType tc;
  auto [tcStatus, tcId] = provider.addSpecialization(&tc);
  EXPECT_EQ(tcStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(tcId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), tc.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), tc.specializationReferenceTimestamp());
  TimeConditionalType tcRecieved;
  EXPECT_EQ(timeConditionalWriter_->read(&tcRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(tc, tcRecieved);

  WaterZoneConditionalType wz;
  auto [wzStatus, wzId] = provider.addSpecialization(&wz);
  EXPECT_EQ(wzStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(wzId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), wz.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), wz.specializationReferenceTimestamp());
  WaterZoneConditionalType wzRecieved;
  EXPECT_EQ(waterZoneConditionalWriter_->read(&wzRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(wz, wzRecieved);

  YawRateConditionalType yr;
  auto [yrStatus, yrId] = provider.addSpecialization(&yr);
  EXPECT_EQ(yrStatus, arlcore::io::SendStatus::SUCCESS);
  conditional = provider.getConditionalById(yrId);
  ASSERT_TRUE(conditional.has_value());
  EXPECT_EQ(conditional->specializationID(), yr.specializationReferenceID());
  EXPECT_EQ(conditional->specializationTimestamp(), yr.specializationReferenceTimestamp());
  YawRateConditionalType yrRecieved;
  EXPECT_EQ(yawRateConditionalWriter_->read(&yrRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(yr, yrRecieved);
}

TEST_F(ConditionalReportProviderTest, testSendReport) {
  arlcore::NumericGuid sourceId = arlcore::UuidFactory::getInstance().generateGuid();
  arlcore::umaa::conditional::ConditionalReportProvider provider(sourceId, io_);
  arlcore::umaa::LargeSetReader<ConditionalType, ConditionalReportTypeConditionalsSetElement> reader(conditionalSetWriter_);

  ConstraintViolatedConditionalType cv;
  auto [cvStatus, cvId] = provider.addSpecialization(&cv);
  EXPECT_EQ(cvStatus, arlcore::io::SendStatus::SUCCESS);

  DepthConditionalType dc;
  auto [dcStatus, dcId] = provider.addSpecialization(&dc);
  EXPECT_EQ(dcStatus, arlcore::io::SendStatus::SUCCESS);

  DepthRateConditionalType dr;
  auto [drStatus, drId] = provider.addSpecialization(&dr);
  EXPECT_EQ(drStatus, arlcore::io::SendStatus::SUCCESS);

  std::vector<ConditionalType> conditionals = provider.getConditionals();
  provider.sendReport();
  ConditionalReportType report;
  EXPECT_EQ(reportWriter_->read(&report), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(sourceId, report.source().id());
  auto result = reader.getSetFromMetadata(report.conditionalsSetMetadata());
  ASSERT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_TRUE(std::is_permutation(set->begin(), set->end(), conditionals.begin()));
  } else {
    FAIL() << "Unable to aquire lock";
  }
}

TEST_F(ConditionalReportProviderTest, testRemoveConditional) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  std::array<ConditionalType, 3> conditionals;
  std::for_each(conditionals.begin(), conditionals.end(), [&](ConditionalType& c) {
    c.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
    EXPECT_EQ(provider.addConditional(c), SendStatus::SUCCESS);
  });

  auto c1 = provider.getConditionalById(arlcore::NumericGuid(conditionals[0].conditionalID()));
  ASSERT_TRUE(c1.has_value());
  EXPECT_EQ(c1.value(), conditionals[0]);

  auto list = provider.getConditionals();
  EXPECT_EQ(list.size(), conditionals.size());

  EXPECT_EQ(provider.removeConditional(conditionals[0]), SendStatus::SUCCESS);
  c1 = provider.getConditionalById(arlcore::NumericGuid(conditionals[0].conditionalID()));
  EXPECT_FALSE(c1.has_value());

  list = provider.getConditionals();
  EXPECT_EQ(list.size(), conditionals.size() - 1);
}

TEST_F(ConditionalReportProviderTest, testRemoveSpecialization) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  ConstraintViolatedConditionalType cv;
  auto [cvStatus, cvId] = provider.addSpecialization(&cv);
  EXPECT_EQ(cvStatus, arlcore::io::SendStatus::SUCCESS);
  ConstraintViolatedConditionalType cvRecieved;
  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(cv, cvRecieved);

  DepthConditionalType dc;
  auto [dcStatus, dcId] = provider.addSpecialization(&dc);
  EXPECT_EQ(dcStatus, arlcore::io::SendStatus::SUCCESS);
  DepthConditionalType dcRecieved;
  EXPECT_EQ(depthConditionalWriter_->read(&dcRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(dc, dcRecieved);

  DepthRateConditionalType dr;
  auto [drStatus, drId] = provider.addSpecialization(&dr);
  EXPECT_EQ(drStatus, arlcore::io::SendStatus::SUCCESS);
  DepthRateConditionalType drRecieved;
  EXPECT_EQ(depthRateConditionalWriter_->read(&drRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(dr, drRecieved);

  std::vector<ConditionalType> allConditionals = provider.getConditionals();
  std::optional<ConditionalType> dcConditional = provider.getConditionalById(dcId);
  ASSERT_TRUE(dcConditional.has_value());

  EXPECT_EQ(provider.removeSpecialization(dcId), SendStatus::SUCCESS);
  EXPECT_EQ(depthConditionalWriter_->read(&dcRecieved), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(dc.specializationReferenceID(), dcRecieved.specializationReferenceID());

  std::vector<ConditionalType> subsetConditionals = provider.getConditionals();
  std::vector<ConditionalType> difference;

  // std::set_difference requires both ranges sorted by the comparator; the provider returns
  // conditionals in container order, which is unrelated to ConditionalType's operator<.
  std::sort(allConditionals.begin(), allConditionals.end());
  std::sort(subsetConditionals.begin(), subsetConditionals.end());
  std::set_difference(allConditionals.begin(), allConditionals.end(), subsetConditionals.begin(), subsetConditionals.end(), std::back_inserter(difference));
  EXPECT_EQ(difference.size(), 1);
  EXPECT_EQ(difference.at(0), dcConditional.value());
}

TEST_F(ConditionalReportProviderTest, testUpdateConditional) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  std::array<ConditionalType, 3> conditionals;
  std::for_each(conditionals.begin(), conditionals.end(), [&](ConditionalType& c) {
    c.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    c.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
    EXPECT_EQ(provider.addConditional(c), SendStatus::SUCCESS);
  });

  auto c1 = provider.getConditionalById(arlcore::NumericGuid(conditionals[0].conditionalID()));
  ASSERT_TRUE(c1.has_value());
  EXPECT_EQ(c1.value(), conditionals[0]);

  auto list = provider.getConditionals();
  EXPECT_EQ(list.size(), conditionals.size());

  ConditionalType prev = conditionals[0];
  conditionals[0].specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());

  EXPECT_EQ(provider.updateConditional(conditionals[0]), SendStatus::SUCCESS);

  std::optional<ConditionalType> found = provider.getConditionalById(arlcore::NumericGuid(conditionals[0].conditionalID()));
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found.value(), conditionals[0]);
  EXPECT_NE(found.value(), prev);
}

TEST_F(ConditionalReportProviderTest, testUpdateSpecialization) {
  arlcore::umaa::conditional::ConditionalReportProvider provider(arlcore::NIL_GUID, io_);

  ConstraintViolatedConditionalType cv;
  auto [cvStatus, cvId] = provider.addSpecialization(&cv);
  EXPECT_EQ(cvStatus, arlcore::io::SendStatus::SUCCESS);
  std::optional<ConditionalType> initial = provider.getConditionalById(cvId);
  ASSERT_TRUE(initial.has_value());

  ConstraintViolatedConditionalType cvRecieved;
  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(cv, cvRecieved);

  ConstraintViolatedConditionalType cv2 = cv;
  EXPECT_EQ(provider.updateSpecialization(cvId, &cv2), SendStatus::SUCCESS);
  std::optional<ConditionalType> updated = provider.getConditionalById(cvId);
  ASSERT_TRUE(updated.has_value());

  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(cv2, cvRecieved);

  EXPECT_EQ(cv.specializationReferenceID(), cv2.specializationReferenceID());
  EXPECT_EQ(initial->specializationID(), updated->specializationID());
  EXPECT_LT(cv.specializationReferenceTimestamp(), cv2.specializationReferenceTimestamp());
  EXPECT_LT(initial->specializationTimestamp(), updated->specializationTimestamp());

  ConstraintViolatedConditionalType cv3 = cv2;
  // Generate a new specialization ID
  EXPECT_EQ(provider.updateSpecialization(cvId, &cv3, true), SendStatus::SUCCESS);
  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(cvRecieved.specializationReferenceID(), cv.specializationReferenceID());

  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(cv3, cvRecieved);
  EXPECT_NE(cvRecieved.specializationReferenceID(), cv.specializationReferenceID());

  // Change conditional specialization type
  DepthConditionalType dc;
  dc.specializationReferenceID(cv3.specializationReferenceID());
  EXPECT_EQ(provider.updateSpecialization(cvId, &dc), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(constraintViolatedConditionalWriter_->read(&cvRecieved), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(cvRecieved.specializationReferenceID(), cv3.specializationReferenceID());

  DepthConditionalType dcRecieved;
  EXPECT_EQ(depthConditionalWriter_->read(&dcRecieved), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(dc, dcRecieved);

}