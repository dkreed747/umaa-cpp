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
#ifndef INCLUDE_IO_UDP_UDPTHREAD_H_
#define INCLUDE_IO_UDP_UDPTHREAD_H_

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>

#include "DataRingBuffer.h"

#include "Udp.h"

namespace arlcore::io {

const uint32_t DEFAULT_POLL_RATE_MS = 5;
const uint32_t DEFAULT_BUFFER_SIZE = 100;

//! \brief Maintains a thread for sending and receiving UDP messages
class UdpThread {
 public:
  //! \brief Construct a new Udp Thread object
  //! \param pollRateMs - The rate to poll the input socket at in milliseconds
  //! \param bufferSize - The size of the ring buffer.
  UdpThread(uint32_t pollRateMs = DEFAULT_POLL_RATE_MS,
            uint32_t bufferSize = DEFAULT_BUFFER_SIZE);

  //! \brief Destroy the UDP Thread object
  ~UdpThread();

  //! \brief Check if the thread is currently running
  //! \return Boolean that is true if the thread is running
  bool isRunning() const {
    return isRunning_;
  }

  //! \brief Check if the thread is configured to be a reader
  //! \return Boolean that is true if the thread configured to read UDP messages
  bool isReader() const {
    return inUdpReady_;
  }

  //! \brief Check if the thread is configured to be a writer
  //! \return Boolean that is true if the thread configured to write UDP messages
  bool isWriter() const {
    return outUdpReady_;
  }

  //! \brief Set up connections, initialize variables for reader
  //! \param options UDP options type for instantiation
  //! \return Boolean that is true if successful
  bool initUdpThreadReader(const UdpOptions& options);

  //! \brief Set up connections, initialize variables for writer
  //! \param options UDP options type for instantiation
  //! \return Boolean that is true if successful
  bool initUdpThreadWriter(const UdpOptions& options);

  //! \brief Starts UDP processing thread
  //! \return Boolean that is true if thread starts successfully
  bool startUdpThread();

  //! \brief Stops UDP processing thread
  //! \return Boolean that is true if thread stops successfully
  bool stopUdpThread();

  //! \brief Issue a command to be transmitted when resources are available
  //! \param input
  void addUdpCommand(const UMSG& input);

  //! \brief Issue a command to be transmitted immediately
  //! \param input
  void writeUdpCommand(const UMSG& input);

  //! \brief Check flag if there is new data to pull
  //! \return Boolean that is set to true when there is new data to grab
  bool checkStatData();

  //! \brief Get Status data
  //! Get latest data and reset the data flag to false
  //! \return UMSG
  UMSG returnStatData();

  //! \brief Worker thread listens for serial data and makes it available
  //! Accepts Serial commands and issues them to HW.
  void runUdpConnection();

 private:
  std::atomic<bool> isRunning_ = ATOMIC_VAR_INIT(false);

  std::chrono::milliseconds pollRateMs_;

  UMSG listenBuffer_;

  UdpOptions inUdpOptions_;
  int inUdpSock_ = -1;
  struct sockaddr_in inAddr_;
  unsigned int inAddrLen_ = 0;
  std::atomic<bool> inUdpReady_ = ATOMIC_VAR_INIT(false);

  UdpOptions outUdpOptions_;
  int outUdpSock_ = -1;
  struct sockaddr_in outAddr_;
  unsigned int outAddrLen_ = 0;
  std::atomic<bool> outUdpReady_ = ATOMIC_VAR_INIT(false);

  //! \brief Structure to accept the incoming data from HW
  arlcore::DataRingBuffer<UMSG> myStatData_;

  //! \brief A secondary buffer - we will double buffer data so we can export it in a thread-safe fashion
  UMSG myExternalStatData_;

  //! \brief Structure to accept the outgoing commands to HW
  arlcore::DataRingBuffer<UMSG> myCmdData_;

  //! \brief A secondary buffer - we will double buffer data so we can export it in a thread-safe fashion
  UMSG myExternalCmdData_;

  //! \brief The actual thread to do the work
  std::shared_ptr<std::thread> udpThread_;

  //! \brief input protection mutex
  std::mutex inputProtectionMutex_;

  //! \brief output protection mutex
  std::mutex outputProtectionMutex_;
};
}  // namespace arlcore::io
#endif  // INCLUDE_IO_UDP_UDPTHREAD_H_
