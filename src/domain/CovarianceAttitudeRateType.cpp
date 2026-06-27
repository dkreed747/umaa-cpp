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

#include "CovarianceAttitudeRateType.h"

namespace arlcore {
  const flt64_t& CovarAttitudeRateType::getPitchPitchAttRateErrCovar() const {
    return rpRp_;
  }
  const flt64_t& CovarAttitudeRateType::getPitchYawAttRateErrCovar() const {
    return rpRy_;
  }
  const flt64_t& CovarAttitudeRateType::getRollPitchAttRateErrCovar() const {
    return rrRp_;
  }
  const flt64_t& CovarAttitudeRateType::getRollRollAttRateErrCovar() const {
    return rrRr_;
  }
  const flt64_t& CovarAttitudeRateType::getRollYawAttRateErrCovar() const {
    return rrRy_;
  }
  const flt64_t& CovarAttitudeRateType::getYawYawAttRateErrCovar() const {
    return ryRy_;
  }

  void CovarAttitudeRateType::setPitchPitchAttRateErrCovar(
    const flt64_t& inPitchPitchRateErr) {
    rpRp_ = inPitchPitchRateErr;
  }

  void CovarAttitudeRateType::setPitchYawAttRateErrCovar(
    const flt64_t& inPitchYawRateErr) {
    rpRy_ = inPitchYawRateErr;
  }

  void CovarAttitudeRateType::setRollPitchAttRateErrCovar(
    const flt64_t& inRollPitchRateErr) {
    rrRp_ = inRollPitchRateErr;
  }

  void CovarAttitudeRateType::setRollRollAttRateErrCovar(
    const flt64_t& inRollRollRateErr) {
    rrRr_ = inRollRollRateErr;
  }

  void CovarAttitudeRateType::setRollYawAttRateErrCovar(
    const flt64_t& inRollYawRateErr) {
    rrRy_ = inRollYawRateErr;
  }

  void CovarAttitudeRateType::setYawYawAttRateErrCovar(
    const flt64_t& inYawYawRateErr) {
    ryRy_ = inYawYawRateErr;
  }

}  // namespace arlcore

