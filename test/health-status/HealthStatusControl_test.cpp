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


#include <string>
#include <dds/dds.hpp>
#include <gtest/gtest.h>
#include <memory>

#include <UMAA/SO/HealthReport/HealthReportType.hpp>

#include "HealthStatusControl.h"
#include "CycloneUtilities.h"
#include "RealtimeSystemClock.h"
#include "NumericGuid.h"
#include "UuidFactory.h"
#include "LocalReaderSender.h"

class HealthStatusControlTest : public ::testing::Test {
  protected:

    void SetUp() {
      healthSourceId_ = UMAA::Common::IdentifierType(arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
                                                     arlcore::NIL_GUID.getGuid());
      healthReaderSender_ = std::make_shared<arlcore::io::LocalReaderSender<HealthReportType>>();
      healthControl_ = std::make_unique<arlcore::HealthStatusControl>(healthSourceId_, healthReaderSender_);

      UMAA::Common::IdentifierType resourceIdA_(
        arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
        arlcore::NIL_GUID.getGuid());
  
      UMAA::Common::IdentifierType resourceIdB_(
        arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
        arlcore::NIL_GUID.getGuid());
    }

    // Class members can be used by all tests in the test suite.
    static std::shared_ptr<arlcore::io::LocalReaderSender<HealthReportType>> healthReaderSender_;
    static UMAA::Common::IdentifierType healthSourceId_;
    static UMAA::Common::IdentifierType resourceIdA_;
    static UMAA::Common::IdentifierType resourceIdB_;
    static std::unique_ptr<arlcore::HealthStatusControl> healthControl_;
    static const uint32_t HEARTBEATMS_ = 1000;

};

  std::shared_ptr<arlcore::io::LocalReaderSender<HealthReportType>> HealthStatusControlTest::healthReaderSender_;
  UMAA::Common::IdentifierType HealthStatusControlTest::healthSourceId_;
  UMAA::Common::IdentifierType HealthStatusControlTest::resourceIdA_;
  UMAA::Common::IdentifierType HealthStatusControlTest::resourceIdB_;
  std::unique_ptr<arlcore::HealthStatusControl> HealthStatusControlTest::healthControl_;
  const uint32_t HealthStatusControlTest::HEARTBEATMS_;

TEST_F(HealthStatusControlTest, registerAndDeregisterResource) {

  UMAA::SO::HealthReport::HealthReportType healthReport;
  std::string message = "No error condition exists.";

  /* Registers ResourceIdA/SOFTWARE - 1 total */
  healthControl_->registerResource(resourceIdA_, ErrorCodeEnumType::SOFTWARE);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::SOFTWARE);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdA_);
  EXPECT_EQ(healthReport.logTime().seconds(), arlcore::umaa::getTimestamp().seconds()); // Check the seconds
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());

  /* Registers ResourceIdA/SOFTWARE. Since it has already been registered, no new report is sent. */
  healthControl_->registerResource(resourceIdA_, ErrorCodeEnumType::SOFTWARE);
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::NO_DATA);

  /* Registers ResourceIdA/ACTUATOR  - 2 total */
  healthControl_->registerResource(resourceIdA_, ErrorCodeEnumType::ACTUATOR);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::ACTUATOR);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdA_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());

  /* Registers ResourceIdB/POWER - 3 total */
  healthControl_->registerResource(resourceIdB_, ErrorCodeEnumType::POWER);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::POWER);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());

  /* Deregisters ResourceIdA/SOFTWARE - 2 total */
  healthControl_->deregisterResource(resourceIdA_, ErrorCodeEnumType::SOFTWARE);
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::DISPOSED);

  /* Deregisters ResourceIdA/NONE. Since it does not exist, nothing happens. */
  healthControl_->deregisterResource(resourceIdA_, ErrorCodeEnumType::NONE); // DNE
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::NO_DATA);

  // Tests how many reports are stored in the in HealthStatusControl

  ASSERT_EQ(healthReaderSender_->count(), 0);

  healthControl_->startThread();
  // Sleeps for a little less than the heartbeat duration to ensure that reports are only sent once
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Expected 2 Health Reports to be sent 
  ASSERT_EQ(healthReaderSender_->count(), 2);

  healthControl_->stopThread();

}

TEST_F(HealthStatusControlTest, updateSystemHealth) {

  UMAA::SO::HealthReport::HealthReportType healthReport;

  /* PROCESSOR and INFO */ 
  auto severity = ErrorConditionEnumType::INFO;
  auto code = ErrorCodeEnumType::PROCESSOR;
  std::string message = "PROCESSOR with INFO health level";


  healthControl_->updateSystemHealth(resourceIdA_, code, severity, message);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), severity);
  EXPECT_EQ(healthReport.code(), code);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdA_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());


  /* PROCESSOR and NONE -> updates a pre-existing ResourceID/Code Pair */
  severity = ErrorConditionEnumType::NONE;
  code = ErrorCodeEnumType::PROCESSOR;
  message = "PROCESSOR with NONE health level";

  healthControl_->updateSystemHealth(resourceIdA_, code, severity, message);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::PROCESSOR);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdA_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());


  /* FILESYS and WARN */
  severity = ErrorConditionEnumType::WARN;
  code = ErrorCodeEnumType::FILESYS;
  message = "FILESYS with WARN health level";

  healthControl_->updateSystemHealth(resourceIdB_, code, severity, message);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::WARN);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::FILESYS);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.logTime().seconds(), arlcore::umaa::getTimestamp().seconds());
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());

  /* ROM and WARN */
  severity = ErrorConditionEnumType::WARN;
  code  = ErrorCodeEnumType::ROM;
  message = "ROM with WARN health level";

  healthControl_->updateSystemHealth(resourceIdB_, code, severity,  message);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::WARN);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::ROM);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());


  /* SENSOR and WARN */
  severity = ErrorConditionEnumType::WARN;
  code = ErrorCodeEnumType::SENSOR;
  message = "SENSOR with WARN health level";

  healthControl_->updateSystemHealth(resourceIdB_, code, severity, message);

  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::WARN);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::SENSOR);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());


  /* SOFTWARE and WARN */
  severity = ErrorConditionEnumType::NONE;
  code = ErrorCodeEnumType::SENSOR;
  message = "SENSOR with WARN health level";

  healthControl_->updateSystemHealth(resourceIdB_, code, severity, message);
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::SENSOR);
  EXPECT_EQ(healthReport.status().value(), message);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);
  // Just check the seconds of the timestamp.
  EXPECT_EQ(healthReport.timeStamp().seconds(), arlcore::umaa::getTimestamp().seconds());

  ASSERT_EQ(healthReaderSender_->count(), 0);

  healthControl_->startThread();
  // Sleeps for a little less than the heartbeat duration to ensure that reports are only sent once
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Expected 4 Health Reports to be sent 
  ASSERT_EQ(healthReaderSender_->count(), 4);

  healthControl_->stopThread();

}

TEST_F(HealthStatusControlTest, sendReportsHeartbeat) {

  UMAA::SO::HealthReport::HealthReportType healthReport;
  ASSERT_EQ(healthReaderSender_->count(), 0);
  healthControl_->registerResource(resourceIdA_, ErrorCodeEnumType::RAM);
  healthControl_->registerResource(resourceIdB_, ErrorCodeEnumType::POWER);
  ASSERT_EQ(healthReaderSender_->count(), 2);
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  ASSERT_EQ(healthReaderSender_->count(), 0);

  /* TEST 1 - Report 2 Health Reports*/
  healthControl_->startThread();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  arlcore::io::SampleEnvelope<HealthReportType> readReports[5];

  ASSERT_EQ(healthReaderSender_->readUpToN(readReports, 5), 2);
  // HealthStatusControl iterates over an unordered map, so the order of which
  //    reports are sent are not necessarily the order in which they were registerd
  EXPECT_EQ(readReports[0].data.source(), healthSourceId_);
  EXPECT_EQ(readReports[0].data.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(readReports[0].data.code(), ErrorCodeEnumType::POWER);
  EXPECT_EQ(readReports[0].data.resourceID(), resourceIdB_);
  EXPECT_EQ(readReports[1].data.source(), healthSourceId_);
  EXPECT_EQ(readReports[1].data.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(readReports[1].data.code(), ErrorCodeEnumType::RAM);
  EXPECT_EQ(readReports[1].data.resourceID(), resourceIdA_);

  healthControl_->stopThread();

  /* TEST 2 - Updates a one of the Health Reports */
  healthControl_->updateSystemHealth(resourceIdA_, ErrorCodeEnumType::RAM, ErrorConditionEnumType::FAIL, "");
  healthControl_->startThread();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  ASSERT_EQ(healthReaderSender_->readUpToN(readReports, 5), 3);
  // Report sent when System was updated
  EXPECT_EQ(readReports[0].data.source(), healthSourceId_);
  EXPECT_EQ(readReports[0].data.severity(), ErrorConditionEnumType::FAIL);
  EXPECT_EQ(readReports[0].data.code(), ErrorCodeEnumType::RAM);
  EXPECT_EQ(readReports[0].data.resourceID(), resourceIdB_);
  // Reports send from heartbeat
  EXPECT_EQ(readReports[1].data.source(), healthSourceId_);
  EXPECT_EQ(readReports[1].data.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(readReports[1].data.code(), ErrorCodeEnumType::POWER);
  EXPECT_EQ(readReports[1].data.resourceID(), resourceIdA_);
  EXPECT_EQ(readReports[2].data.source(), healthSourceId_);
  EXPECT_EQ(readReports[2].data.severity(), ErrorConditionEnumType::FAIL);
  EXPECT_EQ(readReports[2].data.code(), ErrorCodeEnumType::RAM);
  EXPECT_EQ(readReports[2].data.resourceID(), resourceIdB_);
  healthControl_->stopThread();

  // logTime should be the same, but readReport[0] was sent before readReport[2]
  EXPECT_EQ(readReports[0].data.logTime(), readReports[2].data.logTime());
  EXPECT_LT(readReports[0].data.timeStamp(), readReports[2].data.timeStamp());

  /* TEST 3 - Deregisters a Health Report */

  healthControl_->deregisterResource(resourceIdB_, ErrorCodeEnumType::RAM);
  healthControl_->startThread();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  ASSERT_EQ(healthReaderSender_->count(), 2);
  // Report Disposed 
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::DISPOSED);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::FAIL);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::RAM);
  EXPECT_EQ(healthReport.resourceID(), resourceIdB_);

  // Report send from heartbeat
  EXPECT_EQ(healthReaderSender_->read(&healthReport), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(healthReport.source(), healthSourceId_);
  EXPECT_EQ(healthReport.severity(), ErrorConditionEnumType::NONE);
  EXPECT_EQ(healthReport.code(), ErrorCodeEnumType::POWER);
  EXPECT_EQ(healthReport.resourceID(), resourceIdA_);
  healthControl_->stopThread();
}