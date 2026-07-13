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

#ifndef TEST_IO_DDS_UTIL_LOCALREADERSENDER_H_
#define TEST_IO_DDS_UTIL_LOCALREADERSENDER_H_

#include <algorithm>

#include "ReaderBase.h"
#include "SenderBase.h"
#include "DataRingBuffer.h"
#include "DdsIoTypes.h"

namespace arlcore::io {

//! \brief An implementation of the Reader and Sender interfaces that stores data locally in an internal buffer rather
//!        than sending it over the DDS bus. Useful unit tests.
//! \tparam DataType The type of the data to read/send
template <class DataType>
class LocalReaderSender : public ReaderBase<DataType>, public SenderBase<DataType> {
 public:
  static const uint32_t DEFAULT_BUFFER_SIZE = 2048;
  //! \brief Constructor
  LocalReaderSender() : dataRingBuffer_(DEFAULT_BUFFER_SIZE) {}

  //! \brief Write a sample to the internal buffer
  //! \param data the data to write to the internal buffer
  //! \return Always returns SendStatus::SUCCESS
  SendStatus send(const DataType& data) override {
    SampleEnvelope<DataType> e;
    e.data = data;
    e.status = ReadStatus::SUCCESS;
    dataRingBuffer_.push(e);
    return SendStatus::SUCCESS;
  }

  //! \brief Write a sample to the internal buffer that when read has a ReadStatus of DISPOSED
  //! \param data the data representing an instance being disposed
  //! \return Always returns SendStatus::SUCCESS
  SendStatus dispose(const DataType& data) override {
    SampleEnvelope<DataType> e;
    e.data = data;
    e.status = ReadStatus::DISPOSED;
    dataRingBuffer_.push(e);
    return SendStatus::SUCCESS;
  }

  //! \brief Pop the next sample from the internal buffer
  //! \param outSample The location to store the sample output
  //! \return ReadStatus of the operation
  ReadStatus read(DataType* outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return ReadStatus::ERROR;
    }

    if (dataRingBuffer_.isEmpty()) {
      return ReadStatus::NO_DATA;
    }

    SampleEnvelope<DataType> e = dataRingBuffer_.get();
    *outSample = e.data;
    return e.status;
  }

  //! \brief Read only the latest sample, discarding the rest
  //! \param outSample The location to store the sample output
  //! \return ReadStatus of the operation
  ReadStatus readLatest(DataType* outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return ReadStatus::ERROR;
    }

    if (dataRingBuffer_.isEmpty()) {
      return ReadStatus::NO_DATA;
    }

    // Get the latest sample and clear list
    SampleEnvelope<DataType> e = dataRingBuffer_.back();
    *outSample = e.data;

    dataRingBuffer_.clear();

    return e.status;
  }

  //! \brief Read up to N samples from the internal buffer
  //! \param outSamples The location to store SampleEnvelopes containing the samples read
  //! \param n The maximum number of samples to store in the output buffer
  //! \return The number of samples read in to the output buffer
  size_t readUpToN(SampleEnvelope<DataType>* outSamples, const uint32_t& n) override {
    if (outSamples == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return 0;
    }

    if (dataRingBuffer_.isEmpty()) {
      return 0;
    }

    uint32_t samplesToRead = std::min(dataRingBuffer_.size(), n);
    for (uint32_t i = 0; i < samplesToRead; i++) {
      outSamples[i] = dataRingBuffer_.get();
    }

    return static_cast<size_t>(samplesToRead);
  }

  //! \brief Reads and returns the sample matching the key exactly.
  //!  Unlike DDS messages, LocalReaderSender has no concept of "keyed" attributes,
  //!  so given key must exactly match the sample to be read.
  //! \param key The key to match.
  //! \param outSample The location to store the returned sample - if present.
  //! \return The status of the read operation.
  ReadStatus readInstance(const DataType& key, DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!")
      return ReadStatus::ERROR;
    }

    if (dataRingBuffer_.isEmpty()) {
      return ReadStatus::NO_DATA;
    }

    std::optional<SampleEnvelope<DataType>> matchingItem;
    auto foundIdx = 0;
    int bufSize = count();

    // Get items in the queue until one is found that matches the key
    for (auto i = 0; i < bufSize; ++i) {
      auto sample = dataRingBuffer_.get();
      if (sample.data == key) {
        matchingItem = sample;
        break;
      } else {
        dataRingBuffer_.push(sample);
        foundIdx++;
      }
    }

    // Restore remaining items in buffer if match was found
    for (int i = 0; i < bufSize - (matchingItem.has_value() ? foundIdx + 1 : bufSize); ++i) {
        auto sample = dataRingBuffer_.get();
        dataRingBuffer_.push(sample);
    }

    if (matchingItem) {
      *outSample = matchingItem.value().data;
      return matchingItem.value().status;
    } else {
      return ReadStatus::NO_DATA;
    }
  }

  //! \brief Clear the internal buffer discarding all pending samples
  void clear() {
    dataRingBuffer_.clear();
  }

  //! \brief Get the number of pending samples in the buffer that can be read
  //! \return The number of samples in the internal buffer
  uint32_t count() {
    return dataRingBuffer_.size();
  }

 private:
  DataRingBuffer<SampleEnvelope<DataType>> dataRingBuffer_;
};

}  // namespace arlcore::io
#endif  // TEST_IO_DDS_UTIL_LOCALREADERSENDER_H_
