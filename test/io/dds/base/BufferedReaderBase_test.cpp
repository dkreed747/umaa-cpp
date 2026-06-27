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

#include "BufferedReaderBase.h"

#include <gtest/gtest.h>
#include <unistd.h>

using arlcore::io::BufferedReaderBase;
using arlcore::io::SampleEnvelope;
using arlcore::io::ReadStatus;
using arlcore::io::ReaderHealthStats;

template <class T>
class TestBufferedReader : public BufferedReaderBase<T> {
 public:
  TestBufferedReader() : BufferedReaderBase<T>(2048) {}

  void addSample(const SampleEnvelope<T> &sample) { this->dataRingBuffer_.push(sample); }

  ReaderHealthStats getHealthInfo() override {
    ReaderHealthStats info = {0};
    return info;
  }

  void runReader() override {
    while (this->isThreadRunning_) {
      usleep(20000);
    }
  }
};

TEST(BufferedReaderBase, startStopThread) {
  TestBufferedReader<std::string> reader;
  EXPECT_TRUE(reader.startReaderThread());
  // Returns true if already running
  EXPECT_TRUE(reader.startReaderThread());

  EXPECT_TRUE(reader.isRunning());
  EXPECT_TRUE(reader.stopReaderThread());
  // Returns true if thread already stopped
  EXPECT_TRUE(reader.stopReaderThread());

  EXPECT_FALSE(reader.isRunning());
}

TEST(BufferedReaderBase, read) {
  TestBufferedReader<std::string> reader;
  SampleEnvelope<std::string> samples_in[3];

  // Create test samples
  samples_in[0].data = "hello";
  samples_in[0].status = ReadStatus::SUCCESS;
  samples_in[1].data = "world";
  samples_in[1].status = ReadStatus::DISPOSED;
  samples_in[2].data = "!";
  samples_in[2].status = ReadStatus::INVALID_DATA;

  // Add samples to internal buffer
  EXPECT_EQ(reader.size(), (size_t)0);
  reader.addSample(samples_in[0]);
  reader.addSample(samples_in[1]);
  reader.addSample(samples_in[2]);

  // Check size is correct
  EXPECT_EQ(reader.size(), 3);
  SampleEnvelope<std::string> samples_out[5];

  // Read first sample in queue and check size has reduced by one
  EXPECT_EQ(reader.read(&samples_out[0].data), ReadStatus::SUCCESS);
  EXPECT_EQ(reader.size(), 2);
  EXPECT_EQ(samples_in[0].data, samples_out[0].data);

  // Read up to 4 samples and confirm all remaining samples have been read
  EXPECT_EQ(reader.readUpToN(samples_out + 1, 4), 2);
  EXPECT_EQ(reader.size(), 0);
  EXPECT_EQ(samples_in[1].data, samples_out[1].data);
  EXPECT_EQ(samples_out[1].status, samples_in[1].status);
  EXPECT_EQ(samples_in[2].data, samples_out[2].data);
  EXPECT_EQ(samples_out[2].status, samples_in[2].status);

  // Add the samples to the buffer again and check that read latest takes the "freshest" sample
  reader.addSample(samples_in[0]);
  reader.addSample(samples_in[1]);
  reader.addSample(samples_in[2]);
  EXPECT_EQ(reader.size(), 3);
  EXPECT_EQ(reader.readLatest(&samples_out[4].data), samples_in[2].status);
  EXPECT_EQ(samples_out[4].data, samples_in[2].data);

  // Check that reader functions return NO data when nothing is in the buffer
  EXPECT_EQ(reader.read(&samples_out[3].data), ReadStatus::NO_DATA);
  EXPECT_EQ(reader.readLatest(&samples_out[3].data), ReadStatus::NO_DATA);
  EXPECT_EQ(reader.readUpToN(samples_out + 3, 2), 0);
}
