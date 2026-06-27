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

#include <math.h>
#include <gtest/gtest.h>

#include "GeographicUtils.h"

TEST(GeographicUtilsTest, testCalculateShift) {
  flt64_t dist_m = 10000;
  flt64_t new_lat = arlcore::calculateLatitudeShift(0, dist_m);

  flt64_t haversine_dist = arlcore::getHaversineDistance(std::make_pair(0.0, 0.0), std::make_pair(new_lat, 0.0));
  EXPECT_NEAR(dist_m, haversine_dist, 0.01);

  flt64_t new_lon = arlcore::calculateLongitudeShift(0, 0, dist_m);
  haversine_dist = arlcore::getHaversineDistance(std::make_pair(0.0, 0.0), std::make_pair(0.0, new_lon));
  EXPECT_NEAR(dist_m, haversine_dist, 0.01);
}

TEST(GeographicUtilsTest, testIsInsideEllipse) {
  EllipseVariantType ellipse(GeoPosition2D(0, 0), M_PI / 4, 300, 100);
  EXPECT_FALSE(arlcore::isInsideEllipse(ellipse, GeoPosition2D(arlcore::calculateLatitudeShift(0, 290), 0)));
  EXPECT_TRUE(arlcore::isInsideEllipse(ellipse, GeoPosition2D(arlcore::calculateLatitudeShift(0, 100), 0)));
}

TEST(GeographicUtilsTest, testIsInsidePolygon) {
  const flt64_t unit_lat = arlcore::calculateLatitudeShift(0, 100);
  const flt64_t unit_lon = arlcore::calculateLongitudeShift(0, 0, 100);
  const std::array<GeoPosition2D, 4L> points = {GeoPosition2D(2 * unit_lat, -unit_lon), GeoPosition2D(2 * unit_lat, unit_lon),
                                                GeoPosition2D(-2 * unit_lat, unit_lon), GeoPosition2D(-2 * unit_lat, -unit_lon)};
  PolygonVariantType polygon(LineSegmentEnumType::RHUMB, std::vector<GeoPosition2D>(points.begin(), points.end()));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, unit_lon / 2)));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, unit_lon)));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(0, 0)));
  EXPECT_FALSE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, 2 * unit_lon)));
  EXPECT_FALSE(arlcore::isInsidePolygon(polygon, GeoPosition2D(3 * unit_lat, unit_lon)));
}

TEST(GeographicUtilsTest, testIsInsideCommonPolygon) {
  const flt64_t unit_lat = arlcore::calculateLatitudeShift(0, 100);
  const flt64_t unit_lon = arlcore::calculateLongitudeShift(0, 0, 100);
  const std::array<GeoPosition2D, 4L> points = {GeoPosition2D(2 * unit_lat, -unit_lon), GeoPosition2D(2 * unit_lat, unit_lon),
                                                GeoPosition2D(-2 * unit_lat, unit_lon), GeoPosition2D(-2 * unit_lat, -unit_lon)};
  Polygon polygon(LineSegmentEnumType::RHUMB, std::vector<GeoPosition2D>(points.begin(), points.end()));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, unit_lon / 2)));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, unit_lon)));
  EXPECT_TRUE(arlcore::isInsidePolygon(polygon, GeoPosition2D(0, 0)));
  EXPECT_FALSE(arlcore::isInsidePolygon(polygon, GeoPosition2D(unit_lat, 2 * unit_lon)));
  EXPECT_FALSE(arlcore::isInsidePolygon(polygon, GeoPosition2D(3 * unit_lat, unit_lon)));
}

// Accuracies were compared and tested against online calculator
// https://geodesyapps.ga.gov.au/vincenty-inverse
TEST(GeographicUtilsTest, vincentyInverseAccuracyTest) {

  // South Pacific Ocean
  GeoPosition2D position1SW(-11.786490, -87.743875);
  GeoPosition2D position2SW(-9.987262, -93.410661);
  EXPECT_NEAR(650690.231, arlcore::vincentyInverse(position1SW, position2SW), 0.01);

  // Indian Ocean
  GeoPosition2D position1SE(-3.300667, 62.301834);
  GeoPosition2D position2SE(-3.444885, 63.122234);
  EXPECT_NEAR(92553.601, arlcore::vincentyInverse(position1SE, position2SE), 0.01);
  
  // East China Sea
  GeoPosition2D position1NE(28.271816, 121.517579);
  GeoPosition2D position2NE(28.194159, 121.513831);
  EXPECT_NEAR(8614.057, arlcore::vincentyInverse(position1NE, position2NE), 0.01);

  // Lake Erie
  GeoPosition2D position1NW(42.183635, -80.067610);
  GeoPosition2D position2NW(42.187947, -80.070104);
  EXPECT_NEAR(521.396, arlcore::vincentyInverse(position1NW, position2NW), 0.01);

  // Across equator atlantic ocean
  GeoPosition2D position1EQ(0.450013, -20.337528);
  GeoPosition2D position2EQ(-0.339212, -20.436182);
  EXPECT_NEAR(87956.286, arlcore::vincentyInverse(position1EQ, position2EQ), 0.01);

  // Worst possible points to choose
  GeoPosition2D position5(0, 1);
  GeoPosition2D position6(1, 1);
  EXPECT_NEAR(110574.389, arlcore::vincentyInverse(position5, position6), 0.01);
   
  // Positions with the same lat
  GeoPosition2D position7(42.187947, -80.067610);
  GeoPosition2D position8(42.187947, -80.070104);

  // Test when latitude is the same
  EXPECT_NEAR(205.479, arlcore::vincentyInverse(position7, position8), 0.01);
}

// Accuracies were compared and tested against online calculator
// https://geodesyapps.ga.gov.au/vincenty-direct
TEST(GeographicUtilsTest, vincentyDirectAccuracyTest) {
  GeoPosition2D startingPosition(42.1835460, -80.0664050);
  flt64_t distance = 100;
  flt64_t azimuthBearing = 1.67;

  GeoPosition2D results = arlcore::vincentyDirect(startingPosition, azimuthBearing, distance);
  EXPECT_NEAR(42.1834568, results.geodeticLatitude() , 0.000001);
  EXPECT_NEAR(-80.0652005, results.geodeticLongitude() , 0.000001);
}

TEST(GeographicUtilsTest, testIsWithinElevationRange) {
  GlobalPoseReportType gpData;
  ElevationVariantType ceil, floor;

  // Test Depth Variant
  ceil.ElevationVariantTypeSubtypes().DepthVariantVariant(UMAA::Common::Measurement::DepthVariantType());
  floor.ElevationVariantTypeSubtypes().DepthVariantVariant(UMAA::Common::Measurement::DepthVariantType());
  ceil.ElevationVariantTypeSubtypes().DepthVariantVariant().depth(10);
  floor.ElevationVariantTypeSubtypes().DepthVariantVariant().depth(50);

  // Test no depth data
  gpData.depth().reset();
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).has_value());

  gpData.depth(0);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.depth(10);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.depth(15);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.depth(50);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.depth(55);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());

  // Test Mean Sea Level Variant
  ceil.ElevationVariantTypeSubtypes().AltitudeMSLVariantVariant(UMAA::Common::Measurement::AltitudeMSLVariantType());
  floor.ElevationVariantTypeSubtypes().AltitudeMSLVariantVariant(UMAA::Common::Measurement::AltitudeMSLVariantType());
  ceil.ElevationVariantTypeSubtypes().AltitudeMSLVariantVariant().altitude(500);
  floor.ElevationVariantTypeSubtypes().AltitudeMSLVariantVariant().altitude(300);

  gpData.altitude(550);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitude(500);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitude(400);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitude(300);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitude(200);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());

  // Test Geodetic Variant
  ceil.ElevationVariantTypeSubtypes().AltitudeGeodeticVariantVariant(UMAA::Common::Measurement::AltitudeGeodeticVariantType());
  floor.ElevationVariantTypeSubtypes().AltitudeGeodeticVariantVariant(UMAA::Common::Measurement::AltitudeGeodeticVariantType());
  ceil.ElevationVariantTypeSubtypes().AltitudeGeodeticVariantVariant().altitude(500);
  floor.ElevationVariantTypeSubtypes().AltitudeGeodeticVariantVariant().altitude(-100);

  gpData.altitudeGeodetic(550);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeGeodetic(500);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeGeodetic(0);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeGeodetic(-100);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeGeodetic(-200);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());

  // Test Above Sea Floor Variant
  ceil.ElevationVariantTypeSubtypes().AltitudeASFVariantVariant(UMAA::Common::Measurement::AltitudeASFVariantType());
  floor.ElevationVariantTypeSubtypes().AltitudeASFVariantVariant(UMAA::Common::Measurement::AltitudeASFVariantType());
  ceil.ElevationVariantTypeSubtypes().AltitudeASFVariantVariant().altitude(500);
  floor.ElevationVariantTypeSubtypes().AltitudeASFVariantVariant().altitude(100);

  gpData.altitudeASF(550);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeASF(500);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeASF(400);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeASF(100);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeASF(0);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());

  // Test Above Ground Level Variant
  ceil.ElevationVariantTypeSubtypes().AltitudeAGLVariantVariant(UMAA::Common::Measurement::AltitudeAGLVariantType());
  floor.ElevationVariantTypeSubtypes().AltitudeAGLVariantVariant(UMAA::Common::Measurement::AltitudeAGLVariantType());
  ceil.ElevationVariantTypeSubtypes().AltitudeAGLVariantVariant().altitude(500);
  floor.ElevationVariantTypeSubtypes().AltitudeAGLVariantVariant().altitude(100);

  gpData.altitudeAGL(550);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeAGL(500);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeAGL(400);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeAGL(100);
  EXPECT_TRUE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
  gpData.altitudeAGL(0);
  EXPECT_FALSE(arlcore::isWithinElevationRange(ceil, floor, gpData).value());
}