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

#include "ObservableReader.h"
#include "LocalReaderSender.h"

class ObservableReaderTest : public ::testing::Test {
 protected:
  class TestObserver : public arlcore::Observer<arlcore::io::SampleEnvelope<int32_t>> {
   public:
    TestObserver() = default;

    void update(const arlcore::io::SampleEnvelope<int32_t>& data) override {
      latest = data.data;
    }

    int32_t latest = 0;
  };

  void SetUp() override {
    io_ = std::make_shared<arlcore::io::LocalReaderSender<int32_t>>();
    observable_ = std::make_shared<arlcore::io::ObservableReader<int32_t>>(io_);
    observer_ = std::make_shared<TestObserver>();
    observable_->registerObserver(observer_);
  }

  void TearDown() override {
    io_.reset();
    observable_.reset();
    observer_.reset();
  }

  std::shared_ptr<arlcore::io::LocalReaderSender<int32_t>> io_;
  std::shared_ptr<arlcore::io::ObservableReader<int32_t>> observable_;
  std::shared_ptr<TestObserver> observer_;
};

TEST_F(ObservableReaderTest, testObserver) {
  int32_t value = 19;
  io_->send(value);
  EXPECT_EQ(observable_->readLatest(), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(observer_->latest, value);
  value = 20;
  io_->send(value);
  EXPECT_EQ(observable_->readLatest(), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(observer_->latest, value);
}

