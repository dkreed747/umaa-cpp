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

#include <gtest/gtest.h>

#include "UdpThread.h"

TEST(UdpThreadTest, udpThreadStartStop) {
  auto thread = new arlcore::io::UdpThread();

  EXPECT_FALSE(thread->isReader());
  EXPECT_FALSE(thread->isWriter());

  EXPECT_TRUE(thread->startUdpThread());
  EXPECT_TRUE(thread->isRunning());

  usleep(100);

  EXPECT_TRUE(thread->stopUdpThread());
  EXPECT_FALSE(thread->isRunning());

  delete thread;
}

TEST(UdpThreadTest, configureReadThread) {
  auto thread = new arlcore::io::UdpThread();
  arlcore::io::UdpOptions read_opts;

  read_opts.ipAddr = "127.0.0.1";
  read_opts.port = 8080;
  read_opts.multicast = false;

  EXPECT_FALSE(thread->isReader());

  EXPECT_TRUE(thread->initUdpThreadReader(read_opts));
  EXPECT_TRUE(thread->isReader());

  EXPECT_FALSE(thread->isRunning());

  delete thread;
}

TEST(UdpThreadTest, configureWriteThread) {
  auto thread = new arlcore::io::UdpThread();
  arlcore::io::UdpOptions write_opts;

  write_opts.ipAddr = "127.0.0.1";
  write_opts.port = 8080;
  write_opts.multicast = false;

  EXPECT_FALSE(thread->isWriter());

  EXPECT_TRUE(thread->initUdpThreadWriter(write_opts));
  EXPECT_TRUE(thread->isWriter());

  EXPECT_FALSE(thread->isRunning());

  delete thread;
}

TEST(UdpThreadTest, configureWriteThreadMulticast) {
  auto thread = new arlcore::io::UdpThread();
  arlcore::io::UdpOptions write_opts;

  write_opts.ipAddr = "224.0.0.1";
  write_opts.port = 8080;
  write_opts.multicast = true;

  EXPECT_FALSE(thread->isWriter());

  EXPECT_TRUE(thread->initUdpThreadWriter(write_opts));
  EXPECT_TRUE(thread->isWriter());

  EXPECT_FALSE(thread->isRunning());

  delete thread;
}

TEST(UdpThreadTest, configureReadThreadMulticast) {
  auto thread = new arlcore::io::UdpThread();
  arlcore::io::UdpOptions read_opts;

  read_opts.ipAddr = "224.0.0.1";
  read_opts.port = 8080;
  read_opts.multicast = true;

  EXPECT_FALSE(thread->isReader());

  EXPECT_TRUE(thread->initUdpThreadReader(read_opts));
  EXPECT_TRUE(thread->isReader());

  EXPECT_FALSE(thread->isRunning());

  delete thread;
}

TEST(UdpThreadTest, testReadThread) {
  auto thread = new arlcore::io::UdpThread();

  arlcore::io::UMSG msg;
  std::string expected = "test";
  memcpy(static_cast<void*>(&msg.data[0]), expected.c_str(), 5);
  msg.len = 5;

  arlcore::io::UdpOptions udp_opts;

  udp_opts.ipAddr = "127.0.0.1";
  udp_opts.port = 8080;
  udp_opts.multicast = false;

  EXPECT_TRUE(thread->initUdpThreadReader(udp_opts));
  EXPECT_TRUE(thread->startUdpThread());

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  EXPECT_GE(sock, 0);

  struct sockaddr_in out_addr;

  bzero(reinterpret_cast<char*>(&out_addr), sizeof(out_addr));
  out_addr.sin_family = AF_INET;
  inet_pton(AF_INET, static_cast<const char*>(udp_opts.ipAddr.c_str()), &out_addr.sin_addr.s_addr);
  out_addr.sin_port = htons(udp_opts.port);

  int64_t bytes_sent = sendto(sock, static_cast<const void*>(&msg.data[0]), msg.len, 0,
                                  reinterpret_cast<struct sockaddr*>(&out_addr), sizeof(out_addr));

  EXPECT_EQ(bytes_sent, msg.len);

  uint8_t timeout = 0;
  while (!thread->checkStatData() && timeout < 10) {
    timeout++;
    usleep(500);
  }
  EXPECT_TRUE(thread->checkStatData());
  arlcore::io::UMSG actual = thread->returnStatData();

  EXPECT_EQ(msg.len, actual.len);
  EXPECT_EQ(memcmp(msg.data, actual.data, actual.len), 0);

  EXPECT_TRUE(thread->stopUdpThread());

  delete thread;
}

TEST(UdpThreadTest, testWriteThread) {
  auto thread = new arlcore::io::UdpThread();

  arlcore::io::UMSG msg;
  std::string expected = "test";
  memcpy(static_cast<void*>(&msg.data[0]), expected.c_str(), 5);
  msg.len = 5;

  arlcore::io::UdpOptions udp_opts;

  udp_opts.ipAddr = "127.0.0.1";
  udp_opts.port = 8080;
  udp_opts.multicast = false;

  EXPECT_TRUE(thread->initUdpThreadWriter(udp_opts));
  EXPECT_TRUE(thread->startUdpThread());

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  EXPECT_GE(sock, 0);

  struct sockaddr_in in_addr;

  bzero(reinterpret_cast<char*>(&in_addr), sizeof(in_addr));
  in_addr.sin_family = AF_INET;
  inet_pton(AF_INET, static_cast<const char*>(udp_opts.ipAddr.c_str()), &in_addr.sin_addr.s_addr);
  in_addr.sin_port = htons(udp_opts.port);
  uint32_t in_addrlen = sizeof(in_addr);

  int32_t optval = 1;

  int32_t optret = setsockopt(sock,
        SOL_SOCKET,
        SO_REUSEADDR,
        &optval,
        sizeof(optval));

  EXPECT_NE(optret, -1);

  int32_t bindret = bind(sock, reinterpret_cast<struct sockaddr *>(&in_addr), sizeof(in_addr));
  EXPECT_GE(bindret, 0);

  usleep(100);
  thread->addUdpCommand(msg);

  arlcore::io::UMSG actual;
  actual.len = 0;
  uint8_t timeout = 0;
  while (actual.len <= 0 && timeout < 10) {
    actual.len = recvfrom(sock, static_cast<void*>(&actual.data[0]), arlcore::io::MAX_UMSG_SIZE, 0,
                          reinterpret_cast<struct sockaddr*>(&in_addr), &in_addrlen);
    timeout++;
    usleep(500);
  }

  EXPECT_EQ(msg.len, actual.len);
  EXPECT_EQ(memcmp(msg.data, actual.data, actual.len), 0);

  EXPECT_TRUE(thread->stopUdpThread());

  delete thread;
}

TEST(UdpThreadTest, testWriteThreadless) {
  auto thread = new arlcore::io::UdpThread();

  arlcore::io::UMSG msg;
  std::string expected = "test";
  memcpy(static_cast<void*>(&msg.data[0]), expected.c_str(), 5);
  msg.len = 5;

  arlcore::io::UdpOptions udp_opts;

  udp_opts.ipAddr = "127.0.0.1";
  udp_opts.port = 8080;
  udp_opts.multicast = false;

  EXPECT_TRUE(thread->initUdpThreadWriter(udp_opts));

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  EXPECT_GE(sock, 0);

  struct sockaddr_in in_addr;

  bzero(reinterpret_cast<char*>(&in_addr), sizeof(in_addr));
  in_addr.sin_family = AF_INET;
  inet_pton(AF_INET, static_cast<const char*>(udp_opts.ipAddr.c_str()), &in_addr.sin_addr.s_addr);
  in_addr.sin_port = htons(udp_opts.port);
  uint32_t in_addrlen = sizeof(in_addr);

  int32_t optval = 1;

  int32_t optret = setsockopt(sock,
        SOL_SOCKET,
        SO_REUSEADDR,
        &optval,
        sizeof(optval));

  EXPECT_NE(optret, -1);

  int32_t bindret = bind(sock, reinterpret_cast<struct sockaddr *>(&in_addr), sizeof(in_addr));
  EXPECT_GE(bindret, 0);

  thread->writeUdpCommand(msg);

  arlcore::io::UMSG actual;
  actual.len = 0;
  uint8_t timeout = 0;
  while (actual.len <= 0 && timeout < 5) {
    actual.len = recvfrom(sock, static_cast<void*>(&actual.data[0]), arlcore::io::MAX_UMSG_SIZE, 0,
                          reinterpret_cast<struct sockaddr*>(&in_addr), &in_addrlen);
    timeout++;
    usleep(100);
  }

  EXPECT_EQ(msg.len, actual.len);
  EXPECT_EQ(memcmp(msg.data, actual.data, actual.len), 0);

  EXPECT_TRUE(thread->stopUdpThread());

  delete thread;
}

TEST(UdpThreadTest, testReadThreadMulticast) {
  auto thread = new arlcore::io::UdpThread();

  arlcore::io::UMSG msg;
  std::string expected = "test";
  memcpy(static_cast<void*>(&msg.data[0]), expected.c_str(), 5);
  msg.len = 5;

  arlcore::io::UdpOptions udp_opts;

  udp_opts.ipAddr = "224.0.0.1";
  udp_opts.port = 8080;
  udp_opts.multicast = true;

  EXPECT_TRUE(thread->initUdpThreadReader(udp_opts));
  EXPECT_TRUE(thread->startUdpThread());

  // set up socket
  int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  EXPECT_GE(sock, 0);

  struct sockaddr_in out_addr;

  bzero(reinterpret_cast<char*>(&out_addr), sizeof(out_addr));
  out_addr.sin_family = AF_INET;
  inet_pton(AF_INET, static_cast<const char*>(udp_opts.ipAddr.c_str()), &out_addr.sin_addr.s_addr);
  out_addr.sin_port = htons(udp_opts.port);

  int32_t bytes_sent = sendto(sock, static_cast<const void*>(&msg.data[0]), msg.len, 0,
                                  reinterpret_cast<struct sockaddr*>(&out_addr), sizeof(out_addr));

  EXPECT_EQ(bytes_sent, msg.len);

  uint8_t timeout = 0;
  while (!thread->checkStatData() && timeout < 10) {
    timeout++;
    usleep(500);
  }
  EXPECT_TRUE(thread->checkStatData());
  arlcore::io::UMSG actual = thread->returnStatData();

  EXPECT_EQ(msg.len, actual.len);
  EXPECT_EQ(memcmp(msg.data, actual.data, actual.len), 0);

  EXPECT_TRUE(thread->stopUdpThread());

  delete thread;
}
