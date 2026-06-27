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

#include "IOReaderRegistry.h"
#include "LocalReaderSender.h"

#include <UMAA/MM/BaseType/HoverObjectiveType.hpp>

using UMAA::MM::BaseType::HoverObjectiveType;
using UMAA::MM::BaseType::HoverObjectiveTypeTopic;
using arlcore::io::IOReaderRegistry;
using arlcore::io::LocalReaderSender;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;

class IOReaderRegistryTest : public ::testing::Test {
 protected:
  IOReaderRegistryTest() {
    readerRegistry_.registerReader<HoverObjectiveType>
      (HoverObjectiveTypeTopic , hoverRS_ = std::make_shared<LocalReaderSender<HoverObjectiveType>>());
  }
  
  std::shared_ptr<LocalReaderSender<HoverObjectiveType>> hoverRS_;
  IOReaderRegistry readerRegistry_;
};

TEST_F(IOReaderRegistryTest, testRegister) {
  HoverObjectiveType cmd;
  cmd.duration(25);
  hoverRS_->send(cmd);

  auto reader = readerRegistry_.getReader<HoverObjectiveType>(HoverObjectiveTypeTopic);
  HoverObjectiveType recv;
  EXPECT_EQ(hoverRS_->read(&recv), ReadStatus::SUCCESS);
  EXPECT_EQ(recv, cmd);

  hoverRS_->dispose(cmd);
  EXPECT_EQ(hoverRS_->read(&recv), ReadStatus::DISPOSED);
  EXPECT_EQ(recv, cmd);
}

TEST_F(IOReaderRegistryTest, testWrongTopic) {
  auto reader = readerRegistry_.getReader<HoverObjectiveType>("WrongTopic");
  EXPECT_FALSE(reader);  //  nullptr
}

TEST_F(IOReaderRegistryTest, testWrongType) {
  auto reader = readerRegistry_.getReader<std::string>(HoverObjectiveTypeTopic);
  EXPECT_FALSE(reader);  //  nullptr
}