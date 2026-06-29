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

#ifndef APPS_AUTOPILOT_INCLUDE_AUTOPILOT_NAVSTATE_H_
#define APPS_AUTOPILOT_INCLUDE_AUTOPILOT_NAVSTATE_H_

#include <mutex>
#include <optional>

#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>
#include <UMAA/SA/SpeedStatus/SpeedReportType.hpp>
#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>

namespace arlcore::autopilot {

//! \brief Thread-safe snapshot of the latest navigation data from the three SA services.
//! The autopilot brain reads from this on every control tick. Writes happen on the nav
//! observer callbacks; getters return copies so callers never hold the lock.
class NavState {
 public:
  void setPose(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose) {
    std::lock_guard<std::mutex> lock(mtx_);
    pose_ = pose;
  }
  void setSpeed(const UMAA::SA::SpeedStatus::SpeedReportType& speed) {
    std::lock_guard<std::mutex> lock(mtx_);
    speed_ = speed;
  }
  void setVelocity(const UMAA::SA::VelocityStatus::VelocityReportType& velocity) {
    std::lock_guard<std::mutex> lock(mtx_);
    velocity_ = velocity;
  }

  std::optional<UMAA::SA::GlobalPoseStatus::GlobalPoseReportType> pose() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return pose_;
  }
  std::optional<UMAA::SA::SpeedStatus::SpeedReportType> speed() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return speed_;
  }
  std::optional<UMAA::SA::VelocityStatus::VelocityReportType> velocity() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return velocity_;
  }

  bool hasPose() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return pose_.has_value();
  }

  //! \brief Current ground speed if reported, else 0.
  double groundSpeedMps() const {
    std::lock_guard<std::mutex> lock(mtx_);
    if (speed_.has_value() && speed_->speedOverGround().has_value()) {
      return speed_->speedOverGround().value();
    }
    return 0.0;
  }

 private:
  mutable std::mutex mtx_;
  std::optional<UMAA::SA::GlobalPoseStatus::GlobalPoseReportType> pose_;
  std::optional<UMAA::SA::SpeedStatus::SpeedReportType> speed_;
  std::optional<UMAA::SA::VelocityStatus::VelocityReportType> velocity_;
};

}  // namespace arlcore::autopilot
#endif  // APPS_AUTOPILOT_INCLUDE_AUTOPILOT_NAVSTATE_H_
