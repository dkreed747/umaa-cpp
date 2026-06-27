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
#include <string>
#include <fstream>

#include "ConfigurationManager.h"

static const char TEST_FILENAME[] = "test.yml";

class ConfigurationManager_test: public testing::Test {
 protected:
  ConfigurationManager_test() {}

  virtual ~ConfigurationManager_test() {}

  static void WriteConfigFile(
    const char* topNode,
    const char* firstNestedNode,
    const int32_t &domainID,
    const char* secondNestedNode,
    const char* qosFile,
    const char* thirdNestedNode,
    const char* qosProfile,
    const char* fourthNestedNode,
    const char* largeCollectionsProfile) {
    char buffer[200];

    std::fstream fileStr;
    fileStr.open(TEST_FILENAME, std::fstream::out);

    std::snprintf(buffer, sizeof(buffer), "%s:\n  %s: %d\n  %s: \"%s\"\n  %s: \"%s\"\n  %s: \"%s\"\n",
     topNode,
     firstNestedNode, domainID,
     secondNestedNode, qosFile,
     thirdNestedNode, qosProfile,
     fourthNestedNode, largeCollectionsProfile);

    fileStr << buffer;
    fileStr.close();
  }


  static void TearDownTestCase() {
    std::remove(TEST_FILENAME);
  }

  virtual void SetUp() {}

  virtual void TearDown() {
    setenv("domain-id", "", true);
    setenv("domain-qos-file", "", true);
    setenv("domain-qos-profile", "", true);
    setenv("large-collections-qos-profile", "", true);
  }
};

TEST_F(ConfigurationManager_test, loadMissingConfig) {
  arlcore::ConfigurationManager mgr;

  ASSERT_FALSE(mgr.loadAppConfig("MissingConfigFile.yml"));
}

TEST_F(ConfigurationManager_test, loadGoodAppConfig) {
  arlcore::ConfigurationManager mgr;

  std::string domainQosProfile = "BuiltinQosLibExp::Generic.BestEffort";
  std::string largeQosProfile = "BuiltinQosLibExp::Generic.BestEffort";

  WriteConfigFile("dds", "domain-id", 12, "domain-qos-file", "test.xml",
   "domain-qos-profile", domainQosProfile.c_str(), "large-collections-qos-profile", largeQosProfile.c_str());

  ASSERT_TRUE(mgr.loadAppConfig(TEST_FILENAME));

  EXPECT_EQ(12, mgr.getAppConfig().getDomainId());
  EXPECT_EQ(domainQosProfile, mgr.getAppConfig().getDomainQosProfile());
  EXPECT_EQ(largeQosProfile, mgr.getAppConfig().getLargeCollectionsQosProfile());
  EXPECT_EQ("test.xml", mgr.getAppConfig().getQosFilePath());

  EXPECT_EQ(12, atoi(std::getenv("domain-id")));
  EXPECT_EQ(domainQosProfile, std::getenv("domain-qos-profile"));
}

TEST_F(ConfigurationManager_test, loadBadAppConfig) {
  arlcore::ConfigurationManager mgr;

  std::string domainQosProfile = "BuiltinQosLibExp::Generic.BestEffort";
  std::string largeQosProfile = "BuiltinQosLibExp::Generic.BestEffort";

  WriteConfigFile("dds", "broken-node", 12, "domain-qos-file", "test.xml",
   "domain-qos-profile", domainQosProfile.c_str(), "large-collections-qos-profile", largeQosProfile.c_str());

  // The behavior of a broken node or missing node is to load default values
  // Default domain ID is 0

  ASSERT_TRUE(mgr.loadAppConfig(TEST_FILENAME));

  EXPECT_EQ(0, mgr.getAppConfig().getDomainId());
  EXPECT_EQ(domainQosProfile, mgr.getAppConfig().getDomainQosProfile());
  EXPECT_EQ(largeQosProfile, mgr.getAppConfig().getLargeCollectionsQosProfile());
  EXPECT_EQ("test.xml", mgr.getAppConfig().getQosFilePath());

  EXPECT_EQ(NULL, atoi(std::getenv("domain-id")));
  EXPECT_EQ(domainQosProfile, std::getenv("domain-qos-profile"));
}
