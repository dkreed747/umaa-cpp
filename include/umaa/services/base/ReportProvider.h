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

#ifndef INCLUDE_UMAA_SERVICES_BASE_REPORTPROVIDER_H_
#define INCLUDE_UMAA_SERVICES_BASE_REPORTPROVIDER_H_

#include "memory"

#include "Logger.h"
#include "NumericGuid.h"
#include "SenderBase.h"
#include "UmaaUtils.h"

using arlcore::io::SendStatus;
using arlcore::io::SenderBase;

namespace arlcore::umaa::services {

template <class ReportType>
class ReportProvider {
 public:
  //! \brief Constructor for ReportProvider
  //! \param sourceId the source ID to sign all samples with
  //! \param reportSender A shared pointer with a concrete implementation of the sender base class
  //! \param parentId the platform this service runs on, stamped as source.parentID
  //!
  //! parentID identifies the parent entity of the reporting service. Consumers use it to
  //! attribute a subsystem's reports to the platform carrying it; leaving it unset makes
  //! every report from a service look like it belongs to no platform at all. It defaults
  //! to the nil GUID so existing callers keep compiling, but a service that knows its
  //! platform should pass it.
  ReportProvider(const NumericGuid& sourceId,
                 std::shared_ptr<SenderBase<ReportType>> reportSender,
                 const NumericGuid& parentId = NumericGuid()) :
                 sourceId_(sourceId), parentId_(parentId), reportSender_(reportSender) {}

  //! \brief Destructor ensures reports are disposed after provider is torn down per UMAA 5.2.1.3
  ~ReportProvider() {
    // Dispose on destruction. source is @key, so parentID is part of the instance
    // identity -- omitting it here would dispose a DIFFERENT instance than the one
    // send() has been writing, leaving the real instance alive forever.
    ReportType disposeSample;
    disposeSample.source().id(sourceId_.getGuid());
    disposeSample.source().parentID(parentId_.getGuid());
    SendStatus stat = reportSender_->dispose(disposeSample);
    if (stat == SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to dispose status report instance on shutdown")
    } else {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Successfully disposed report instance while shutting down")
    }
  }

  //! \brief Send a report and set the source ID to the provider's source ID
  //! \param report a pointer to the report to send
  //! \return a SendStatus enum
  SendStatus send(ReportType* report) {
    if (report == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "report pointer is null!")
      return SendStatus::ERROR;
    }

    report->source().id(sourceId_.getGuid());
    report->source().parentID(parentId_.getGuid());
    report->timeStamp(arlcore::umaa::getTimestamp());
    return reportSender_->send(*report);
  }

 private:
  const NumericGuid sourceId_;
  const NumericGuid parentId_;
  std::shared_ptr<SenderBase<ReportType>> reportSender_;
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_REPORTPROVIDER_H_
