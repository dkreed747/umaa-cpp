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

#ifndef INCLUDE_IO_DDS_CYCLONE_CYCLONEREADER_H_
#define INCLUDE_IO_DDS_CYCLONE_CYCLONEREADER_H_

#include <memory>
#include <mutex>
#include <string>

#include <dds/dds.hpp>

#include "ReaderBase.h"
#include "CycloneUtilities.h"
#include "CycloneQosProviderWrapper.h"

namespace arlcore::io {


template <class DataType>
class CycloneReader : public ReaderBase<DataType> {
 public:
  //! \brief Standard Constructor for a Cyclone Reader object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param topic       The string topic name to subscribe to
  //! \param qos         The DataReader qos profile
  CycloneReader(const dds::domain::DomainParticipant& participant,
            const std::string& topic,
            const dds::sub::qos::DataReaderQos& qos) {
    auto ddsTopic = getTopic<DataType>(participant, topic);
    reader_ = dds::sub::DataReader<DataType>(createSubscriber(participant), ddsTopic, qos);
  }

  //! \brief Qos-Configurable Constructor for Cyclone Reader object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param subscriber  The DDS subscriber
  //! \param topic       The string topic name to subscribe to
  //! \param readerQos   The DataReader qos profile
  //! \param topicQos    The Topic qos profile
  CycloneReader(const dds::domain::DomainParticipant& participant,
            const dds::sub::Subscriber& subscriber,
            const std::string& topic,
            const dds::sub::qos::DataReaderQos& readerQos,
            const dds::topic::qos::TopicQos& topicQos) {
    auto ddsTopic = getTopic<DataType>(participant, topic, topicQos);
    reader_ = dds::sub::DataReader<DataType>(subscriber, ddsTopic, readerQos);
  }

  CycloneReader(const dds::domain::DomainParticipant& participant,
            const dds::sub::Subscriber& subscriber,
            const std::string& topic,
            const std::shared_ptr<arlcore::io::CycloneQosProviderWrapper>& wrapper,
            const std::string& profile = "") {
    auto ddsTopic = getTopic<DataType>(participant, topic, wrapper->topic_qos(profile));
    reader_ = dds::sub::DataReader<DataType>(subscriber, ddsTopic, wrapper->datareader_qos(profile));
  }

  //! \brief Reads and points outSample to the sample read.
  //!        If a content filter has been specified, points outSample
  //!        to the first sample matching the filter.
  // Implements a read function using CycloneDDS
  //! \param outSample Returned next sample variable
  //! \return          A ReadStatus Enum indicating the results of the read function
  ReadStatus read(DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null!");
      return ReadStatus::ERROR;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    auto status = ReadStatus::NO_DATA;
    // Takes only one sample

    dds::sub::LoanedSamples<DataType> samples;
    try {
      samples = reader_.select().max_samples(1).take();
    } catch(const std::exception& e) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, e.what())
      return ReadStatus::ERROR;
    }

    if (samples.length() == 0) {
      // ReadStatus::NO_DATA
      return status;
    }

    const auto& sample = *(samples.begin());
    DataType sampleData;
    dds::sub::status::InstanceState instanceState = sample.info().state().instance_state();
    bool instanceAlive = instanceState == dds::sub::status::InstanceState::alive();

    if (instanceAlive || sample.info().valid()) {
      sampleData = sample.data();
    } else {
      reader_.key_value(sampleData, sample.info().instance_handle());
    }

    if (std::shared_ptr<ReaderFilter<DataType>> filter = this->manualFilter.lock()) {
      if (!filter->filter(sampleData)) {
        readerLock_.unlock();
        return read(outSample);
      }
    }

    if (instanceAlive || sample.info().valid()) {
      *outSample = sampleData;
      status = ReadStatus::SUCCESS;
    } else if (instanceState == dds::sub::status::InstanceState::not_alive_disposed()) {
      *outSample = sampleData;
      status = ReadStatus::DISPOSED;
    } else if (instanceState == dds::sub::status::InstanceState::not_alive_no_writers()) {
      *outSample = sampleData;
      status = ReadStatus::SUCCESS;
    } else {
      invalidSampleCount_++;
      status = ReadStatus::INVALID_DATA;
    }
    readSampleCount_++;
    return status;
  }

  //! \brief Reads and points outSample to the latest sample read.
  //!        If a content filter has been specified, points outSample
  //!        to the latest sample matching the filter.
  //! \param outSample Returned latest sample variable
  //! \return          A ReadStatus Enum indicating the results of the read function
  ReadStatus readLatest(DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null.")
      return ReadStatus::ERROR;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    auto status = ReadStatus::NO_DATA;
    dds::sub::LoanedSamples<DataType> samples = reader_.take();
    if (samples.length() == 0) {
      return status;
    }

    dds::sub::SampleRef<DataType> latestSample;
    DataType sampleData;
    dds::sub::status::InstanceState instanceState;
    bool hasSample = false;

    // If filter is specified, begin filtering data. Otherwise, use the latest sample.
    if (std::shared_ptr<ReaderFilter<DataType>> filter = this->manualFilter.lock()) {
      // Start on the latest element
      for (auto sampleIter = std::prev(samples.end()); sampleIter >= samples.begin(); --sampleIter) {
        latestSample = *sampleIter;
        auto &dataToFilter = latestSample.data();
        // Found a sample matching the filter
        if (filter -> filter(dataToFilter)) {
          hasSample = true;
          break;
        }
      }
      if (!hasSample) {
        // We've read all the samples but found no matching data.
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "No samples matching the filter were read.");
        return status;  // ReadStatus::NO_DATA
      }
    } else {
      latestSample = *std::prev(samples.end());
    }

    instanceState = latestSample.info().state().instance_state();
    if (instanceState == dds::sub::status::InstanceState::alive()) {
      sampleData = latestSample.data();
    } else {
      reader_.key_value(sampleData, latestSample.info().instance_handle());
    }

    if (instanceState == dds::sub::status::InstanceState::alive()) {
      *outSample = sampleData;
      status = ReadStatus::SUCCESS;
    } else if (instanceState == dds::sub::status::InstanceState::not_alive_disposed()) {
      *outSample = sampleData;
      status = ReadStatus::DISPOSED;
    } else {
      invalidSampleCount_++;
      status = ReadStatus::INVALID_DATA;
    }
    readSampleCount_++;
    return status;
  }

  //! \brief Reads up to N samples and stores in a SampleEnvelope to be set in outSample.
  //! \param outSamples Returned sample envelopes variable
  //! \param n          Max number of samples to write to the outSamples buffer
  //! \return The number of samples read from the bus. Will be equal to min(NUM_SAMPLES_ON_BUS, \param n)
  size_t readUpToN(SampleEnvelope<DataType>* outSamples, const u_int32_t& n) override {
    if (outSamples == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null.")
      return 0;
    }

    uint32_t samplesRead = 0;
    DataType data;
    ReadStatus status = ReadStatus::NO_DATA;

    while (samplesRead < n) {
      status = read(&data);
      if (status == ReadStatus::NO_DATA) {
        break;
      }

      SampleEnvelope<DataType> sampleEnvelope;
      if (status != ReadStatus::INVALID_DATA) {
        sampleEnvelope.data = data;
      }

      sampleEnvelope.status = status;
      outSamples[samplesRead] = sampleEnvelope;
      samplesRead++;
    }
    return samplesRead;
  }

  //! \brief Reads a DDS keyed instance.
  //! Currently, this is being used to read specializations.
  //! \param key The key to match.
  //! \param outSample The location to store the returned sample - if present.
  //! \return The status of the read operation.
  ReadStatus readInstance(const DataType& key, DataType *outSample) override {
    if (outSample == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outSample pointer is null.")
      return ReadStatus::ERROR;
    }

    std::lock_guard<std::mutex> lock(readerLock_);
    dds::sub::LoanedSamples<DataType> samples;
    try {
      auto ih = reader_.lookup_instance(key);
      samples = reader_.select().max_samples(1).instance(ih).take();
    } catch (const std::exception& e) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, e.what())
      return ReadStatus::ERROR;
    }

    if (samples.length() == 0) {
      return ReadStatus::NO_DATA;
    }

    const auto& sample = *(samples.begin());
    DataType sampleData;
    dds::sub::status::InstanceState instanceState = sample.info().state().instance_state();
    bool instanceAlive = instanceState == dds::sub::status::InstanceState::alive();

    if (instanceAlive || sample.info().valid()) {
      sampleData = sample.data();
    } else {
      // Only sets the keyed fields in sampleData
      reader_.key_value(sampleData, sample.info().instance_handle());
    }

    if (std::shared_ptr<ReaderFilter<DataType>> filter = this->manualFilter.lock()) {
      if (!filter->filter(sampleData)) {
        readerLock_.unlock();
        return readInstance(sampleData, outSample);
      }
    }

    auto status = ReadStatus::NO_DATA;
    if (instanceAlive || sample.info().valid()) {
      *outSample = sampleData;
      status = ReadStatus::SUCCESS;
    } else if (instanceState == dds::sub::status::InstanceState::not_alive_disposed() ||
                instanceState == dds::sub::status::InstanceState::not_alive_no_writers()) {
      *outSample = sampleData;
      status = ReadStatus::DISPOSED;
    } else {
      invalidSampleCount_++;
      status = ReadStatus::INVALID_DATA;
    }
    readSampleCount_++;
    return status;
  }

  //! \brief Gets the Health Information for the DataReader.
  //!        Populates the ReaderHealthStats object with data from the underlying DataReader.
  //! \return ReaderHealthStats with DataReader health information.
  ReaderHealthStats getHealthInfo() override {
    std::lock_guard<std::mutex> lock(readerLock_);
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
  std::mutex readerLock_;
  int32_t invalidSampleCount_ = 0;
  int32_t readSampleCount_ = 0;
};
}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONEREADER_H_
