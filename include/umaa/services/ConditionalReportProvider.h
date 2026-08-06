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

#ifndef INCLUDE_UMAA_SERVICES_CONDITIONALREPORTPROVIDER_H_
#define INCLUDE_UMAA_SERVICES_CONDITIONALREPORTPROVIDER_H_

#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <UMAA/MM/Conditional/ConditionalType.hpp>

#include "LargeSetWriter.h"
#include "ReportProvider.h"
#include "ConditionalReportProviderIo.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;

using ConditionalReportProviderBase =
  arlcore::umaa::services::ReportProvider<ConditionalReportType>;
using ConditionalLargeSetWriterBase =
  arlcore::umaa::LargeSetWriter<ConditionalType, ConditionalReportTypeConditionalsSetElement>;

//! \brief A class to manage UMAA Conditional Reports and create internal representations of the UMAA conditional
//!        objects that can be evaluated
class ConditionalReportProvider : private ConditionalReportProviderBase, private ConditionalLargeSetWriterBase {
 public:
  //! \brief Constructor
  //! \param sourceId The source ID to sign all report samples with
  //! \param io Shared pointer to the object containing the group of readers and writers required
  //! \param parentId The platform this conditional service runs on, stamped as source.parentID
  ConditionalReportProvider(const NumericGuid& sourceId, std::shared_ptr<ConditionalReportProviderIo> io,
                            const NumericGuid& parentId = NumericGuid());

  //! \brief Send the current working conditional report to consumers
  //! \return Status from sending the report
  SendStatus sendReport();

  //! \brief Get the collection of conditionals in the working conditional report
  //! \return A vector of ConditionalTypes representing the conditionals in the report
  std::vector<ConditionalType> getConditionals() const;

  //! \brief Get the ConditionalType from the working conditional report corresponding to the given conditionalID
  //! \param id The ConditionalID of the ConditionalType to be retrieved
  //! \return An optional of ConditionalType populated if the ID exists
  std::optional<ConditionalType> getConditionalById(const NumericGuid &id) const;

  //! \brief An internal function to get topic information and the corresponding writer of a specialized conditional
  //! given its type using template specialization
  //! \tparam Specialized The UMAA specialized conditional type
  //! \param cond The specialized conditional to get information about
  //! \return A pair of the SpecializationTopic and pointer to the writer for the specialized type
  template <class Specialized>
  std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<Specialized>>> getTopicAndWriter(Specialized cond) {
    // If no specialization exists
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to get information about specialized conditional of unknown type")
    return std::make_pair("INVALID", std::shared_ptr<arlcore::io::SenderBase<Specialized>>());
  }

  //! \brief Dispose the specialized conditional corresponding to the given ConditionalType.
  //! (This function should not need to be called directly)
  //! \param cond The ConditionalType whose corresponding specialization should be disposed
  //! \return The status resulting from the disposal
  SendStatus disposeConditionalSpecialization(ConditionalType cond);

  //! \brief Add a conditional to the working ConditionalReport
  //! \param conditional The conditional to be added
  //! \return The status of the add operation
  SendStatus addConditional(const ConditionalType& conditional);

  //! \brief Add a new conditional corresponding to the provided specialized conditional to the working
  //! ConditionalReport and update the metadata of the provided specialized conditional
  //! \tparam Specialized The UMAA specialized conditional type
  //! \param[inout] specialization A pointer specialized conditional to add to the report and whose metadata will be
  //! updated with the new specializationTimestamp and (optionally) specializationID
  //! \param generateSpecializationId Whether to generate a new specializationID for the given conditional
  //! (default true)
  //! \return A pair of the status of writing the specialized conditional and the new conditionalID of the
  //! ConditionalType added to the working report
  template <class Specialized>
  std::pair<SendStatus, NumericGuid> addSpecialization(Specialized *specialization,
      bool generateSpecializationId = true) {
    auto [topic, writer] = getTopicAndWriter(*specialization);
    if (topic == "INVALID") {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to add conditional of invalid type")
      return std::make_pair(SendStatus::NOT_IMPLEMENTED, NIL_GUID);
    }

    if (generateSpecializationId) {
      specialization->specializationReferenceID(UuidFactory::getInstance().generateGuid());
    }

    ConditionalType generic;
    generic.conditionalID(UuidFactory::getInstance().generateGuid());
    generic.specializationTopic(topic);
    generic.specializationID(specialization->specializationReferenceID());

    DateTime sync = getTimestamp();
    specialization->specializationReferenceTimestamp(sync);
    generic.specializationTimestamp(sync);

    SendStatus result = writer->send(*specialization);

    if (result != SendStatus::SUCCESS) {
      return std::make_pair(result, NumericGuid(generic.conditionalID()));
    }

    result = this->insert(generic);
    return std::make_pair(result, NumericGuid(generic.conditionalID()));
  }

  //! \brief Update an existing conditional to a new value with the same conditionalID
  //! \param conditional The updated conditional
  //! \return The status of updating the conditional
  SendStatus updateConditional(const ConditionalType& conditional);

  //! \brief Update an existing conditional from the conditional report with a new specialized conditional
  //! \tparam Specialized Specialized The UMAA specialized conditional type
  //! \param conditionalId The conditionalID of the conditional to be updated
  //! \param[inout] specialization A pointer specialized conditional to update the conditional to and whose metadata
  //! will be updated with the new specializationTimestamp and (optionally) specializationID
  //! \param generateSpecializationId Whether to generate a new specializationID for the given conditional
  //! (default false)
  //! \return The status of writing the specialized conditional
  template <class Specialized>
  SendStatus updateSpecialization(const NumericGuid &conditionalId, Specialized *specialization,
      bool generateSpecializationId = false) {
    if (generateSpecializationId) {
      specialization->specializationReferenceID(UuidFactory::getInstance().generateGuid());
    }

    std::optional<ConditionalType> existing = getConditionalById(conditionalId);
    if (!existing.has_value()) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to update conditional with unknown ID")
      return SendStatus::ERROR;
    }

    auto [topic, writer] = getTopicAndWriter(*specialization);

    // If we change the specialization ID or type, dispose the old instance
    if (generateSpecializationId || specialization->specializationReferenceID() != existing->specializationID()
        || existing->specializationTopic() != topic) {
      if (disposeConditionalSpecialization(existing.value()) != SendStatus::SUCCESS) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Failed to dispose old specialization while updating conditional")
      }
    }

    ConditionalType updated = existing.value();
    updated.specializationID(specialization->specializationReferenceID());
    updated.specializationTopic(topic);

    DateTime sync = getTimestamp();
    specialization->specializationReferenceTimestamp(sync);
    updated.specializationTimestamp(sync);

    SendStatus result = writer->send(*specialization);
    if (result != SendStatus::SUCCESS) {
      return result;
    }

    return this->update(existing.value(), updated);
  }

  //! \brief Remove a conditional from the working conditional report
  //! \param conditional The conditional to remove
  //! \return The status of removing the conditional
  SendStatus removeConditional(const ConditionalType& conditional);

  //! \brief Remove a conditional from the working conditional report based on its conditionalID and dispose
  //! its specialization
  //! \param id The conditionalID of the conditional to remove from the report
  //! \return The status of disposing the corresponding specialized conditional
  SendStatus removeSpecialization(const NumericGuid &id);

 private:
  NumericGuid sourceId_;
  std::shared_ptr<ConditionalReportProviderIo> io_;
};

// Explicit specializations of getTopicAndWriter (defined in
// ConditionalReportProvider.cpp) for every supported specialized conditional.
// They must be declared here: a translation unit that uses one of these types
// without seeing its declaration would silently instantiate the "unknown type"
// primary template instead.
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::ConstraintViolatedConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::ConstraintViolatedConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::DepthConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::DepthConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::DepthRateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::DepthRateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::EmitterPresetConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::EmitterPresetConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::ExpConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::ExpConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::HeadingSectorConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::HeadingSectorConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::LogicalANDConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::LogicalANDConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::LogicalNOTConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::LogicalNOTConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::LogicalORConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::LogicalORConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::MissionStateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::MissionStateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::ObjectiveStateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::ObjectiveStateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::PitchRateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::PitchRateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::RelativeSpeedConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::RelativeSpeedConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::RollRateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::RollRateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::SpeedConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::SpeedConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::TaskStateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::TaskStateConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::TimeConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::TimeConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::WaterZoneConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::WaterZoneConditionalType cond);
template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<UMAA::MM::Conditional::YawRateConditionalType>>>
ConditionalReportProvider::getTopicAndWriter(UMAA::MM::Conditional::YawRateConditionalType cond);

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_SERVICES_CONDITIONALREPORTPROVIDER_H_
