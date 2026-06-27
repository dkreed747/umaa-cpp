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

#include "LocalReaderSender.h"

using arlcore::io::SendStatus;
using arlcore::io::ReadStatus;

class LocalReaderSenderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    io_ = std::make_shared<arlcore::io::LocalReaderSender<std::string>>();
  }

  void TearDown() override {
    io_.reset();
  }

  std::shared_ptr<arlcore::io::LocalReaderSender<std::string>> io_;
};

TEST_F(LocalReaderSenderTest, clear) {
  EXPECT_EQ(io_->send("test"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 1);
  io_->clear();
  EXPECT_EQ(io_->count(), 0);
  std::string buf;
  EXPECT_EQ(io_->read(&buf), ReadStatus::NO_DATA);
}

TEST_F(LocalReaderSenderTest, sendReadMultiple) {
  EXPECT_EQ(io_->send("test1"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send("test2"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 2);
  std::string buf;
  EXPECT_EQ(io_->read(&buf), ReadStatus::SUCCESS);
  EXPECT_EQ(buf, "test1");
  EXPECT_EQ(io_->read(&buf), ReadStatus::SUCCESS);
  EXPECT_EQ(buf, "test2");
  EXPECT_EQ(io_->read(&buf), ReadStatus::NO_DATA);
}

TEST_F(LocalReaderSenderTest, readLatest) {
  EXPECT_EQ(io_->send("test1"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send("test2"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 2);
  std::string buf;
  EXPECT_EQ(io_->readLatest(&buf), ReadStatus::SUCCESS);
  EXPECT_EQ(buf, "test2");
  EXPECT_EQ(io_->count(), 0);
  EXPECT_EQ(io_->read(&buf), ReadStatus::NO_DATA);
}

TEST_F(LocalReaderSenderTest, readUpToN) {
  EXPECT_EQ(io_->send("test1"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send("test2"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send("test3"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 3);
  arlcore::io::SampleEnvelope<std::string> buf[2];
  EXPECT_EQ(io_->readUpToN(buf, 2), 2);
  EXPECT_EQ(buf[0].data, "test1");
  EXPECT_EQ(buf[0].status, ReadStatus::SUCCESS);
  EXPECT_EQ(buf[1].data, "test2");
  EXPECT_EQ(buf[1].status, ReadStatus::SUCCESS);
  EXPECT_EQ(io_->readUpToN(buf, 2), 1);
  EXPECT_EQ(buf[0].data, "test3");
  EXPECT_EQ(buf[0].status, ReadStatus::SUCCESS);
}

TEST_F(LocalReaderSenderTest, dispose) {
  EXPECT_EQ(io_->send("test1"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->dispose("test1"), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 2);
  std::string buf;
  EXPECT_EQ(io_->read(&buf), ReadStatus::SUCCESS);
  EXPECT_EQ(buf, "test1");
  EXPECT_EQ(io_->read(&buf), ReadStatus::DISPOSED);
  EXPECT_EQ(buf, "test1");
  EXPECT_EQ(io_->count(), 0);
}

TEST_F(LocalReaderSenderTest, readInstance) {
  // Given
  std::string samp1 = "test1";
  std::string samp2 = "test2";
  std::string samp3 = "test3";
  EXPECT_EQ(io_->send(samp1), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp2), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp3), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 3);
  // When - read to match samp2
  std::string recvSamp;
  EXPECT_EQ(io_->readInstance(samp2, &recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp2);

  // Then - the remaining items should be in the same order
  EXPECT_EQ(io_->count(), 2);

  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp1);
  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp3);
  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::NO_DATA);
}

TEST_F(LocalReaderSenderTest, readInstanceNoMatch) {
  std::string samp1 = "test1";
  std::string samp2 = "test2";
  std::string samp3 = "test3";
  EXPECT_EQ(io_->send(samp1), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp2), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp3), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 3);
  std::string recvSamp;
  EXPECT_EQ(io_->readInstance("abcd", &recvSamp), ReadStatus::NO_DATA);

  EXPECT_EQ(io_->count(), 3);

  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp1);
  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp2);
  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp3);
  EXPECT_EQ(io_->read(&recvSamp), ReadStatus::NO_DATA);
}

TEST_F(LocalReaderSenderTest, readInstanceDisposed) {
  std::string samp1 = "test1";
  std::string samp2 = "test2";
  std::string samp3 = "test3";
  EXPECT_EQ(io_->send(samp1), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp2), SendStatus::SUCCESS);
  EXPECT_EQ(io_->send(samp3), SendStatus::SUCCESS);
  EXPECT_EQ(io_->dispose(samp3), SendStatus::SUCCESS);
  EXPECT_EQ(io_->count(), 4);
  std::string recvSamp;
  EXPECT_EQ(io_->readInstance(samp3, &recvSamp), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSamp, samp3);
  EXPECT_EQ(io_->readInstance(samp3, &recvSamp), ReadStatus::DISPOSED);
  EXPECT_EQ(recvSamp, samp3);

  EXPECT_EQ(io_->count(), 2);
}