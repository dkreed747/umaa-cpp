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

#include "CovariancePositionNEDType.h"

namespace arlcore {
  const flt64_t& CovariancePositionNEDType::getDownDownPosErrCovar() const {
    return pdPd_;
  }
  const flt64_t& CovariancePositionNEDType::getEastDownPosErrCovar() const {
    return pePd_;
  }
  const flt64_t& CovariancePositionNEDType::getEastEastPosErrCovar() const {
    return pePe_;
  }
  const flt64_t& CovariancePositionNEDType::getNorthDownPosErrCovar() const {
    return pnPd_;
  }
  const flt64_t& CovariancePositionNEDType::getNorthEastPosErrCovar() const {
    return pnPe_;
  }
  const flt64_t& CovariancePositionNEDType::getNorthNorthPosErrCovar() const {
    return pnPn_;
  }

  void CovariancePositionNEDType::setDownDownPosErrCovar(
    const flt64_t& inDownDownPosErr) {
    pdPd_ = inDownDownPosErr;
  }

  void CovariancePositionNEDType::setEastDownPosErrCovar(
    const flt64_t& inEastDownPosErr) {
    pePd_ = inEastDownPosErr;
  }

  void CovariancePositionNEDType::setEastEastPosErrCovar(
    const flt64_t& inEastEastPosErr) {
    pePe_ = inEastEastPosErr;
  }

  void CovariancePositionNEDType::setNorthDownPosErrCovar(
    const flt64_t& inNorthDownPosErr) {
    pnPd_ = inNorthDownPosErr;
  }

  void CovariancePositionNEDType::setNorthEastPosErrCovar(
    const flt64_t& inNorthEastPosErr) {
    pnPe_ = inNorthEastPosErr;
  }

  void CovariancePositionNEDType::setNorthNorthPosErrCovar(
    const flt64_t& inNorthNorthPosErr) {
    pnPn_ = inNorthNorthPosErr;
  }

}  // namespace arlcore
