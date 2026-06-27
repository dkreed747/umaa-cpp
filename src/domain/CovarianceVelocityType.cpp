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

#include "CovarianceVelocityType.h"

namespace arlcore {
  const flt64_t& CovarianceVelocityType::getDownDownVelErrCovar() const {
    return vdVd_;
  }
  const flt64_t& CovarianceVelocityType::getEastDownVelErrCovar() const {
    return veVd_;
  }
  const flt64_t& CovarianceVelocityType::getEastEastVelErrCovar() const {
    return veVe_;
  }
  const flt64_t& CovarianceVelocityType::getNorthDownVelErrCovar() const {
    return vnVd_;
  }
  const flt64_t& CovarianceVelocityType::getNorthEastVelErrCovar() const {
    return vnVe_;
  }
  const flt64_t& CovarianceVelocityType::getNorthNorthVelErrCovar() const {
    return vnVn_;
  }

  void CovarianceVelocityType::setDownDownVelErrCovar(
    const flt64_t& inDownDownVelErr) {
    vdVd_ = inDownDownVelErr;
  }

  void CovarianceVelocityType::setEastDownVelErrCovar(
    const flt64_t& inEastDownVelErr) {
    veVd_ = inEastDownVelErr;
  }

  void CovarianceVelocityType::setEastEastVelErrCovar(
    const flt64_t& inEastEastVelErr) {
    veVe_ = inEastEastVelErr;
  }

  void CovarianceVelocityType::setNorthDownVelErrCovar(
    const flt64_t& inNorthDownVelErr) {
    vnVd_ = inNorthDownVelErr;
  }

  void CovarianceVelocityType::setNorthEastVelErrCovar(
    const flt64_t& inNorthEastVelErr) {
    vnVe_ = inNorthEastVelErr;
  }

  void CovarianceVelocityType::setNorthNorthVelErrCovar(
    const flt64_t& inNorthNorthVelErr) {
    vnVn_ = inNorthNorthVelErr;
  }

}  // namespace arlcore
