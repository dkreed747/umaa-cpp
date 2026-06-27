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

#include "CovarianceOrientationType.h"

namespace arlcore {
  const flt64_t& CovarianceOrientationType::getPitchPitchAngleErrCovar() const {
    return rpRp_;
  }
  const flt64_t& CovarianceOrientationType::getPitchYawAngleErrCovar() const {
    return rpRy_;
  }
  const flt64_t& CovarianceOrientationType::getRollPitchAngleErrCovar() const {
    return rrRp_;
  }
  const flt64_t& CovarianceOrientationType::getRollRollAngleErrCovar() const {
    return rrRr_;
  }
  const flt64_t& CovarianceOrientationType::getRollYawAngleErrCovar() const {
    return rrRy_;
  }
  const flt64_t& CovarianceOrientationType::getYawYawAngleErrCovar() const {
    return ryRy_;
  }

  void CovarianceOrientationType::setPitchPitchAngleErrCovar(
    const flt64_t& inPitchPitchAngleErr) {
    rpRp_ = inPitchPitchAngleErr;
  }

  void CovarianceOrientationType::setPitchYawAngleErrCovar(
    const flt64_t& inPitchYawAngleErr) {
    rpRy_ = inPitchYawAngleErr;
  }

  void CovarianceOrientationType::setRollPitchAngleErrCovar(
    const flt64_t& inRollPitchAngleErr) {
    rrRp_ = inRollPitchAngleErr;
  }

  void CovarianceOrientationType::setRollRollAngleErrCovar(
    const flt64_t& inRollRollAngleErr) {
    rrRr_ = inRollRollAngleErr;
  }

  void CovarianceOrientationType::setRollYawAngleErrCovar(
    const flt64_t& inRollYawAngleErr) {
    rrRy_ = inRollYawAngleErr;
  }

  void CovarianceOrientationType::setYawYawAngleErrCovar(
    const flt64_t& inYawYawAngleErr) {
    ryRy_ = inYawYawAngleErr;
  }

}  // namespace arlcore
