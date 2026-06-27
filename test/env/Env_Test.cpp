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

#include <cstdlib>

#include "Env.h"

TEST(EnvVariablesTest, loadEnvVarNominalOpt) {
  const std::string testVarName = "ENV_VAR_1";
  const std::string testVarValue = "EnvironmentVariable1";

  ASSERT_EQ(setenv(testVarName.c_str(), testVarValue.c_str(), 1), 0);
  std::optional<std::string> testVarValueOpt = arlcore::env::getEnv<std::string>(testVarName);
  EXPECT_TRUE(testVarValueOpt);
  EXPECT_EQ(testVarValue, *testVarValueOpt);
}

TEST(EnvVariablesTest, loadEnvVarUndefinedOpt) {
  const std::string testVarName = "ENV_VAR_2";

  std::optional<std::string> testVarValueOpt = arlcore::env::getEnv<std::string>(testVarName);
  EXPECT_FALSE(testVarValueOpt);
}

TEST(EnvVariablesTest, loadEnvVarNominal) {
  const std::string testVarName = "ENV_VAR_1";
  const std::string testVarValue = "EnvironmentVariable1";

  ASSERT_EQ(setenv(testVarName.c_str(), testVarValue.c_str(), 1), 0);
  std::string outVar;
  bool isSuccessful = arlcore::env::getEnv<std::string>(testVarName, outVar);
  EXPECT_TRUE(isSuccessful);
  EXPECT_EQ(testVarValue, outVar);
}

TEST(EnvVariablesTest, loadEnvVarUndefined) {
  const std::string testVarName = "ENV_VAR_2";

  std::string outVar;
  bool isSuccessful = arlcore::env::getEnv<std::string>(testVarName, outVar);
  EXPECT_FALSE(isSuccessful);
}
