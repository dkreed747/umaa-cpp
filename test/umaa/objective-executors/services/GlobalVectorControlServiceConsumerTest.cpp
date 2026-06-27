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

// See full GTest Documentation for reference:
// https://google.github.io/googletest/
#include <gtest/gtest.h>

#include "GlobalVectorControlServiceConsumer.h"
#include "LocalReaderSender.h"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp"

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;

TEST(GlobalVectorCommandConsumerTest, Constructor) {
  auto gvIo = std::make_shared<arlcore::umaa::GlobalVectorControlServiceConsumerIo>(
  std::make_shared<arlcore::io::LocalReaderSender<GlobalVectorCommandType>>(),
  std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalVectorCommandAckReportType>>(),
  std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalVectorCommandStatusType>>(),
  std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalVectorExecutionStatusReportType>>());

  arlcore::umaa::domain::CommandHeader header;
  header.sourceId = UMAA::Common::IdentifierType(
    arlcore::UuidFactory::getInstance().generateGuid().getGuid(), arlcore::NIL_GUID.getGuid());
  header.destinationId = UMAA::Common::IdentifierType(
    arlcore::UuidFactory::getInstance().generateGuid().getGuid(), arlcore::NIL_GUID.getGuid());

  arlcore::umaa::GlobalVectorControlServiceConsumer gvc(header, gvIo);

  EXPECT_TRUE(gvc.openCommandSession());
  EXPECT_EQ(gvc.closeCommandSession(), SendStatus::SUCCESS);
}
