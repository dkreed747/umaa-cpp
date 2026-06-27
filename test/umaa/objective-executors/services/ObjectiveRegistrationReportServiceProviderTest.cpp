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

// See full GTest Documentation for reference:
// https://google.github.io/googletest/
#include <gtest/gtest.h>

#include "LocalReaderSender.h"
#include "ObjectiveRegistrationReportServiceProvider.h"
#include "UuidFactory.h"

class ObjectiveRegistrationReportServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    objRegReportSvc_ = std::make_unique<arlcore::umaa::ObjectiveRegistrationReportServiceProvider>(
      sourceId_,
      reportIo_ = std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveRegistrationStatusType>>());
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  const arlcore::NumericGuid sourceId_ = arlcore::UuidFactory::getInstance().generateGuid();
  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveRegistrationStatusType>> reportIo_;
  std::unique_ptr<arlcore::umaa::ObjectiveRegistrationReportServiceProvider> objRegReportSvc_;
  const std::string testTopic1_ = "UMAA::TestTopic1";
  const std::string testTopic2_ = "UMAA::TestTopic2";
};

TEST_F(ObjectiveRegistrationReportServiceTest, RegisterObjective) {
  // Register the objective will publish a ObjectiveRegistrationStatusType
  ASSERT_EQ(objRegReportSvc_->registerObjective(testTopic1_), arlcore::io::SendStatus::SUCCESS);

  // Calling registerObjective() on an already registered topic will not publish data and return ERROR
  EXPECT_EQ(objRegReportSvc_->registerObjective(testTopic1_), arlcore::io::SendStatus::ERROR);

  // Check buffer for single report
  ASSERT_EQ(reportIo_->count(), 1);

  // Read the report and validate data
  arlcore::umaa::ObjectiveRegistrationStatusType report;
  ASSERT_EQ(reportIo_->read(&report), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(report.objectiveTopic(), testTopic1_);
  EXPECT_EQ(report.source(), sourceId_.getGuid());
  EXPECT_GT(arlcore::umaa::getTimestamp().nanoseconds(), report.timeStamp().nanoseconds());
}

TEST_F(ObjectiveRegistrationReportServiceTest, UnregisterObjective) {
  // Calling unregisterObjective() on a topic that hasn't been registered returns ERROR
  EXPECT_EQ(objRegReportSvc_->unregisterObjective(testTopic1_), arlcore::io::SendStatus::ERROR);


  // Register two test topics
  ASSERT_EQ(objRegReportSvc_->registerObjective(testTopic1_), arlcore::io::SendStatus::SUCCESS);
  ASSERT_EQ(objRegReportSvc_->registerObjective(testTopic2_), arlcore::io::SendStatus::SUCCESS);

  // Check buffer for 2 reports
  ASSERT_EQ(reportIo_->count(), 2);
  reportIo_->clear();

  // Unregister from test topic 1
  EXPECT_EQ(objRegReportSvc_->unregisterObjective(testTopic1_), arlcore::io::SendStatus::SUCCESS);

  // Check for the dispose sample
  ASSERT_EQ(reportIo_->count(), 1);

  // Read the report and validate data
  arlcore::umaa::ObjectiveRegistrationStatusType report;
  ASSERT_EQ(reportIo_->read(&report), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(report.objectiveTopic(), testTopic1_);
  EXPECT_EQ(report.source(), sourceId_.getGuid());

  // Unregister from test topic 1 again should return error
  EXPECT_EQ(objRegReportSvc_->unregisterObjective(testTopic1_), arlcore::io::SendStatus::ERROR);

  // Unregister from test topic 2
  EXPECT_EQ(objRegReportSvc_->unregisterObjective(testTopic2_), arlcore::io::SendStatus::SUCCESS);

  // Check for the dispose sample
  ASSERT_EQ(reportIo_->count(), 1);

  // Read the report and validate data
  ASSERT_EQ(reportIo_->read(&report), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(report.objectiveTopic(), testTopic2_);
  EXPECT_EQ(report.source(), sourceId_.getGuid());
}
