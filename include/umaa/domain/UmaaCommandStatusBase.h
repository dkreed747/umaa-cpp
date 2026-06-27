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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUSBASE_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUSBASE_H_

#include "GUID.h"

namespace arl {

class UmaaCommandStatusBase {
 public:
  UmaaCommandStatusBase() = default;
  UmaaCommandStatusBase(
      const NumericGUID_t &source,
      const NumericGUID_t &session_id) :
    source_(source),
    session_id_(session_id) {}

  inline NumericGUID_t getSource() const { return source_; }
  inline NumericGUID_t getSessionId() const { return session_id_; }

  inline void setSource(const NumericGUID_t &source) { source_ = source; }
  inline void setSessionId(const NumericGUID_t &session_id) { session_id_ = session_id; }

 private:
  NumericGUID_t source_ = {0};
  NumericGUID_t session_id_ = {0};
};

}  // namespace arl


#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUSBASE_H_
