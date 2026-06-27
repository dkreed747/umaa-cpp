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

#ifndef INCLUDE_IO_DDS_BASE_BUFFEREDREADERBASE_H_
#define INCLUDE_IO_DDS_BASE_BUFFEREDREADERBASE_H_

#include <inttypes.h>
#include <stddef.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include <mutex>

#include "DdsIoTypes.h"
#include "ReaderBase.h"
#include "DataRingBuffer.h"

namespace arlcore::io {
template <class DataType>
class BufferedReaderBase : public ReaderBase<DataType> {
 public:
  static const uint32_t DEFAULT_BUFFER_SIZE = 2048;
  BufferedReaderBase() = delete;

  //! \brief Constructor
  //! \param bufferSize size of internal buffer
  explicit BufferedReaderBase(size_t bufferSize = DEFAULT_BUFFER_SIZE) : dataRingBuffer_(bufferSize) {}

  virtual ~BufferedReaderBase() = default;

  //! \brief Take the next sample from the internal buffer
  //! \param[inout] outSample A buffer to store the sample that is taken
  //! \return ReadStatus enum of the retrieved sample
  ReadStatus read(DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return ReadStatus::ERROR;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    if (dataRingBuffer_.isEmpty()) {
      return ReadStatus::NO_DATA;
    }

    SampleEnvelope<DataType> sampleEnvelope = dataRingBuffer_.get();
    *outSample = sampleEnvelope.data;
    return sampleEnvelope.status;
  }

  //! \brief Read the latest sample from the data queue
  //! \param outSample the latest sample
  //! \return Readstatus enum
  ReadStatus readLatest(DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return ReadStatus::ERROR;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    if (dataRingBuffer_.isEmpty()) {
      return ReadStatus::NO_DATA;
    }

    // Get the latest sample and clear list
    SampleEnvelope<DataType> sampleEnvelope = dataRingBuffer_.back();
    *outSample = sampleEnvelope.data;

    dataRingBuffer_.clear();

    return sampleEnvelope.status;
  }

  //! \brief Take all samples up to \ref "n" from the internal buffer
  //! \param[inout] outSamples A buffer to store the samples that are taken
  //! \param n The length of the output buffer
  //! \return The number of samples taken from the internal buffer
  size_t readUpToN(SampleEnvelope<DataType>* outSamples, const uint32_t& n) override {
    if (outSamples == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return 0;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    if (dataRingBuffer_.isEmpty()) {
      return 0;
    }

    uint32_t samplesToRead = std::min(dataRingBuffer_.size(), n);
    for (uint32_t i = 0; i < samplesToRead; i++) {
      outSamples[i] = dataRingBuffer_.get();
    }

    return static_cast<size_t>(samplesToRead);
  }

  //! \brief Check if the reader thread is currently running
  //! \return Whether or not the reader thread is currently running
  bool isRunning() const { return isThreadRunning_; }

  bool startReaderThread() {
    if (isThreadRunning_) {
      return isThreadRunning_;
    }

    isThreadRunning_ = true;
    dataThread_ = std::thread(&BufferedReaderBase<DataType>::runReader, this);
    return isThreadRunning_;
  }

  bool stopReaderThread() {
    if (!isThreadRunning_) {
      return true;
    }

    if (dataThread_.joinable()) {
      isThreadRunning_ = false;
      dataThread_.join();
      return true;
    }

    return false;
  }

  //! \brief Get the number of samples stored in the internal buffer
  //! \return The number of samples in the internal buffer
  size_t size() {
    std::lock_guard<std::mutex> lock(readerLock_);
    return dataRingBuffer_.size();
  }

 protected:
  std::mutex readerLock_;
  std::thread dataThread_;

  DataRingBuffer<SampleEnvelope<DataType>> dataRingBuffer_;
  std::atomic<bool> isThreadRunning_ = ATOMIC_VAR_INIT(false);

  virtual void runReader() = 0;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_BUFFEREDREADERBASE_H_
