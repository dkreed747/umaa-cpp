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

#ifndef INCLUDE_UMAA_SERVICES_CONDITIONALREPORTCONSUMER_H_
#define INCLUDE_UMAA_SERVICES_CONDITIONALREPORTCONSUMER_H_

#include <vector>
#include <memory>
#include <optional>

#include "LargeSetReader.h"
#include "ReportConsumer.h"
#include "ConditionalFactory.h"
#include "Subject.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;

using ConditionalReportConsumerBase =
  arlcore::umaa::services::ReportConsumer<ConditionalReportType>;
using ConditionalLargeSetReaderBase =
  arlcore::umaa::LargeSetReader<ConditionalType, ConditionalReportTypeConditionalsSetElement>;

//! \brief A class to manage UMAA Conditional Reports and create internal representations of the UMAA conditional
//!        objects that can be evaluated
class ConditionalReportConsumer : public ConditionalReportConsumerBase, private ConditionalLargeSetReaderBase,
  public Subject<std::vector<std::shared_ptr<ConditionalBase>>> {
 public:
  //! \brief Constructor
  //! @param reportReader A reader for the UMAA conditional report type
  //! @param reportSetElementReader A reader for the UMAA conditional set element type for reading the Large Set
  //! @param factory A ConditionalFactory to use to create conditional objects that can be evaluated
  ConditionalReportConsumer(std::shared_ptr<arlcore::io::ReaderBase<ConditionalReportType>> reportReader,
    std::shared_ptr<arlcore::io::ReaderBase<ConditionalReportTypeConditionalsSetElement>> reportSetElementReader,
    std::shared_ptr<ConditionalFactory> factory);

  //! \brief Read in the latest conditional report (if available) and retrieve its conditionals
  //! \return Whether the operation was a success
  bool cycle();

  //! \brief Get the current list of existing conditionals
  //! \return An optional vector of shared pointers to ConditionalBase-derived objects
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> getConditionals();

 private:
  std::shared_ptr<ConditionalFactory> factory_;
  std::optional<ConditionalReportType> currentReport_ = std::nullopt;
  std::optional<std::vector<ConditionalType>> conditionals_ = std::nullopt;
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> conditionalObjects_ = std::nullopt;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_SERVICES_CONDITIONALREPORTCONSUMER_H_
