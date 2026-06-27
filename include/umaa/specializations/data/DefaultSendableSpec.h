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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_DATA_DEFAULTSENDABLESPEC_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_DATA_DEFAULTSENDABLESPEC_H_

#include <memory>
#include <string>

#include "ObjectiveSpecializationUtils.h"
#include "SendableSpecializationBase.h"

using arlcore::io::SenderBase;
using std::shared_ptr;

namespace arlcore::umaa {

//! \brief Default templated data class for storing and sending specialization data.
//! This class is meant for specializations that do not contain messages on other DDS topics.
//! \tparam S The type of specialization stored in this class. Ex. DriftObjectiveType.
//! \param specData The specialization data to store.
//! \param specSender A sender for the specialization data.
template <typename S>
class DefaultSendableSpec : public arlcore::umaa::SendableSpecializationBase {
 public:
  DefaultSendableSpec(S specData, std::shared_ptr<SenderBase<S>> specSender, const std::string& topic)
      : specData_(specData), specSender_(specSender), specTopic_(topic) {}

  DefaultSendableSpec(const DefaultSendableSpec<S>& other)
      : specData_(other.specData_), specSender_(other.specSender_), specTopic_(other.specTopic_) {}

  virtual ~DefaultSendableSpec() {}

  std::shared_ptr<SendableSpecializationBase> with(const NumericGUID& specID,
                                                   const DateTime& timestamp) const override {
    auto specCopy = specData_;
    specCopy.specializationReferenceID(specID);
    specCopy.specializationReferenceTimestamp(timestamp);
    return std::make_shared<DefaultSendableSpec<S>>(specCopy, this->specSender_, this->specTopic_);
  }

  //! \brief OPTIONAL Override to send additional data associated with this class. i.e. Large List
  virtual SendStatus sendDataSpecific() {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called.")
    return SendStatus::NOT_IMPLEMENTED;
  }

  //! \brief OPTIONAL Override to dispose additional data associated with this class. i.e. Large List
  virtual SendStatus disposeDataSpecific() {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called.")
    return SendStatus::NOT_IMPLEMENTED;
  }

  SendStatus send() final {
    if (dataSent) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to send specialization data twice.")
      return SendStatus::ERROR;
    }

    if (sendDataSpecific() == SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run overridden send logic.")
      return SendStatus::ERROR;
    }

    if (auto status = specSender_->send(specData_); status != SendStatus::SUCCESS) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending specialization.")
      return status;
    }
    dataSent = true;
    return SendStatus::SUCCESS;
  }

  SendStatus dispose() final {
    if (!dataSent) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Invalid call of dispose. Specialization data has not been sent.")
      return SendStatus::ERROR;
    }

    if (disposeDataSpecific() == SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run overriden dispose logic.")
      return SendStatus::ERROR;
    }

    if (auto status = specSender_->dispose(specData_); status != SendStatus::SUCCESS) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error disposing specialization.")
      return status;
    }

    dataSent = false;
    return SendStatus::SUCCESS;
  }

  UMAA::Common::Measurement::NumericGUID getSpecializationID() override {
    return specData_.specializationReferenceID();
  }

  DateTime getSpecializationTimestamp() override { return specData_.specializationReferenceTimestamp(); }

  std::string getSpecializationTopic() override { return specTopic_; }

  S getSpecializationData() const { return specData_; }

 protected:
  //! A shared pointer to the Sender of this specific Sendable Specialization.
  shared_ptr<SenderBase<S>> specSender_;
  //! The specialization data stored in this class.
  S specData_;
  //! The topic of the specialization stored in this class.
  std::string specTopic_;

 private:
  //! Boolean that represents whether or not the data has already been sent.
  bool dataSent = false;
};
}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_SPECIALIZATIONS_DATA_DEFAULTSENDABLESPEC_H_
