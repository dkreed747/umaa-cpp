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

#ifndef INCLUDE_UMAA_SPECIALIZATIONCACHE_H_
#define INCLUDE_UMAA_SPECIALIZATIONCACHE_H_

#include <map>
#include <memory>

#include <UMAA/Common/Measurement/Measurements.hpp>

#include "NumericGuid.h"
#include "ReaderBase.h"

using arlcore::io::ReadStatus;

namespace arlcore::umaa {

//! \brief A utility to cache all samples of a UMAA specialization type and be able to obtain them based on their ID
//! \tparam Specialization The UMAA specialized type
template <class Specialization>
class SpecializationCache {
 public:
  //! \brief Constructor
  //! \param reader A shared pointer to the reader that reads the UMAA specialized type
  explicit SpecializationCache(std::shared_ptr<arlcore::io::ReaderBase<Specialization>> reader) : reader_(reader) {}

  //! \brief Handle any incoming samples of the UMAA specialized type
  void update() {
    Specialization buffer;
    ReadStatus status;
    status = reader_->read(&buffer);
    while (status != ReadStatus::NO_DATA && status != ReadStatus::ERROR) {
      arlcore::NumericGuid id(buffer.specializationReferenceID());
      if (status == ReadStatus::DISPOSED) {
        if (lookup_.erase(id) != 1) {
          UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to handle disposal of specialized type with unregistered ID")
        }
      } else {
        lookup_.insert_or_assign(id, buffer);
      }
      status = reader_->read(&buffer);
    }
  }

  //! \brief Retrieve a UMAA specialization from its generalized counterpart
  //! \tparam Generalization The Generalized UMAA type
  //! \param base The UMAA Generalized type
  //! \return A UMAA specialization object or nullopt if not found
  template <class Generalization>
  std::optional<Specialization> getSpecialization(const Generalization &base) {
    update();
    if (auto it = lookup_.find(arlcore::NumericGuid(base.specializationID())); it != lookup_.end() &&
        base.specializationTimestamp() == it->second.specializationReferenceTimestamp()) {
      return it->second;
    }

    return std::nullopt;
  }

  //! \brief Retrieve a UMAA specialization object by its specialization reference ID if it exists
  //! \param id The specializationID of the UMAA specialized type
  //! \param timestamp An optional timestamp to validate the specialization against. If not provided, the latest sample
  //! is returned
  //! \return A UMAA specialization object or nullopt if not found/timestamp mismatch
  std::optional<Specialization> getSpecializationByRefId(const arlcore::NumericGuid &id,
      std::optional<UMAA::Common::Measurement::DateTime> timestamp = std::nullopt) {
    update();
    if (auto it = lookup_.find(id); it != lookup_.end() &&
        (!timestamp.has_value() || it->second.specializationReferenceTimestamp() == timestamp.value())) {
      return it->second;
    }

    return std::nullopt;
  }

 private:
  std::shared_ptr<arlcore::io::ReaderBase<Specialization>> reader_;
  std::map<arlcore::NumericGuid, Specialization> lookup_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_SPECIALIZATIONCACHE_H_
