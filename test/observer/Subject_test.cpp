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

#include <memory>

#include "Subject.h"
#include "TestObserver.h"

struct SubjectData {
  flt64_t latitude;
  flt64_t longitude;
};

TEST(SubjectTest, getObserverCount_empty) {
  arlcore::Subject<SubjectData> subject;

  EXPECT_EQ(0, subject.getObserverCount());
}

TEST(SubjectTest, notify_empty) {
  arlcore::Subject<SubjectData> subject;

  EXPECT_EQ(0, subject.getObserverCount());

  SubjectData testData;

  subject.notify(testData);
}

TEST(SubjectTest, notify_test) {
  arlcore::Subject<SubjectData> subject;

  EXPECT_EQ(0, subject.getObserverCount());

  auto testObs = std::make_shared<arlcoretest::TestObserver<SubjectData>>();
  subject.registerObserver(testObs);

  EXPECT_EQ(1, subject.getObserverCount());

  SubjectData testData;
  testData.latitude = 45.6789;
  testData.longitude = 12.3456789;

  EXPECT_EQ(0, testObs->getDataCount());
  subject.notify(testData);

  // Verify test observer got data
  ASSERT_EQ(1, testObs->getDataCount());

  // Verify data has not been modified
  SubjectData actualData = testObs->getNextData();
  ASSERT_EQ(0, testObs->getDataCount());

  EXPECT_EQ(testData.latitude, actualData.latitude);
  EXPECT_EQ(testData.longitude, actualData.longitude);

}

TEST(SubjectTest, unregister_invalid) {
  arlcore::Subject<int> subject;

  EXPECT_EQ(0, subject.getObserverCount());

  auto testObs = std::make_shared<arlcoretest::TestObserver<int>>();
  subject.registerObserver(testObs);

  EXPECT_EQ(1, subject.getObserverCount());
  testObs.reset();

  // Count doesn't update until notify is called for invalid pointers
  subject.notify(12);
  EXPECT_EQ(0, subject.getObserverCount());
}

TEST(SubjectTest, unregister_observer) {
  arlcore::Subject<int> subject;

  EXPECT_EQ(0, subject.getObserverCount());

  auto testObs = std::make_shared<arlcoretest::TestObserver<int>>();
  subject.registerObserver(testObs);

  EXPECT_EQ(1, subject.getObserverCount());
  subject.unregisterObserver(testObs);

  EXPECT_EQ(0, subject.getObserverCount());
}

TEST(SubjectTest, register_multiple) {
  arlcore::Subject<int> subject;

  EXPECT_EQ(0, subject.getObserverCount());

  std::shared_ptr<arlcoretest::TestObserver<int>> testObs[5] = {std::make_shared<arlcoretest::TestObserver<int>>(), std::make_shared<arlcoretest::TestObserver<int>>(),
    std::make_shared<arlcoretest::TestObserver<int>>(), std::make_shared<arlcoretest::TestObserver<int>>(), std::make_shared<arlcoretest::TestObserver<int>>()};
  subject.registerObserver(testObs[0]);
  subject.registerObserver(testObs[1]);
  subject.registerObserver(testObs[2]);
  subject.registerObserver(testObs[3]);
  subject.registerObserver(testObs[4]);

  EXPECT_EQ(5, subject.getObserverCount());

  testObs[0].reset();
  testObs[2].reset();
  testObs[4].reset();

  subject.notify(12);
  EXPECT_EQ(2, subject.getObserverCount());

  subject.unregisterObserver(testObs[1]);
  subject.unregisterObserver(testObs[3]);

  EXPECT_EQ(0, subject.getObserverCount());
}

TEST(SubjectTest, resend_on_register) {
  arlcore::Subject<int> subject(true);
  auto testObs1 = std::make_shared<arlcoretest::TestObserver<int>>();
  auto testObs2 = std::make_shared<arlcoretest::TestObserver<int>>();
  subject.registerObserver(testObs1);
  subject.notify(12);
  EXPECT_EQ(1, subject.getObserverCount());
  EXPECT_EQ(testObs1->getNextData(), 12);

  subject.registerObserver(testObs2);
  EXPECT_EQ(2, subject.getObserverCount());
  EXPECT_EQ(testObs2->getNextData(), 12);
}