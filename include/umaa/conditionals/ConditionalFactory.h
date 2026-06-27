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

#ifndef INCLUDE_UMAA_CONDITIONALS_CONDITIONALFACTORY_H_
#define INCLUDE_UMAA_CONDITIONALS_CONDITIONALFACTORY_H_

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <queue>
#include <utility>
#include <vector>

#include "ConditionalBase.h"
#include "ConstraintViolatedConditional.h"
#include "DepthConditional.h"
#include "DepthRateConditional.h"
#include "HeadingSectorConditional.h"
#include "LogicalANDConditional.h"
#include "LogicalNOTConditional.h"
#include "LogicalORConditional.h"
#include "PitchRateConditional.h"
#include "RelativeSpeedConditional.h"
#include "RollRateConditional.h"
#include "SpeedConditional.h"
#include "TimeConditional.h"
#include "WaterZoneConditional.h"
#include "YawRateConditional.h"

#include "Logger.h"
#include "NumericGuid.h"
#include "ConditionalFactoryIo.h"


namespace arlcore::umaa::conditional {

//! \brief An object that facilitates the creation of conditional objects from UMAA types
class ConditionalFactory {
 public:
  //! \brief Constructor
  //! \param io Shared pointer to the object containing the group of readers and writers required
  explicit ConditionalFactory(std::shared_ptr<ConditionalFactoryIo> io) : io_(io) {}

  //! \brief Function to determine if the specialization topic of the UMAA conditional references other conditionals
  //! \param conditional The UMAA conditional to check
  //! \return Whether the conditional will reference other conditionals
  static bool hasDependencies(const ConditionalType& conditional);

  //! \brief Generate a vector of shared pointers to conditionals from a vector of UMAA conditionals
  //! \param generalizedConditionals A vector of UMAA conditionals (prior to specialization)
  //! \return A vector of shared pointers to conditionals derived from ConditionalBase, nullopt on failure
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> createConditionals(
      std::vector<ConditionalType> generalizedConditionals);

  //! \brief Determine if a conditional with a given ID has any circular dependencies
  //! \param node The ID of the conditional to check for circular dependencies
  //! \param nodes A map of conditional IDs to their corresponding objects
  //! \return Whether there are any circular dependencies
  static bool hasConflict(NumericGUID node, std::shared_ptr<std::map<NumericGUID,
                          std::shared_ptr<ConditionalBase>>> nodes);

  //! \brief Create a conditional from a UMAA type and specialization
  //! \tparam Derived The class derived from ConditionalBase
  //! \tparam Specialization The specialized UMAA type
  //! \param base The UMAA conditional
  //! \param specialized The specialized UMAA type
  //! \param topic The topic corresponding to the UMAA specialization
  //! \return A shared pointer to a new Derived conditional
  template <class Derived, class Specialization>
  std::optional<std::shared_ptr<Derived>> createConditional(const ConditionalType& base,
                                                            const Specialization& specialized,
                                                            const std::string& topic);

  //! \brief Create a ConstraintViolatedConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new ConstraintViolatedConditional or nullopt on failure
  std::optional<std::shared_ptr<ConstraintViolatedConditional>> createConstraintViolatedConditional(
      const ConditionalType& base);

  //! \brief Create a DepthConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new DepthConditional or nullopt on failure
  std::optional<std::shared_ptr<DepthConditional>> createDepthConditional(const ConditionalType& base);

  //! \brief Create a DepthRateConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new DepthRateConditional or nullopt on failure
  std::optional<std::shared_ptr<DepthRateConditional>> createDepthRateConditional(const ConditionalType& base);

  //! \brief Create a HeadingSectorConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new HeadingSectorConditional or nullopt on failure
  std::optional<std::shared_ptr<HeadingSectorConditional>> createHeadingSectorConditional(const ConditionalType& base);

  //! \brief Create a LogicalANDConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new LogicalANDConditional or nullopt on failure
  std::optional<std::shared_ptr<LogicalANDConditional>> createLogicalANDConditional(const ConditionalType& base);

  //! \brief Create a LogicalNOTConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new LogicalNOTConditional or nullopt on failure
  std::optional<std::shared_ptr<LogicalNOTConditional>> createLogicalNOTConditional(const ConditionalType& base);

  //! \brief Create a LogicalORConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new LogicalORConditional or nullopt on failure
  std::optional<std::shared_ptr<LogicalORConditional>> createLogicalORConditional(const ConditionalType& base);

  //! \brief Create a PitchRateConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new PitchRateConditional or nullopt on failure
  std::optional<std::shared_ptr<PitchRateConditional>> createPitchRateConditional(const ConditionalType& base);

  //! \brief Create a RelativeSpeedConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new RelativeSpeedConditional or nullopt on failure
  std::optional<std::shared_ptr<RelativeSpeedConditional>> createRelativeSpeedConditional(const ConditionalType& base);

  //! \brief Create a RollRateConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new RollRateConditional or nullopt on failure
  std::optional<std::shared_ptr<RollRateConditional>> createRollRateConditional(const ConditionalType& base);

  //! \brief Create a SpeedConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new SpeedConditional or nullopt on failure
  std::optional<std::shared_ptr<SpeedConditional>> createSpeedConditional(const ConditionalType& base);

  //! \brief Create a TimeConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new TimeConditional or nullopt on failure
  std::optional<std::shared_ptr<TimeConditional>> createTimeConditional(const ConditionalType& base);

  //! \brief Create a WaterZoneConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new WaterZoneConditional or nullopt on failure
  std::optional<std::shared_ptr<WaterZoneConditional>> createWaterZoneConditional(const ConditionalType& base);

  //! \brief Create a YawRateConditional from a base UMAA conditional
  //! \param base The UMAA conditional object
  //! \return A shared pointer to a new YawRateConditional or nullopt on failure
  std::optional<std::shared_ptr<YawRateConditional>> createYawRateConditional(const ConditionalType& base);

 private:
  std::shared_ptr<ConditionalFactoryIo> io_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_CONDITIONALFACTORY_H_
