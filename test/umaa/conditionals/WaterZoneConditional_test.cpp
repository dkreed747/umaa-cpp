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

#include "WaterZoneConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::WaterZoneKindEnumModule::WaterZoneKindEnumType;
using UMAA::Common::Measurement::GeoPosition2D;
using UMAA::Common::Measurement::ElevationVariantTypeEnum;
using arlcore::umaa::conditional::WaterZoneConditional;

class WaterZoneConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::WaterZoneConditionalTypeTopic);
    specializedConditional_.specializationReferenceID(specializationId);
    specializedConditional_.specializationReferenceTimestamp(specializationTimestamp);
    specializedConditional_.zoneKind(defaultKind);
    specializedConditional_.ceiling().ElevationVariantTypeSubtypes().DepthVariantVariant(UMAA::Common::Measurement::DepthVariantType());
    specializedConditional_.ceiling().ElevationVariantTypeSubtypes().DepthVariantVariant().depth(ceilDepth);
    specializedConditional_.floor().ElevationVariantTypeSubtypes().DepthVariantVariant(UMAA::Common::Measurement::DepthVariantType());
    specializedConditional_.floor().ElevationVariantTypeSubtypes().DepthVariantVariant().depth(floorDepth);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::WaterZoneConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultWaterZone;
  static const WaterZoneKindEnumType defaultKind;
  static const UMAA::Common::Measurement::ElevationVariantTypeEnum defaultElevationVarient;
  static const flt64_t floorDepth;
  static const flt64_t ceilDepth;
  static const UMAA::MM::BaseType::EllipseVariantType ellipse1;
  static const UMAA::MM::BaseType::EllipseVariantType ellipse2;
  static const flt64_t pointDist1;
  static const flt64_t pointDist2;
  // Ellipse centered at 0,0 with a major-axis of 300m and minor axis of 100m with a rotation of PI/4 radians from true North
  static const std::array<arlcore::umaa::conditional::GeoPosition2D, 4L> points1;
  // Ellipse centered at 0,0 with a major-axis of 300m and minor axis of 100m with a rotation of 0 radians from true North
  static const std::array<arlcore::umaa::conditional::GeoPosition2D, 4L> points2;
  // Square centered at 0,0 with sides approx 500m in length
  static const UMAA::MM::BaseType::PolygonVariantType polygon1;
  // Square centered at 0,0 with sides approx 1000m in length
  static const UMAA::MM::BaseType::PolygonVariantType polygon2;
};

const arlcore::NumericGuid WaterZoneConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid WaterZoneConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string WaterZoneConditionalTest::conditionalName = "TestWaterZoneConditional";
const UMAA::Common::Measurement::DateTime WaterZoneConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t WaterZoneConditionalTest::defaultWaterZone = 12.34;
const WaterZoneKindEnumType WaterZoneConditionalTest::defaultKind = WaterZoneKindEnumType::INSIDE;
const UMAA::Common::Measurement::ElevationVariantTypeEnum WaterZoneConditionalTest::defaultElevationVarient = UMAA::Common::Measurement::ElevationVariantTypeEnum::DEPTHVARIANT_D;
const flt64_t WaterZoneConditionalTest::floorDepth = 100;
const flt64_t WaterZoneConditionalTest::ceilDepth = 20;
const UMAA::MM::BaseType::EllipseVariantType WaterZoneConditionalTest::ellipse1 = UMAA::MM::BaseType::EllipseVariantType(GeoPosition2D(0, 0), M_PI / 4, 300, 100);
const UMAA::MM::BaseType::EllipseVariantType WaterZoneConditionalTest::ellipse2 = UMAA::MM::BaseType::EllipseVariantType(GeoPosition2D(0, 0), 0, 300, 100);
const flt64_t WaterZoneConditionalTest::pointDist1 = arlcore::calculateLatitudeShift(0, 250);
const flt64_t WaterZoneConditionalTest::pointDist2 = arlcore::calculateLatitudeShift(0, 500);
const std::array<GeoPosition2D, 4L> WaterZoneConditionalTest::points1 = {GeoPosition2D(pointDist1, -pointDist1), GeoPosition2D(pointDist1, pointDist1), 
                                                                         GeoPosition2D(-pointDist1, pointDist1), GeoPosition2D(-pointDist1, -pointDist1)};
const std::array<GeoPosition2D, 4L> WaterZoneConditionalTest::points2 = {GeoPosition2D(pointDist2, -pointDist2), GeoPosition2D(pointDist2, pointDist2), 
                                                                         GeoPosition2D(-pointDist2, pointDist2), GeoPosition2D(-pointDist2, -pointDist2)};
const UMAA::MM::BaseType::PolygonVariantType WaterZoneConditionalTest::polygon1 = UMAA::MM::BaseType::PolygonVariantType(arlcore::umaa::conditional::LineSegmentEnumType::RHUMB,
  std::vector<arlcore::umaa::conditional::GeoPosition2D>(points1.begin(), points1.end()));
const UMAA::MM::BaseType::PolygonVariantType WaterZoneConditionalTest::polygon2 = UMAA::MM::BaseType::PolygonVariantType(arlcore::umaa::conditional::LineSegmentEnumType::RHUMB,
  std::vector<arlcore::umaa::conditional::GeoPosition2D>(points2.begin(), points2.end()));

TEST_F(WaterZoneConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  EXPECT_EQ(waterZoneConditional.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(waterZoneConditional.getConditionalWaterZoneKind(), defaultKind);
  EXPECT_EQ(waterZoneConditional.getCeiling(), specializedConditional_.ceiling().ElevationVariantTypeSubtypes());
  EXPECT_EQ(waterZoneConditional.getFloor(), specializedConditional_.floor().ElevationVariantTypeSubtypes());
  auto zones = waterZoneConditional.getZones();
  EXPECT_EQ(zones.size(), specializedConditional_.zone().size());
  EXPECT_TRUE(std::equal(zones.begin(), zones.end(), specializedConditional_.zone().begin()));
  auto deps = waterZoneConditional.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(WaterZoneConditionalTest, noGlobalPoseData) {
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  EXPECT_EQ(waterZoneConditional.evaluateConditional(), std::nullopt);
}

TEST_F(WaterZoneConditionalTest, testInsideCircle) {
  UMAA::MM::BaseType::ShapeVariantType zone;
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant().semiMajorRadius(100); // Overwrite for circle
  std::array<UMAA::MM::BaseType::ShapeVariantType, 1L> zones = { zone };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testOutsideCircle) {
  specializedConditional_.zoneKind(WaterZoneKindEnumType::OUTSIDE);
  UMAA::MM::BaseType::ShapeVariantType zone;
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant().semiMajorRadius(100); // Overwrite for circle
  std::array<UMAA::MM::BaseType::ShapeVariantType, 1L> zones = { zone };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Inside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testInsideEllipse) {
  UMAA::MM::BaseType::ShapeVariantType zone;
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse1);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 1L> zones = { zone };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 100), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testInsideEllipseMultiple) {
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone1.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of ellipses
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 120), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testOutsideEllipseMultiple) {
  specializedConditional_.zoneKind(WaterZoneKindEnumType::OUTSIDE);
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone1.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipse
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of ellipses
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 120), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Outside of ellipses
  gpData.position(GeoPosition2D(0, arlcore::calculateLatitudeShift(0, 250)));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testInsidePolygon) {
  UMAA::MM::BaseType::ShapeVariantType zone;
  zone.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 1L> zones = { zone };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testOutsidePolygon) {
  specializedConditional_.zoneKind(WaterZoneKindEnumType::OUTSIDE);
  UMAA::MM::BaseType::ShapeVariantType zone;
  zone.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 1L> zones = { zone };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Outside of polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Inside of polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testInsideMultiplePolygons) {
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone2.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of both polygons
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 600), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());
  
  // Outside of one polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of both polygons
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testOutsideMultiplePolygons) {
  specializedConditional_.zoneKind(WaterZoneKindEnumType::OUTSIDE);
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone2.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Outside of both polygons
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 600), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
  
  // Outside of one polygon
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 300), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of both polygons
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 90), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testInsideMixedZoneTypes) {
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());

  // Outside of both zones
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 400), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());
  
  // Outside of one zone
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_FALSE(waterZoneConditional.evaluateConditional().value());

  // Inside of both zones
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 240), 0));
  waterZoneConditional.update(gpData);
  ASSERT_TRUE(waterZoneConditional.evaluateConditional().has_value());
  EXPECT_TRUE(waterZoneConditional.evaluateConditional().value());
}

TEST_F(WaterZoneConditionalTest, testOutsideMixedZoneTypes) {
  specializedConditional_.zoneKind(WaterZoneKindEnumType::OUTSIDE);
  UMAA::MM::BaseType::ShapeVariantType zone1;
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(UMAA::MM::BaseType::PolygonVariantType());
  zone1.ShapeVariantTypeSubtypes().PolygonVariantVariant(polygon1);
  UMAA::MM::BaseType::ShapeVariantType zone2;
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(UMAA::MM::BaseType::EllipseVariantType());
  zone2.ShapeVariantTypeSubtypes().EllipseVariantVariant(ellipse2);
  std::array<UMAA::MM::BaseType::ShapeVariantType, 2L> zones = { zone1, zone2 };
  specializedConditional_.zone(std::vector<UMAA::MM::BaseType::ShapeVariantType>(zones.begin(), zones.end()));
  arlcore::umaa::conditional::WaterZoneConditional waterZoneConditional(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  // Outside of depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth - 10);
  waterZoneConditional.update(gpData);
  auto eval = waterZoneConditional.evaluateConditional();
  ASSERT_TRUE(eval.has_value());
  EXPECT_TRUE(eval.value());

  // Within depth range
  gpData.position(GeoPosition2D(0, 0));
  gpData.depth(ceilDepth + 10);
  waterZoneConditional.update(gpData);
  eval = waterZoneConditional.evaluateConditional();
  ASSERT_TRUE(eval.has_value());
  EXPECT_FALSE(eval.value());

  // Outside of both zones
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 400), 0));
  waterZoneConditional.update(gpData);
  eval = waterZoneConditional.evaluateConditional();
  ASSERT_TRUE(eval.has_value());
  EXPECT_TRUE(eval.value());
  
  // Outside of one zone
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0));
  waterZoneConditional.update(gpData);
  eval = waterZoneConditional.evaluateConditional();
  ASSERT_TRUE(eval.has_value());
  EXPECT_FALSE(eval.value());

  // Inside of both zones
  gpData.position(GeoPosition2D(arlcore::calculateLatitudeShift(0, 240), 0));
  waterZoneConditional.update(gpData);
  eval = waterZoneConditional.evaluateConditional();
  ASSERT_TRUE(eval.has_value());
  EXPECT_FALSE(eval.value());
}