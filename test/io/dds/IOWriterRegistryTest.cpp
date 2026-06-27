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

#include "IOWriterRegistry.h"
#include "LocalReaderSender.h"

#include <UMAA/MM/BaseType/HoverObjectiveType.hpp>

using UMAA::MM::BaseType::HoverObjectiveType;
using UMAA::MM::BaseType::HoverObjectiveTypeTopic;
using arlcore::io::IOWriterRegistry;
using arlcore::io::LocalReaderSender;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SenderBase;
using arlcore::io::SendStatus;


class IOWriterRegistryTest : public ::testing::Test {
 protected:
  IOWriterRegistryTest() {
    registry_.registerWriter<HoverObjectiveType>
      (HoverObjectiveTypeTopic , hoverRS_ = std::make_shared<LocalReaderSender<HoverObjectiveType>>());
  }

  std::shared_ptr<LocalReaderSender<HoverObjectiveType>> hoverRS_;
  IOWriterRegistry registry_;
};

TEST_F(IOWriterRegistryTest, testRegister) {
  HoverObjectiveType cmd;
  cmd.duration(25);
  auto writer = registry_.getWriter<HoverObjectiveType>(HoverObjectiveTypeTopic);
  writer->send(cmd);

  // Verify that the writer successfully wrote.
  HoverObjectiveType recv;
  EXPECT_EQ(hoverRS_->read(&recv), ReadStatus::SUCCESS);
  EXPECT_EQ(recv, cmd);

  writer->dispose(cmd);
  EXPECT_EQ(hoverRS_->read(&recv), ReadStatus::DISPOSED);
  EXPECT_EQ(recv, cmd);
}

TEST_F(IOWriterRegistryTest, testWrongTopic) {
  auto writer = registry_.getWriter<HoverObjectiveType>("WrongTopic");
  EXPECT_FALSE(writer);  //  nullptr
}

TEST_F(IOWriterRegistryTest, testWrongType) {
  auto writer = registry_.getWriter<std::string>(HoverObjectiveTypeTopic);
  EXPECT_FALSE(writer);  //  nullptr
}