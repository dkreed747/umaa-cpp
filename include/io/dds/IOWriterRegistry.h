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

#ifndef INCLUDE_IO_DDS_IOWRITERREGISTRY_H_
#define INCLUDE_IO_DDS_IOWRITERREGISTRY_H_

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <string>

#include "SenderBase.h"
#include "StringUtils.h"

using std::type_index;
using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::unordered_map;
using arlcore::StringHash;

namespace arlcore::io {

//! \brief IO Writer Registry for Senders implementing SenderBase.
class IOWriterRegistry {
 public:
  IOWriterRegistry() = default;

  //! \brief Registers a Topic key and SenderBase.
  //! \tparam T The data type of the writer.
  //! \param topic Topic key.
  //! \param writer The sender to register.
  template <typename T>
  bool registerWriter(const std::string& topic, const shared_ptr<SenderBase<T>>& writer) {
    return writerMap_.emplace(topic, std::make_pair(type_index(typeid(T)), writer)).second;
  }

  //! \brief Gets a SenderBase object from a topic.
  //! \tparam T the data type of the the writer.
  //! \param topic Topic key.
  //! \return The corresponding sender. If no topic match is found or if the found sender does not match the
  //! templated type of the sender requested, returns a nullptr.
  template <typename T>
  shared_ptr<SenderBase<T>> getWriter(const std::string& topic) {
    // Find and check type
    if (const auto it = writerMap_.find(topic); it != writerMap_.end() && type_index(typeid(T)) == it->second.first) {
      return static_pointer_cast<SenderBase<T>>(it->second.second);
    }
    return nullptr;
  }

 private:
  //! \brief Map for storing SenderBase objects.
  unordered_map<std::string, std::pair<std::type_index, shared_ptr<void>>, StringHash> writerMap_;
};
}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_IOWRITERREGISTRY_H_
