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

#ifndef INCLUDE_DOMAIN_COVARIANCEPOSITIONNEDTYPE_H_
#define INCLUDE_DOMAIN_COVARIANCEPOSITIONNEDTYPE_H_

#include "InternalTypes.h"

namespace arlcore {

class CovariancePositionNEDType {
 public:
  CovariancePositionNEDType() = default;
  virtual ~CovariancePositionNEDType() = default;

  const flt64_t& getDownDownPosErrCovar() const;
  const flt64_t& getEastDownPosErrCovar() const;
  const flt64_t& getEastEastPosErrCovar() const;
  const flt64_t& getNorthDownPosErrCovar() const;
  const flt64_t& getNorthEastPosErrCovar() const;
  const flt64_t& getNorthNorthPosErrCovar() const;

  void setDownDownPosErrCovar(const flt64_t& inDownDownPosErr);
  void setEastDownPosErrCovar(const flt64_t& inEastDownPosErr);
  void setEastEastPosErrCovar(const flt64_t& inEastEastPosErr);
  void setNorthDownPosErrCovar(const flt64_t& inNorthDownPosErr);
  void setNorthEastPosErrCovar(const flt64_t& inNorthEastPosErr);
  void setNorthNorthPosErrCovar(const flt64_t& inNorthNorthPosErr);

 private:
  flt64_t pdPd_ = 0.0;  // Down-Down position error covariance.
                        // Units are Meters squared.
  flt64_t pePd_ = 0.0;  // East-Down position error covariance.
                        // Units are Meters squared.
  flt64_t pePe_ = 0.0;  // East-East position error covariance.
                        // Units are Meters squared.
  flt64_t pnPd_ = 0.0;  // North-Down position error covariance.
                        // Units are Meters squared.
  flt64_t pnPe_ = 0.0;  // North-East position error covariance.
                        // Units are Meters squared.
  flt64_t pnPn_ = 0.0;  // North-North position error covariance.
                        // Units are Meters squared.
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_COVARIANCEPOSITIONNEDTYPE_H_
