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

#ifndef INCLUDE_IO_DDS_CYCLONE_CYCLONEBUFFEREDREADER_H_
#define INCLUDE_IO_DDS_CYCLONE_CYCLONEBUFFEREDREADER_H_

#include <stddef.h>

#include <mutex>
#include <string>
#include <thread>
#include <dds/dds.hpp>

#include "Logger.h"
#include "BufferedReaderBase.h"
#include "CycloneUtilities.h"

namespace arlcore::io {

template <class DataType>
class CycloneBufferedReader : public BufferedReaderBase<DataType> {
 public:
  CycloneBufferedReader() = delete;

  //! \brief Standard Constructor for a Cyclone Buffered Reader object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param topic       The string topic name to subscribe to
  //! \param qos         The DataReader qos profile
  //! \param bufferSize  The buffer size. If not specified, a default is used.
  CycloneBufferedReader(const dds::domain::DomainParticipant& participant,
                        const std::string& topic,
                        const dds::sub::qos::DataReaderQos& qos,
                        size_t bufferSize = BufferedReaderBase<DataType>::DEFAULT_BUFFER_SIZE)
      : BufferedReaderBase<DataType>(bufferSize) {
    auto ddsTopic = getTopic<DataType>(participant, topic);
    auto subscriber = createSubscriber(participant);
    reader_ = dds::sub::DataReader<DataType>(subscriber, ddsTopic, qos);
  }

  //! \brief Gets the Health Information for the DataReader.
  //!        Populates the ReaderHealthStats object with data from the underlying DataReader.
  //! \return ReaderHealthStats with DataReader health infomation.
  ReaderHealthStats getHealthInfo() override {
    std::lock_guard<std::mutex> lock(this->readerLock_);
    ReaderHealthStats info;
    info.matchedSenders = reader_.liveliness_changed_status().alive_count();
    info.rejectedSamples = reader_.sample_rejected_status().total_count();
    info.lostSamples = reader_.sample_lost_status().total_count();
    info.missedDeadlines = reader_.requested_deadline_missed_status().total_count();
    info.incompatibleQos = reader_.requested_incompatible_qos_status().total_count();
    info.invalidSamples = invalidSampleCount_;
    info.readSamples = readSampleCount_;
    return info;
  }

 private:
  dds::sub::DataReader<DataType> reader_ = dds::core::null;
  dds::sub::Subscriber subscriber_ = dds::core::null;
  std::mutex readerLock_;
  int32_t invalidSampleCount_ = 0;
  int32_t readSampleCount_ = 0;

  void runReader() override {
    const std::chrono::milliseconds SLEEP_TIMER(10);

    while (this->isThreadRunning_) {
      std::this_thread::sleep_for(SLEEP_TIMER);
      processData();
    }
  }

  void processData() {
    dds::sub::LoanedSamples<DataType> samples = reader_.take();

    for (const auto& sample : samples) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Instance state changed to " << sample.info().state().instance_state())

      SampleEnvelope<DataType> sampleEnvelope;
      sampleEnvelope.status = ReadStatus::SUCCESS;

      auto info = sample.info();
      if (!info.valid()) {
        sampleEnvelope.status = ReadStatus::INVALID_DATA;
        invalidSampleCount_++;
      }

      if (sample.info().state().instance_state() == dds::sub::status::InstanceState::not_alive_disposed()) {
        DataType disposedSample;
        reader_.key_value(disposedSample, info.instance_handle());
        sampleEnvelope.data = disposedSample;
        sampleEnvelope.status = ReadStatus::DISPOSED;
      } else {
        sampleEnvelope.data = sample.data();
      }

      readSampleCount_++;
      this->dataRingBuffer_.push(sampleEnvelope);
    }
  }
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONEBUFFEREDREADER_H_
