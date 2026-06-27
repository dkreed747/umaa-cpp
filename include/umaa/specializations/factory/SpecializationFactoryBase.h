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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_SPECIALIZATIONFACTORYBASE_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_SPECIALIZATIONFACTORYBASE_H_

#include <memory>

#include "DefaultSendableSpec.h"
#include "IOReaderRegistry.h"
#include "IOWriterRegistry.h"
#include "Logger.h"
#include "SendableSpecializationBase.h"

using arlcore::io::IOReaderRegistry;
using arlcore::io::IOWriterRegistry;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SenderBase;
using arlcore::io::SendStatus;
using std::make_shared;
using std::shared_ptr;

namespace arlcore::umaa {

//! \brief A Specialization Factory Utility class that can be derived to read specializations off the DDS bus, given a
//! generalization type.
//! \tparam G The Generalization Type. Ex. ObjectiveType
template <typename G>
class SpecializationFactoryBase {
 public:
  SpecializationFactoryBase() = delete;

  virtual ~SpecializationFactoryBase() = default;
  //! \brief Constructor for the factory. This factory can be constructed with as many readers/writers as needed.
  //! For example, if you want to use this factory class as a util to read and create only HoverObjectiveTypes,
  //! you only need HoverObjectiveType readers and writers.
  //! \param readerReg An IOReaderRegistry with necessary readers.
  //! \param writerReg An IOWriterRegistry with necessary writers.
  SpecializationFactoryBase(const shared_ptr<IOReaderRegistry>& readerReg,
                            const shared_ptr<IOWriterRegistry>& writerReg)
      : readerRegistry_(readerReg), writerRegistry_(writerReg) {}

  //! \brief Given an Generalization Type, reads the specialization off the DDS bus and creates a
  //! sendable specialization object.
  //! \param genType The generalization Type. Ex. ObjectiveType
  //! \return A sendable specialization. If the Factory does not have the required readers/writers to perform this
  //! operation, this function will return a null ptr. Callers of this function are responsible for validating the
  //! return.
  virtual shared_ptr<SendableSpecializationBase> createSpecialization(const G& genType) = 0;

 protected:
  //! \brief Utility function for reading off the DDS bus and creating a sendable specialization.
  //! \tparam S The specialization type. Ex. DriftObjectiveType
  //! \param genType Then generalization Type. Ex. ObjectiveType
  //! \return A sendable specialization or a nullptr if unable to create one.
  template <typename S>
  shared_ptr<SendableSpecializationBase> makeSpecialization(const G& genType) {
    auto reader = readerRegistry_->getReader<S>(genType.specializationTopic());
    auto writer = writerRegistry_->getWriter<S>(genType.specializationTopic());
    // Check that necessary reader and writer exists.
    if (!reader || !writer) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Required reader or writer not found for " << genType.specializationTopic())
      return nullptr;
    }

    auto readSpec = readSpecialization<S>(genType, reader);
    return readSpec ? make_shared<DefaultSendableSpec<S>>(readSpec.value(), writer, genType.specializationTopic())
                    : nullptr;
  }

  //! \brief Utility function for reading specializations off the DDS bus.
  //! \tparam  S The specialization type. Ex. DriftObjectiveType
  //! \param obj The ObjectiveType
  //! \param reader The reader for the specialization type
  //! \return An optional specialization type read from the bus. NullOpt if not found.
  template <typename S>
  std::optional<S> readSpecialization(const G& genType, shared_ptr<ReaderBase<S>> reader) {
    S key;
    key.specializationReferenceID(genType.specializationID());
    // Timestamp is not a keyed value, but LocalReaderSender unit tests depend on this being set to read instances.
    key.specializationReferenceTimestamp(genType.specializationTimestamp());

    S dataSample;
    auto printable = arlcore::NumericGuid(genType.specializationID());
    do {
      switch (reader->readInstance(key, &dataSample)) {
        case ReadStatus::SUCCESS:
          break;
          // Check if specID and timestamp are the same, if timestamp is different, keep reading until no data.
        case ReadStatus::DISPOSED:
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
                         "Specialization with Topic: " << genType.specializationTopic() << ", ID: ["
                                                       << printable.getTwoDigitHexString() << "] has been disposed")
          return std::nullopt;
        case ReadStatus::NO_DATA:
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
                         "No Specialization Data found for Topic: "
                             << genType.specializationTopic() << ", ID: [" << printable.getTwoDigitHexString() << "]"
                             << ", Timestamp: " << genType.specializationTimestamp())
          return std::nullopt;
        default:
          return std::nullopt;
      }
    } while (genType.specializationID() != dataSample.specializationReferenceID() &&
             genType.specializationTimestamp() != dataSample.specializationReferenceTimestamp());
    // We're here if we found a sample that matches the specializationID and Timestamp
    return std::make_optional<S>(dataSample);
  }

  shared_ptr<IOReaderRegistry> readerRegistry_;
  shared_ptr<IOWriterRegistry> writerRegistry_;
};
}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_SPECIALIZATIONFACTORYBASE_H_
