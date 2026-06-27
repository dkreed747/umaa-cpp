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

#ifndef INCLUDE_IO_DDS_BASE_OBSERVABLEREADER_H_
#define INCLUDE_IO_DDS_BASE_OBSERVABLEREADER_H_

#include <memory>
#include <mutex>

#include "ReaderBase.h"
#include "Subject.h"

namespace arlcore::io {

//! \brief A class that polls a reader and triggers an update to any observers when new data comes in
//! \tparam T The type being read by the reader and being sent to the observers
template <class T>
class ObservableReader : public arlcore::Subject<SampleEnvelope<T>> {
 public:
  //! \brief Constructor
  //! \param reader The reader to poll for updates
  explicit ObservableReader(std::shared_ptr<ReaderBase<T>> reader) : reader_(reader) {}


  //! \brief Read the next incoming sample and notify observers accordingly
  //! \return The ReadStatus returned by the Reader
  ReadStatus read() {
    SampleEnvelope<T> envelope;
    envelope.status = reader_->read(&(envelope.data));
    switch (envelope.status) {
      case ReadStatus::SUCCESS:
        latest_ = envelope.data;
        // Intentional fallthrough
      case ReadStatus::DISPOSED:
        this->notify(envelope);
        break;
      default:
        break;
    }
    return envelope.status;
  }

  //! \brief Read the latest sample and notify observers accordingly
  //! \return The ReadStatus returned by the Reader
  ReadStatus readLatest() {
    SampleEnvelope<T> envelope;
    envelope.status = reader_->readLatest(&(envelope.data));
    switch (envelope.status) {
      case ReadStatus::SUCCESS:
        latest_ = envelope.data;
        // Intentional fallthrough
      case ReadStatus::DISPOSED:
        this->notify(envelope);
        break;
      default:
        break;
    }
    return envelope.status;
  }

 private:
  std::shared_ptr<ReaderBase<T>> reader_;
  T latest_;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_OBSERVABLEREADER_H_
