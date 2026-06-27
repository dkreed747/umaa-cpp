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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEFACTORY_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEFACTORY_H_

#include <memory>
#include <optional>

#include "ObjectiveBase.h"
#include "GlobalPoseReportConsumer.h"
#include "SpeedReportConsumer.h"
#include "VelocityReportConsumer.h"

namespace arlcore::umaa {

class ObjectiveFactory {
 public:
  ObjectiveFactory(
    std::shared_ptr<services::GlobalPoseReportConsumer> gpReportConsumer,
    std::shared_ptr<services::SpeedReportConsumer> sReportConsumer,
    std::shared_ptr<services::VelocityReportConsumer> vReportConsumer) :
    gpReportConsumer_(gpReportConsumer),
    sReportConsumer_(sReportConsumer),
    vReportConsumer_(vReportConsumer) {}

  ObjectiveFactory() {}
  virtual ~ObjectiveFactory() {}

  //! \brief Pure virtual method to build a concrete ObjectiveBase
  //! The builder function may fail to build an objective and can return a nullopt
  //! \param obj The umaa base objective type used to create the specialized objective
  //! \return optional value that contains a pointer to the new objective
  virtual std::optional<std::shared_ptr<ObjectiveBase>> build(const ObjectiveType& obj) const = 0;

 protected:
  const std::shared_ptr<services::GlobalPoseReportConsumer> gpReportConsumer_;
  const std::shared_ptr<services::SpeedReportConsumer> sReportConsumer_;
  const std::shared_ptr<services::VelocityReportConsumer> vReportConsumer_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEFACTORY_H_
