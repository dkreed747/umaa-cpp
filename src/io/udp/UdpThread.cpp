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
#include "UdpThread.h"
#include "Logger.h"

#include <cstring>

namespace arlcore::io {

UdpThread::UdpThread(uint32_t pollRateMs, uint32_t bufferSize) :
    pollRateMs_(pollRateMs),
    myStatData_(bufferSize),
    myCmdData_(bufferSize) {
  listenBuffer_.len = 0;
}

UdpThread::~UdpThread() {
  this->stopUdpThread();

  if (this->inUdpSock_ != -1) {
    close(inUdpSock_);
  }

  if (this->outUdpSock_ != -1) {
    close(outUdpSock_);
  }
}

bool UdpThread::initUdpThreadReader(const UdpOptions& ReaderOptions) {
  inUdpOptions_ = ReaderOptions;
  struct ip_mreq mreq;

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  this->inUdpSock_ = sock;
  if (sock < 0) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Listener Thread for UDP Reader socket creation")
    return false;
  }
  memset(reinterpret_cast<void*>(&inAddr_), 0, sizeof(inAddr_));
  inAddr_.sin_family = AF_INET;
  if (inUdpOptions_.multicast) {
    inAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    inet_pton(AF_INET, inUdpOptions_.ipAddr.c_str(), &inAddr_.sin_addr.s_addr);
  }
  inAddr_.sin_port = htons(inUdpOptions_.port);
  inAddrLen_ = sizeof(inAddr_);

  int32_t optval = 1;

  if (setsockopt(sock,
      SOL_SOCKET,
      SO_REUSEADDR,
      &optval,
      sizeof(optval)) == -1) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Listener Thread for UDP Reader socket setsockopt")
    return false;
  }

  if (bind(sock, reinterpret_cast<struct sockaddr *>(&inAddr_), sizeof(inAddr_)) < 0) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Listener Thread for UDP Reader socket bind")
    return false;
  }

  if (inUdpOptions_.multicast) {
    mreq.imr_multiaddr.s_addr = inet_addr(inUdpOptions_.ipAddr.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Listener Thread for UDP Reader socket setsockopt mreq")
      return false;
    }
  }

  inUdpReady_ = true;
  return true;
}

bool UdpThread::initUdpThreadWriter(const UdpOptions& WriterOptions) {
  outUdpOptions_ = WriterOptions;

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  this->outUdpSock_ = sock;
  if (sock < 0) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Writer Thread for UDP Reader socket creation")
    return false;
  }
  memset(static_cast<void*>(&outAddr_), 0, sizeof(outAddr_));
  outAddr_.sin_family = AF_INET;
  inet_pton(AF_INET, outUdpOptions_.ipAddr.c_str(), &outAddr_.sin_addr.s_addr);
  outAddr_.sin_port = htons(outUdpOptions_.port);
  outAddrLen_ = sizeof(outAddr_);

  // setsockopt check and bind not necessary for initializing writers

  outUdpReady_ = true;
  return true;
}

bool UdpThread::startUdpThread() {
  // Return true if thread is already running
  if (isRunning_) {
    return true;
  }

  udpThread_ = std::make_shared<std::thread>(&UdpThread::runUdpConnection, this);
  bool joinable = udpThread_->joinable();

  if (joinable) {
    isRunning_ = true;
  }

  return joinable;
}

void UdpThread::addUdpCommand(const UMSG& input) {
  const std::lock_guard<std::mutex> lock(outputProtectionMutex_);
  if (myCmdData_.size() > 100) {
    myCmdData_.clear();
  }
  myExternalCmdData_ = input;
  myCmdData_.push(myExternalCmdData_);
}

void UdpThread::writeUdpCommand(const UMSG &input) {
  if (outUdpReady_) {
    int64_t bytes_sent = sendto(outUdpSock_, static_cast<const void*>(&input.data[0]), input.len, 0,
                                reinterpret_cast<struct sockaddr*>(&outAddr_), outAddrLen_);

    if (bytes_sent != input.len) {
      UMAA_LOG_INFO(util::SYSTEM_LOGGER, "UDP Message of size " << input.len << " Failed")
    }
  }
}

bool UdpThread::checkStatData() {
  bool has_data = false;
  const std::lock_guard<std::mutex> lock(inputProtectionMutex_);

  has_data = !myStatData_.isEmpty();

  return has_data;
}

UMSG UdpThread::returnStatData() {
  // Double buffer (from_data vs external_from_data) to ensure
  // that we can serve up the latest complete packet across threads

  // copy data to second buffer (struct) for external delivery
  if (checkStatData()) {
    std::lock_guard<std::mutex> lock(inputProtectionMutex_);
    myExternalStatData_ = myStatData_.back();
    myStatData_.clear();
  }

  return myExternalStatData_;
}

void UdpThread::runUdpConnection() {
  // This block of variables and mutex operations is to schedule
  // the thread cycle time to operate on a conditional mutex.  this
  // provides more consistent timing than having the thread sleep, as
  // it accounts for the time it takes to execute the code in the loop
  std::condition_variable cv;
  std::mutex cv_m;
  std::unique_lock<std::mutex> lk(cv_m);

  auto absTime = std::chrono::system_clock::now();

  // Set to run the loop every 5ms (200Hz)
  const auto dt = std::chrono::milliseconds(5);

  int64_t bytes_sent = -1;

  if (inUdpReady_) {
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Listener Thread for UDP:" <<  inUdpOptions_.ipAddr.c_str() <<
     ":" << inUdpOptions_.port << " Starting")
  }
  if (outUdpReady_) {
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Writer Thread for UDP:" <<  outUdpOptions_.ipAddr.c_str() <<
     ":" << outUdpOptions_.port << " Starting")
  }

  while (isRunning_) {
    if (outUdpReady_ && !myCmdData_.isEmpty()) {
      // -------------------------------------------------------------------------------//
      // --------------------------Handle Outgoing Data---------------------------------//
      // -------------------------------------------------------------------------------//
      // send one message per loop from myCmdData_...

      // Pop next cmd message off the queue.
      UMSG cmd = myCmdData_.get();

      bytes_sent = sendto(outUdpSock_, static_cast<void *>(&cmd.data[0]),
          cmd.len, 0, reinterpret_cast<struct sockaddr *>(&outAddr_), outAddrLen_);

      std::lock_guard<std::mutex> lock(outputProtectionMutex_);
      if (bytes_sent != cmd.len) {
        UMAA_LOG_INFO(util::SYSTEM_LOGGER, "UDP Message of size " << cmd.len << " Failed, remaining size: "
          << myCmdData_.size()-1)
      }
    }

    if (inUdpReady_) {
      // -------------------------------------------------------------------------------//
      // --------------------------Handle Incoming Data---------------------------------//
      // -------------------------------------------------------------------------------//
      // Try reading just one message per loop for now... have logic here to clear the buffer if it works

      // Pull UDP data off the port, return number of bytes read and fill tempresult.data
      listenBuffer_.len = recvfrom(inUdpSock_, static_cast<void *>(&listenBuffer_.data[0]),
                                    MAX_UMSG_SIZE, 0, reinterpret_cast<struct sockaddr *>(&inAddr_), &inAddrLen_);

      if (listenBuffer_.len > 0) {
        std::lock_guard<std::mutex> lock(inputProtectionMutex_);
        myStatData_.push(listenBuffer_);
      }
    }

    // -------------------------------------------------------------------------------//
    // --------------------------Handle Loop Timing-----------------------------------//
    // -------------------------------------------------------------------------------//
    // Some logic and mechanisms to schedule the next cycle for this loop (100Hz based on dt of 10ms)
    // Add delta t to the absolute time.  This affirms consistent thread timing

    absTime += dt;

    // Set up for a conditional wait until we are at absTime (last loop time + dt)
    // We don't lock cv_m here because wait_until needs to take ownership to resume the thread
    cv.wait_until(lk, absTime);
  }
}

bool UdpThread::stopUdpThread() {
  // Return true if thread is already stopped
  if (!isRunning_) {
    return true;
  }

  const std::lock_guard<std::mutex> out_lock(outputProtectionMutex_);
  const std::lock_guard<std::mutex> in_lock(inputProtectionMutex_);

  isRunning_ = false;

  bool joinable = udpThread_->joinable();

  if (joinable) {
    udpThread_->join();
  }

  return joinable;
}
}  // namespace arlcore::io
