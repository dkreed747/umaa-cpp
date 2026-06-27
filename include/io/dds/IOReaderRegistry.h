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

#ifndef INCLUDE_IO_DDS_IOREADERREGISTRY_H_
#define INCLUDE_IO_DDS_IOREADERREGISTRY_H_

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <string>

#include "ReaderBase.h"
#include "StringUtils.h"

using std::type_index;
using std::shared_ptr;
using std::static_pointer_cast;
using std::unordered_map;
using arlcore::StringHash;

namespace arlcore::io {

//! \brief IO Registry designed to hold ReaderBase objects.
class IOReaderRegistry {
 public:
  IOReaderRegistry() = default;

  //! \brief Registers a Topic key and ReaderBase.
  //! \tparam T the data type of the reader.
  //! \param topic Topic key.
  //! \param reader The reader to register.
  template <typename T>
  bool registerReader(const std::string& topic, const shared_ptr<ReaderBase<T>>& reader) {
    return readerMap_.emplace(topic, std::make_pair(type_index(typeid(T)), reader)).second;
  }

  //! \brief Gets a ReaderBase object from a topic.
  //! \tparam T The data type of the reader.
  //! \param topic Topic key.
  //! \return The corresponding reader. If no topic match is found or if the found reader does not match the
  //! templated type of the reader requested, returns a nullptr.
  template <typename T>
  shared_ptr<ReaderBase<T>> getReader(const std::string& topic) {
    if (const auto it = readerMap_.find(topic); it != readerMap_.end() && type_index(typeid(T)) == it->second.first) {
      return static_pointer_cast<ReaderBase<T>>(it->second.second);
    }
    return nullptr;
  }

 private:
  //! \brief Map for storing ReaderBase objects
  unordered_map<std::string, std::pair<std::type_index, shared_ptr<void>>, StringHash> readerMap_;
};
}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_IOREADERREGISTRY_H_
