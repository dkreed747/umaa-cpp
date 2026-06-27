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

#ifndef INCLUDE_UMAA_SERVICES_BASE_REPORTCONSUMER_H_
#define INCLUDE_UMAA_SERVICES_BASE_REPORTCONSUMER_H_

#include <memory>
#include <optional>

#include "Logger.h"
#include "NumericGuid.h"
#include "ReaderBase.h"
#include "Subject.h"

using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;

namespace arlcore::umaa::services {

//! \brief Report Consumer template class
//! Getting reports from specific source IDs is handled at the DDS content filter level
//! \tparam ReportType
template <class ReportType>
class ReportConsumer {
 public:
  //! \brief Constructor for the report consumer obj
  //! \param reportReader A constant pointer to an io reader
  explicit ReportConsumer(std::shared_ptr<ReaderBase<ReportType>> reportReader) :
    reportReader_(reportReader) {}

  //! \brief Read all reports currently available from the DDS bus and return only the newest one
  //! \return ReadStatus enum that provides the outcome of the read operation
  ReadStatus read(ReportType *outReport) { return reportReader_->readLatest(outReport); }

  //! \brief Cycle the ReportConsumer to check for new reports
  //! \return Whether the ReportConsumer was able to check for new reports without errors
  ReadStatus cycle() {
    ReportType data;
    ReadStatus status = reportReader_->readLatest(&data);
    if (status == ReadStatus::SUCCESS) {
      latest_ = data;
      reportSubject_.notify(latest_.value());
    }
    return status;
  }

  //! \brief Get a copy of the latest report object if it exists
  //! \return The optional latest Report
  std::optional<ReportType> getReport() const {
    return reportSubject_.getLatestNotification();
  }

  Subject<ReportType>& getReportSubject() {
    return reportSubject_;
  }

 private:
  std::shared_ptr<ReaderBase<ReportType>> reportReader_;
  std::optional<ReportType> latest_;
  Subject<ReportType> reportSubject_ = Subject<ReportType>(true);
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_REPORTCONSUMER_H_
