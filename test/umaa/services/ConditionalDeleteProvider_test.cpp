//---------------------------------------------------------------------------
// Copyright 2026 Pennsylvania State University
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

#include "ConditionalDeleteProvider.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"

using arlcore::io::LocalReaderSender;
using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::SpeedConditionalType;

class ConditionalDeleteProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    reportRw_ = std::make_shared<LocalReaderSender<ConditionalReportType>>();
    setElementRw_ = std::make_shared<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>>();
    ownedDepthRw_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();
    ownedSpeedRw_ = std::make_shared<LocalReaderSender<SpeedConditionalType>>();

    auto reportIo = std::make_shared<arlcore::umaa::ConditionalReportProviderIo>(
      reportRw_, setElementRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ConstraintViolatedConditionalType>>(),
      ownedDepthRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::DepthRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::EmitterPresetConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ExpConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::HeadingSectorConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalANDConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalNOTConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalORConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::MissionStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ObjectiveStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::PitchRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RelativeSpeedConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RollRateConditionalType>>(),
      ownedSpeedRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TaskStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TimeConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::WaterZoneConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::YawRateConditionalType>>());
    reportProvider_ = std::make_shared<arlcore::umaa::conditional::ConditionalReportProvider>(
      arlcore::UuidFactory::getInstance().generateGuid(), reportIo);

    cmdRw_ = std::make_shared<LocalReaderSender<ConditionalDeleteCommandType>>();
    ackRw_ = std::make_shared<LocalReaderSender<ConditionalDeleteCommandAckReportType>>();
    statusRw_ = std::make_shared<LocalReaderSender<ConditionalDeleteCommandStatusType>>();
    deleteIo_ = std::make_shared<ConditionalDeleteProviderIo>(cmdRw_, ackRw_, statusRw_);
    source_ = arlcore::UuidFactory::getInstance().generateGuid();
  }

  arlcore::NumericGuid seedDepthConditional() {
    DepthConditionalType payload;
    auto [status, id] = reportProvider_->addSpecialization(&payload);
    EXPECT_EQ(status, SendStatus::SUCCESS);
    DepthConditionalType published;
    EXPECT_EQ(ownedDepthRw_->read(&published), ReadStatus::SUCCESS);
    return id;
  }

  arlcore::NumericGuid seedSpeedConditional() {
    SpeedConditionalType payload;
    auto [status, id] = reportProvider_->addSpecialization(&payload);
    EXPECT_EQ(status, SendStatus::SUCCESS);
    SpeedConditionalType published;
    EXPECT_EQ(ownedSpeedRw_->read(&published), ReadStatus::SUCCESS);
    return id;
  }

  ConditionalDeleteCommandType makeCommand(std::optional<arlcore::NumericGuid> conditionalId) {
    ConditionalDeleteCommandType cmd;
    if (conditionalId.has_value()) {
      cmd.conditionalID(conditionalId->getGuid());
    }
    cmd.sessionID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    cmd.destination().id(source_.getGuid());
    return cmd;
  }

  void expectStatus(CommandStatusEnumType status,
      CommandStatusReasonEnumType reason = CommandStatusReasonEnumType::SUCCEEDED) {
    ConditionalDeleteCommandStatusType sample;
    ASSERT_EQ(statusRw_->read(&sample), ReadStatus::SUCCESS);
    EXPECT_EQ(sample.commandStatus(), status);
    EXPECT_EQ(sample.commandStatusReason(), reason);
  }

  arlcore::NumericGuid source_;
  std::shared_ptr<LocalReaderSender<ConditionalReportType>> reportRw_;
  std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> setElementRw_;
  std::shared_ptr<LocalReaderSender<DepthConditionalType>> ownedDepthRw_;
  std::shared_ptr<LocalReaderSender<SpeedConditionalType>> ownedSpeedRw_;
  std::shared_ptr<LocalReaderSender<ConditionalDeleteCommandType>> cmdRw_;
  std::shared_ptr<LocalReaderSender<ConditionalDeleteCommandAckReportType>> ackRw_;
  std::shared_ptr<LocalReaderSender<ConditionalDeleteCommandStatusType>> statusRw_;
  std::shared_ptr<ConditionalDeleteProviderIo> deleteIo_;
  std::shared_ptr<arlcore::umaa::conditional::ConditionalReportProvider> reportProvider_;
};

TEST_F(ConditionalDeleteProviderTest, deleteByIdRemovesConditionalAndDisposesPayload) {
  arlcore::umaa::conditional::ConditionalDeleteProvider provider(source_, deleteIo_, reportProvider_);

  arlcore::NumericGuid depthId = seedDepthConditional();
  arlcore::NumericGuid speedId = seedSpeedConditional();
  std::optional<ConditionalType> depthConditional = reportProvider_->getConditionalById(depthId);
  ASSERT_TRUE(depthConditional.has_value());

  EXPECT_EQ(cmdRw_->send(makeCommand(depthId)), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::COMMANDED);
  expectStatus(CommandStatusEnumType::EXECUTING);
  expectStatus(CommandStatusEnumType::COMPLETED);

  EXPECT_FALSE(reportProvider_->getConditionalById(depthId).has_value());
  EXPECT_TRUE(reportProvider_->getConditionalById(speedId).has_value());

  DepthConditionalType disposed;
  ASSERT_EQ(ownedDepthRw_->read(&disposed), ReadStatus::DISPOSED);
  EXPECT_EQ(arlcore::NumericGuid(disposed.specializationReferenceID()),
    arlcore::NumericGuid(depthConditional->specializationID()));

  ConditionalReportType report;
  EXPECT_EQ(reportRw_->read(&report), ReadStatus::SUCCESS);
}

TEST_F(ConditionalDeleteProviderTest, deleteWithoutIdRemovesEverything) {
  arlcore::umaa::conditional::ConditionalDeleteProvider provider(source_, deleteIo_, reportProvider_);

  seedDepthConditional();
  seedSpeedConditional();
  EXPECT_EQ(reportProvider_->getConditionals().size(), 2);

  EXPECT_EQ(cmdRw_->send(makeCommand(std::nullopt)), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::COMMANDED);
  expectStatus(CommandStatusEnumType::EXECUTING);
  expectStatus(CommandStatusEnumType::COMPLETED);

  EXPECT_TRUE(reportProvider_->getConditionals().empty());

  DepthConditionalType disposedDepth;
  EXPECT_EQ(ownedDepthRw_->read(&disposedDepth), ReadStatus::DISPOSED);
  SpeedConditionalType disposedSpeed;
  EXPECT_EQ(ownedSpeedRw_->read(&disposedSpeed), ReadStatus::DISPOSED);
}

TEST_F(ConditionalDeleteProviderTest, deleteUnknownIdFailsValidation) {
  arlcore::umaa::conditional::ConditionalDeleteProvider provider(source_, deleteIo_, reportProvider_);

  seedDepthConditional();

  EXPECT_EQ(cmdRw_->send(makeCommand(arlcore::UuidFactory::getInstance().generateGuid())), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::FAILED, CommandStatusReasonEnumType::VALIDATION_FAILED);

  EXPECT_EQ(reportProvider_->getConditionals().size(), 1);
}
