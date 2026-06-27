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

#ifndef INCLUDE_NUMERICGUID_H_
#define INCLUDE_NUMERICGUID_H_

#include <array>
#include <ostream>
#include <sstream>
#include <string>

#include "InternalTypes.h"

namespace arlcore {
static const uint32_t GUID_SIZE = 16;

//! \brief Representation of a 128-bit number according to RFC 4122
class NumericGuid {
 public:
  NumericGuid() {
    // Populate guid with 0s
    guid_.fill(0);
  }

  explicit NumericGuid(const std::array<uint8_t, GUID_SIZE> &inputGuid) :
      guid_(inputGuid) {
  }

  virtual ~NumericGuid() = default;
  NumericGuid(const NumericGuid &rhs) = default;

  //! \brief Conversion operator to array to prevent needing to call getGuid all the time
  operator const std::array<uint8_t, GUID_SIZE>() const { return guid_; }

  void setGuid(const std::array<uint8_t, GUID_SIZE> &inputGuid) {
    guid_ = inputGuid;
  }

  const std::array<uint8_t, GUID_SIZE>& getGuid() const {
    return guid_;
  }

  bool isEqual(const std::array<uint8_t, GUID_SIZE> &guidToCompare) const {
    return guid_ == guidToCompare;
  }

  bool isEqual(const NumericGuid &guidToCompare) const {
    return guid_ == guidToCompare.getGuid();
  }

  void operator=(const std::array<uint8_t, GUID_SIZE>& array) {
    guid_ = array;
  }

  bool operator==(const std::array<uint8_t, GUID_SIZE>& guidToCompare) const {
    return guid_ == guidToCompare;
  }

  bool operator==(const NumericGuid& guidToCompare) const {
    return guid_ == guidToCompare.getGuid();
  }

  bool operator!=(const std::array<uint8_t, GUID_SIZE>& guidToCompare) const {
    return guid_ != guidToCompare;
  }

  bool operator!=(const NumericGuid& guidToCompare) const {
    return guid_ != guidToCompare.getGuid();
  }

  bool operator<(const NumericGuid &guidToCompare) const {
    return guid_ < guidToCompare.getGuid();
  }

  bool operator>(const NumericGuid &guidToCompare) const {
    return guid_ > guidToCompare.getGuid();
  }

  bool operator<=(const NumericGuid &guidToCompare) const {
    return guid_ <= guidToCompare.getGuid();
  }

  bool operator>=(const NumericGuid &guidToCompare) const {
    return guid_ >= guidToCompare.getGuid();
  }

  //! \brief Helper function to convert a uint8_t array into a two digit hex string formatted for DDS topic filters
  //! Original UUID: 00112233-4455-6677-8899-AABBCCDDEEFF
  //! example output: "00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF"
  std::string getTwoDigitHexString() const {
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string retString;

    for (uint8_t value : guid_) {
      retString.push_back(hex[value >> 4]);
      retString.push_back(hex[value & 0x0f]);
      retString.push_back(' ');
    }

    retString.pop_back();
    return retString;
  }

 private:
  std::array<uint8_t, GUID_SIZE> guid_;
};

static std::ostream& operator<<(std::ostream& os, const NumericGuid& guidToPrint) {
  os << "[";
  uint8_t count = 0;
  for (uint8_t element : guidToPrint.getGuid()) {
    os << std::to_string(element);
    if (++count < GUID_SIZE) {
      os << "-";
    }
  }
  os << "]";
  return os;
}

static const NumericGuid NIL_GUID;

}  // namespace arlcore

#endif  // INCLUDE_NUMERICGUID_H_
