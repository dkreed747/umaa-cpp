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

#ifndef INCLUDE_ALGORITHMS_GEOGRAPHICUTILS_H_
#define INCLUDE_ALGORITHMS_GEOGRAPHICUTILS_H_

#include <math.h>
#include <algorithm>
#include <utility>

#include <UMAA/MM/BaseType/EllipseVariantType.hpp>
#include <UMAA/MM/BaseType/PolygonVariantType.hpp>
#include "UMAA/MM/BaseType/WaypointType.hpp"
#include "UMAA/Common/Measurement/Polygon.hpp"
#include <UMAA/Common/Measurement/Measurements.hpp>
#include <UMAA/Common/Measurement/ElevationVariantType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "GeographicLib/Geocentric.hpp"
#include "GeographicLib/LocalCartesian.hpp"
#include "GeographicLib/Geodesic.hpp"
#include "GuidanceInput.h"

#include "InternalTypes.h"
#include "Logger.h"

using UMAA::MM::BaseType::EllipseVariantType;
using UMAA::MM::BaseType::PolygonVariantType;
using UMAA::Common::Measurement::Polygon;
using UMAA::Common::Measurement::ElevationVariantType;
using UMAA::Common::Measurement::ElevationVariantTypeEnum;
using UMAA::Common::Measurement::ElevationVariantTypeUnion;
using UMAA::Common::Measurement::GeoPosition2D;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::Common::Enumeration::LineSegmentEnumModule::LineSegmentEnumType;

namespace arlcore {

constexpr auto RADIUS_OF_EARTH_M = 6371000;
constexpr auto DEGREES_TO_RADIANS_ = M_PI / 180.00;
constexpr auto RADIANS_TO_DEGREES_ = 180.00 / M_PI;

//! \brief Calculates a new latitude based on a shift in meters
//! \param lat Latitude to be shifted
//! \param shift Number of meters to shift
//! \return New latitude value
static flt64_t calculateLatitudeShift(flt64_t lat, flt64_t shift) {
  flt64_t newLatitude = lat + (shift / RADIUS_OF_EARTH_M) * (180 / M_PI);
  return newLatitude;
}

//! \brief Calculates a new longitude based on a shift in meters
//! \param lat Latitude
//! \param lon Longitude to be shifted
//! \param shift Number of meters to shift
//! \return New longitude value
static flt64_t calculateLongitudeShift(flt64_t lat, flt64_t lon, flt64_t shift) {
  flt64_t newLongitude = lon + (shift / RADIUS_OF_EARTH_M) * (180 / M_PI) / cos(lat * M_PI / 180);
  return newLongitude;
}

//! \brief Calculates a new position based on a shift of latitude and longitude in meters
//! \param origin The original position shift from
//! \param shift_lat_m The amount to shift the latitude in meters (can be negative)
//! \param shift_lon_m The amount to shift the longitude in meters (can be negative)
//! \return The new shifted position
static GeoPosition2D calculateShift(const GeoPosition2D &origin, flt64_t shift_lat_m, flt64_t shift_lon_m) {
  GeoPosition2D result;
  result.geodeticLatitude(calculateLatitudeShift(origin.geodeticLatitude(), shift_lat_m));
  result.geodeticLongitude(calculateLongitudeShift(origin.geodeticLatitude(), origin.geodeticLongitude(), shift_lon_m));
  return result;
}

//! \brief Calculates the heading (bearing) between to lat/lon points.
//! \param lat1 Latitude of point 1
//! \param lon1 Longitude of point 1
//! \param lat2 Latitude of point 2
//! \param lon2 Longitude of point 2
//! \return Heading [-180, 180] in degrees
static flt64_t calculateHeadingBetweenWaypoints(flt64_t lat1, flt64_t lon1, flt64_t lat2, flt64_t lon2) {
  flt64_t deltaLon = lon2 - lon1;
  flt64_t x = cos(lat2) * sin(deltaLon);
  flt64_t y = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(deltaLon);
  flt64_t heading = atan2(x, y);
  heading = heading * 180 / M_PI;
  return heading;
}

//! \brief Calculate the yaw for each waypoint based on the position of the next one
//! \param latPoint1 Current waypoint latitude
//! \param lonPoint1 Current waypoint longitude
//! \param latPoint2 Next waypoint latitude
//! \param lonPoint2 Next waypoint longitude
//! \return The resulting yaw
static flt64_t azimuthBetweenPoints(flt64_t latPoint1, flt64_t lonPoint1,
    flt64_t latPoint2, flt64_t lonPoint2) {
  GeographicLib::Geodesic earth_geodesic(GeographicLib::Constants::WGS84_a(),
                                        GeographicLib::Constants::WGS84_f());

  flt64_t x;
  flt64_t y;
  earth_geodesic.Inverse(latPoint1, lonPoint1, latPoint2, lonPoint2, x, y);

  return y * (M_PI / 180);  // degrees to radians
}

//! \brief Projects the vector of the current vehicle position onto the target line between the previous and next
//! waypoint.
//! \param in GuidanceInput that holds the CurrentDeg, PrevTargetDeg, and TargetDeg.
//! \param projectedLat The variable to store the projected lat
//! \param projectedLon The variable to store the projected lon
static void projectPositionOntoVector(const arl::algorithm::GuidanceInput& in, flt64_t* projectedLat,
    flt64_t* projectedLon) {
  flt64_t posDeltaX = in.lonCurrentDeg - in.lonPrevTargetDeg;
  flt64_t posDeltaY = in.latCurrentDeg - in.latPrevTargetDeg;

  flt64_t wptDeltaX = in.lonTargetDeg - in.lonPrevTargetDeg;
  flt64_t wptDeltaY = in.latTargetDeg - in.latPrevTargetDeg;

  flt64_t dotProduct = posDeltaX * wptDeltaX + posDeltaY * wptDeltaY;

  flt64_t magnitude = wptDeltaX * wptDeltaX + wptDeltaY * wptDeltaY;

  *projectedLon = (dotProduct / magnitude) * wptDeltaX + in.lonPrevTargetDeg;
  *projectedLat = (dotProduct / magnitude) * wptDeltaY + in.latPrevTargetDeg;
}

//! \brief Projects the vector of the current vehicle position onto the target line between the previous
//! and next waypoint
//! \param globalPoseReport The current global pose of the vehicle
//! \param currentWaypoint The target waypoint
//! \param previousWaypoint The previous waypoint
//! \param projectedLat The variable to store the projected lat
//! \param projectedLon The variable to store the projected lon
static void projectPositionOntoVector(GlobalPoseReportType globalPoseReport,
    UMAA::MM::BaseType::WaypointType currentWaypoint, UMAA::MM::BaseType::WaypointType previousWaypoint,
    flt64_t* projectedLat, flt64_t* projectedLon) {
  flt64_t posDeltaX = globalPoseReport.position().geodeticLongitude() -
    previousWaypoint.position().geodeticLongitude();
  flt64_t posDeltaY = globalPoseReport.position().geodeticLatitude() -
    previousWaypoint.position().geodeticLatitude();

  flt64_t wptDeltaX = currentWaypoint.position().geodeticLongitude() - previousWaypoint.position().geodeticLongitude();
  flt64_t wptDeltaY = currentWaypoint.position().geodeticLatitude() - previousWaypoint.position().geodeticLatitude();

  flt64_t dotProduct = posDeltaX * wptDeltaX + posDeltaY * wptDeltaY;
  flt64_t magnitude = wptDeltaX * wptDeltaX + wptDeltaY * wptDeltaY;

  *projectedLon = (dotProduct / magnitude) * wptDeltaX + previousWaypoint.position().geodeticLongitude();
  *projectedLat = (dotProduct / magnitude) * wptDeltaY + previousWaypoint.position().geodeticLatitude();
}

//! \brief Returns the unwound value (removing any modulo 2PI radians) of input in
//! radians. This is used in guidance and control algorithms to keep angle in -PI
//! to PI radians.
//! \param[in] angleRad in radians
//! \return unwound value of input in radians
static flt64_t Unwind(flt64_t angleRad) {
  angleRad = angleRad - static_cast<int>(angleRad / (2.0 * M_PI)) * 2.0 * M_PI;
  if (angleRad > M_PI) {
    angleRad -= 2 * M_PI;
  } else if (angleRad <= -M_PI) {
    angleRad += 2 * M_PI;
  }
  return angleRad;
}

//! \brief Convert latitude and longitude to NED
//! \param lat
//! \param lon
//! \param h
//! \param lat0
//! \param lon0
//! \param n
//! \param e
//! \param d
static void convertLatLonToNed(flt64_t lat, flt64_t lon, flt64_t h, flt64_t lat0, flt64_t lon0,
  flt64_t *n, flt64_t *e, flt64_t *d) {
  flt64_t temp_up;
  // set NED origin to (lat0, lon0, 0)
  GeographicLib::LocalCartesian ned_frame(lat0, lon0, 0);  // use default WGS84
  // convert waypoint lat/lon to NED
  // It seems that "LocalCartesian.Forward(lat, lon, h, x, y, z)" returns ENU,
  // that is, x: east, y: north, z: up
  ned_frame.Forward(lat, lon, h, *e, *n, temp_up);
  *d = -temp_up;
}

//! \brief Convert latitude and longitude to NED where origin is ECEF
//! \param lat Location lat
//! \param lon Location lon
//! \param h Location height
//! \param[out] n North
//! \param[out] e East
//! \param[out] d Down
//! \param x origin x
//! \param y origin y
//! \param z origin z
static void convertLatLonToNedWithEcefOrigin(flt64_t lat, flt64_t lon, flt64_t h, flt64_t *n, flt64_t *e, flt64_t *d,
    flt64_t x, flt64_t y, flt64_t z) {
  // first convert origin ecef to lat/lon
  flt64_t lat0, lon0, h0;
  GeographicLib::Geocentric earth(GeographicLib::Constants::WGS84_a(),
                                  GeographicLib::Constants::WGS84_f());

  earth.Reverse(x, y, z, lat0, lon0, h0);

  convertLatLonToNed(lat, lon, h, lat0, lon0, n, e, d);
}

//! \brief Get the Haversine distance between two points on a sphere
//! https://www.geeksforgeeks.org/haversine-formula-to-find-distance-between-two-points-on-a-sphere/
//! \param posLat1 point 1 latitude
//! \param posLon1 point 1 longitude
//! \param posLat2 point 2 latitude
//! \param posLon2 point 2 longitude
//! \return distance in meters flt64_t
static flt64_t getHaversineDistance(
  const flt64_t& posLat1, const flt64_t& posLon1,
  const flt64_t& posLat2, const flt64_t& posLon2) {
  // convert to radians for calculation
  const flt64_t currentLat = posLat1 * M_PI / 180;
  const flt64_t currentLon = posLon1 * M_PI / 180;
  const flt64_t targetLat = posLat2 * M_PI / 180;
  const flt64_t targetLon = posLon2 * M_PI / 180;

  flt64_t deltaLat = currentLat - targetLat;
  flt64_t deltaLon = currentLon - targetLon;

  flt64_t a = pow(sin(deltaLat / 2), 2) + pow(sin(deltaLon / 2), 2) * cos(targetLat) * cos(currentLat);
  flt64_t radius = 6371000;  // mean radius of the Earth in meters.
  flt64_t c = 2 * asin(sqrt(a));
  return radius * c;
}

//! \brief Get the Haversine distance with two std::pair lat lon types
//! \param posLatLon1 point 1 std::pair<flt64_t latitude, flt64_t longitude>
//! \param posLatLon2 point 2 std::pair<flt64_t latitude, flt64_t longitude>
//! \return distance in meters flt64_t
static flt64_t getHaversineDistance(
  const std::pair<flt64_t, flt64_t>& posLatLon1,
  const std::pair<flt64_t, flt64_t>& posLatLon2) {
  return getHaversineDistance(posLatLon1.first, posLatLon1.second, posLatLon2.first, posLatLon2.second);
}

//! \brief Get the Haversine distance with two std::pair lat lon types
//! \param posLatLon1 point 1 std::pair<flt64_t latitude, flt64_t longitude>
//! \param posLatLon2 point 2 std::pair<flt64_t latitude, flt64_t longitude>
//! \return distance in meters flt64_t
static flt64_t getHaversineDistance(
  const GeoPosition2D& posLatLon1,
  const GeoPosition2D& posLatLon2) {
  return getHaversineDistance(posLatLon1.geodeticLatitude(), posLatLon1.geodeticLongitude(),
    posLatLon2.geodeticLatitude(), posLatLon2.geodeticLongitude());
}

//! \brief Compare the elevation data from a GlobalPoseReportType to an ElevationVariantTypeUnion in the format
//! of the discriminator defined in the ElevationVariantTypeUnion
//! \param gp The global pose report to compare
//! \param elevation The elevation to compare
//! \return The difference in elevation between the GlobalPoseReport data and the provided elevation
static std::optional<flt64_t> compareGlobalPoseToElevation(const GlobalPoseReportType& gp,
    const ElevationVariantTypeUnion& elevation) {
  switch (elevation._d()) {
    case ElevationVariantTypeEnum::ALTITUDEAGLVARIANT_D:
      return gp.altitudeAGL().has_value() ? std::optional<flt64_t>(gp.altitudeAGL().value()
        - elevation.AltitudeAGLVariantVariant().altitude()) : std::nullopt;
    case ElevationVariantTypeEnum::ALTITUDEASFVARIANT_D:
      return gp.altitudeASF().has_value() ? std::optional<flt64_t>(gp.altitudeASF().value()
        - elevation.AltitudeASFVariantVariant().altitude()) : std::nullopt;
    case ElevationVariantTypeEnum::ALTITUDEGEODETICVARIANT_D:
      return gp.altitudeGeodetic().has_value() ? std::optional<flt64_t>(gp.altitudeGeodetic().value()
        - elevation.AltitudeGeodeticVariantVariant().altitude()) : std::nullopt;
    case ElevationVariantTypeEnum::ALTITUDEMSLVARIANT_D:
      return gp.altitude().has_value() ? std::optional<flt64_t>(gp.altitude().value()
        - elevation.AltitudeMSLVariantVariant().altitude()) : std::nullopt;
    case ElevationVariantTypeEnum::DEPTHVARIANT_D:
      return gp.depth().has_value() ? std::optional<flt64_t>(gp.depth().value()
        - elevation.DepthVariantVariant().depth()) : std::nullopt;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Elevation Variant Subtype is not supported: " << elevation._d())
      return std::nullopt;
  }
}

//! \brief Determine if the elevation contained in the provided global pose report is within the bounds of the
//! provided floor and ceiling elevations (inclusive)
//! \param ceil The ceiling of the elevation boundary
//! \param floor The floor of the elevation boundary
//! \param gpData The global pose report data to compare with the elevation boundary
//! \return Whether the elevation contained in the global pose report is within the elevation boundary
//! specified by the floor and ceiling provided. Returns nullopt if there is insufficient data to make the comparison
static std::optional<bool> isWithinElevationRange(const ElevationVariantType& ceil,
    const ElevationVariantType& floor, const GlobalPoseReportType& gpData) {
  // Check Ceiling Bounds
  auto ceilDiff = compareGlobalPoseToElevation(gpData, ceil.ElevationVariantTypeSubtypes());

  if (!ceilDiff) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Cannot compare conditional ceiling bound with global pose data")
    return std::nullopt;
  }

  if (ceil.ElevationVariantTypeSubtypes()._d() == ElevationVariantTypeEnum::DEPTHVARIANT_D) {
    if (ceilDiff.value() < 0) {
      return false;
    }
  } else {
    if (ceilDiff.value() > 0) {
      return false;
    }
  }

  // Check Floor Bounds
  auto floorDiff = compareGlobalPoseToElevation(gpData, floor.ElevationVariantTypeSubtypes());

  if (!floorDiff) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Cannot compare conditional floor bound with global pose data")
    return std::nullopt;
  }

  if (floor.ElevationVariantTypeSubtypes()._d() == ElevationVariantTypeEnum::DEPTHVARIANT_D) {
    if (floorDiff.value() > 0) {
      return false;
    }
  } else {
    if (floorDiff.value() < 0) {
      return false;
    }
  }

  return true;
}

//! \brief Describes the orientation of an ordered triplet
enum class Orientation {
  COLINEAR,
  CLOCKWISE,
  COUNTERCLOCKWISE
};

//! \brief Finds the orientation of ordered triplet (p, q, r).
//! \param p the first point
//! \param q the second point
//! \param r the third point
//! \return The orientation of the ordered triplet
static Orientation orientation(GeoPosition2D p, GeoPosition2D q, GeoPosition2D r) {
  flt64_t value = (q.geodeticLatitude() - p.geodeticLatitude()) * (r.geodeticLongitude() - q.geodeticLongitude())
                - (q.geodeticLongitude() - p.geodeticLongitude()) * (r.geodeticLatitude() - q.geodeticLatitude());
  if (value == 0) {
    return Orientation::COLINEAR;
  }
  return (value > 0) ? Orientation::CLOCKWISE : Orientation::COUNTERCLOCKWISE;
}

//! \brief Given three potentially collinear GeoPositions p, q, r,
//! the function checks if GeoPosition q lies on the line segment 'pr'.
//! \param p the first point
//! \param q the second point
//! \param r the third point
//! \return whether point q lies on the segment 'pr'
static bool onSegment(GeoPosition2D p, GeoPosition2D q, GeoPosition2D r) {
  return (q.geodeticLongitude() <= std::max(p.geodeticLongitude(), r.geodeticLongitude()) &&
          q.geodeticLongitude() >= std::min(p.geodeticLongitude(), r.geodeticLongitude()) &&
          q.geodeticLatitude() <= std::max(p.geodeticLatitude(), r.geodeticLatitude()) &&
          q.geodeticLatitude() >= std::min(p.geodeticLatitude(), r.geodeticLatitude()));
}

//! \brief Returns true if the line segment 'p1q1' and 'p2q2' intersect.
//! \param p1 the first point of te first line
//! \param q1 the second point of the first line
//! \param p2 p2 the first point of the second line
//! \param q2 q2 the second point of the second line
//! \return whether the line segments intersect
static bool intersects(GeoPosition2D p1, GeoPosition2D q1, GeoPosition2D p2, GeoPosition2D q2) {
  std::array<Orientation, 4> orientations;
  orientations[0] = orientation(p1, q1, p2);
  orientations[1] = orientation(p1, q1, q2);
  orientations[2] = orientation(p2, q2, p1);
  orientations[3] = orientation(p2, q2, q1);

  // General case
  if (orientations[0] != orientations[1] && orientations[2] != orientations[3]) {
    return true;
  }

  // Special Cases
  // p1, q1 and p2 are collinear and
  // p2 lies on segment p1q1
  if (orientations[0] == Orientation::COLINEAR && onSegment(p1, p2, q1)) {
    return true;
  }

  // p1, q1 and p2 are collinear and
  // q2 lies on segment p1q1
  if (orientations[1] == Orientation::COLINEAR && onSegment(p1, q2, q1)) {
    return true;
  }

  // p2, q2 and p1 are collinear and
  // p1 lies on segment p2q2
  if (orientations[2] == Orientation::COLINEAR && onSegment(p2, p1, q2)) {
    return true;
  }

  // p2, q2 and q1 are collinear and
  // q1 lies on segment p2q2
  if (orientations[3] == Orientation::COLINEAR && onSegment(p2, q1, q2)) {
    return true;
  }

  // Doesn't fall in any of the above cases
  return false;
}

//! \brief Checks if the GeoPosition2d pos is within the UMAA::MM::BaseType::PolygonVariantType.
//! \param polygon The UMAA::MM::BaseType::PolygonVariantType area.
//! \param pos The point to check if it is within the polygon.
//! \return True: The point is within the polygon. False: The point is outside the polygon.
static bool isInsidePolygon(const PolygonVariantType &polygon, const GeoPosition2D &pos) {
  // There must be at least 3 vertices in polygon
  if (polygon.referencePoints().size() < 3) {
    return false;
  }

  // Create a GeoPosition for line segment from current position to LON_MAX
  GeoPosition2D extreme(pos.geodeticLatitude(), 180);

  std::size_t intersectionCount = 0;
  std::size_t numPoints = polygon.referencePoints().size();
  // Count intersections of the above line with sides of polygon
  for (std::size_t i = 0; i < numPoints; ++i) {
    std::size_t next = (i + 1) % numPoints;

    // Check if the line segment from 'p' to 'extreme' intersects with the line
    // segment from 'referencePoints[i]' to 'referencePoints[next]'
    if (intersects(polygon.referencePoints().at(i), polygon.referencePoints().at(next), pos, extreme)) {
      if (orientation(polygon.referencePoints().at(i), pos, polygon.referencePoints().at(next))
        == Orientation::COLINEAR) {
        return onSegment(polygon.referencePoints().at(i), pos, polygon.referencePoints().at(next));
      }
      intersectionCount++;
    }
  }

  // In Zone if count is odd, outside otherwise
  return intersectionCount % 2 == 1;
}

//! \brief Checks if the GeoPosition2d pos is within the UMAA::Common::Measurement::Polygon.
//! \param polygon The UMAA::Common::Measurement::Polygon area.
//! \param pos The point to check if it is within the polygon.
//! \return True: The point is within the polygon. False: The point is outside the polygon.
static bool isInsidePolygon(const Polygon &polygon, const GeoPosition2D &pos) {
  // There must be at least 3 vertices in polygon
  if (polygon.referencePoint().size() < 3) {
    return false;
  }

  // Create a GeoPosition for line segment from current position to LON_MAX
  GeoPosition2D extreme(pos.geodeticLatitude(), 180);

  std::size_t intersectionCount = 0;
  std::size_t numPoints = polygon.referencePoint().size();
  // Count intersections of the above line with sides of polygon
  for (std::size_t i = 0; i < numPoints; ++i) {
    std::size_t next = (i + 1) % numPoints;

    // Check if the line segment from 'p' to 'extreme' intersects with the line
    // segment from 'referencePoint[i]' to 'referencePoint[next]'
    if (intersects(polygon.referencePoint().at(i), polygon.referencePoint().at(next), pos, extreme)) {
      if (orientation(polygon.referencePoint().at(i), pos, polygon.referencePoint().at(next))
        == Orientation::COLINEAR) {
        return onSegment(polygon.referencePoint().at(i), pos, polygon.referencePoint().at(next));
      }
      intersectionCount++;
    }
  }

  // In Zone if count is odd, outside otherwise
  return intersectionCount % 2 == 1;
}

//! \brief Normalizes a cyclical value to a range centered about a given value.
//! \param a Value to normalize.
//! \param center Center point of the cycle.
//! \param range Absolute width of the cycle.
static flt64_t normalizeCyclical(flt64_t a, flt64_t center, flt64_t range) {
  if (a > center - range / 2.0 && a < center + range / 2.0) {
    return a;  // common case, and avoids slight double precision shifting
  }
  return a - range * floor((range / 2.0 - center + a) / range);
}

//! \brief Find a point along a bearing from a given lon,lat geolocation using vincenty's direct problem solution.
//! https://en.wikipedia.org/wiki/Vincenty%27s_formulae
//! \param currentVehiclePosition The current lat/lon position of the UxV in degrees.
//! \param azimuthBearing azimuthal bearing in radians between the UxV and the center of the loiter capture radius.
//! \param distance How far out to calculate the new point
//! \return GeoPosition2D.
static GeoPosition2D vincentyDirect(
    const GeoPosition2D& currentVehiclePosition,
    const flt64_t& azimuthBearing,
    const flt64_t& distance) {

  const flt64_t SEMIMINOR_AXIS = GeographicLib::Constants::WGS84_a() * (1 - GeographicLib::Constants::WGS84_f());
  const flt64_t EP2 = (pow(GeographicLib::Constants::WGS84_a(), 2) - pow(SEMIMINOR_AXIS, 2)) / pow(SEMIMINOR_AXIS, 2);
  const flt64_t cosA1 = cos(azimuthBearing);
  const flt64_t sinA1 = sin(azimuthBearing);
  const flt64_t tanU1 =
    (1 - GeographicLib::Constants::WGS84_f()) * tan(DEGREES_TO_RADIANS_ * currentVehiclePosition.geodeticLatitude());
  const flt64_t cosU1 = 1 / sqrt(1 + tanU1 * tanU1);
  const flt64_t sinU1 = tanU1 * cosU1;
  const flt64_t sig1 = atan2(tanU1, cosA1);
  const flt64_t sinAlpha = cosU1 * sinA1;
  const flt64_t cosSqAlpha = 1 - sinAlpha * sinAlpha;
  const flt64_t uSq = cosSqAlpha * EP2;
  const flt64_t A = 1 + uSq / 16384.00 * (4096.00 + uSq * (-768.00 + uSq * (320.00 - 175.00 * uSq)));
  const flt64_t B = uSq / 1024.00 * (256.00 + uSq * (-128.00 + uSq * (74.00 - 47.00 * uSq)));

  flt64_t sigma = distance / (SEMIMINOR_AXIS * A);
  flt64_t sigmaP;
  flt64_t sinSigma;
  flt64_t cosSigma;
  flt64_t cos2SigmaM;
  flt64_t deltaSigma;

  do {
      cos2SigmaM = cos(2 * sig1 + sigma);
      sinSigma = sin(sigma);
      cosSigma = cos(sigma);

      deltaSigma = B * sinSigma * (cos2SigmaM + (B / 4.00) * (
              cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) - (B / 6) * cos2SigmaM * (-3
                      + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));
      sigmaP = sigma;
      sigma = distance / (SEMIMINOR_AXIS * A) + deltaSigma;
  } while (std::fabs(sigma - sigmaP) > 1E-12);

  flt64_t tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosA1;
  flt64_t  lat2 = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosA1,
    (1 - GeographicLib::Constants::WGS84_f()) * sqrt(sinAlpha * sinAlpha + tmp * tmp));

  flt64_t lambda = atan2(sinSigma * sinA1, cosU1 * cosSigma - sinU1 * sinSigma * cosA1);
  flt64_t c = GeographicLib::Constants::WGS84_f() /
    16 * cosSqAlpha * (4 + GeographicLib::Constants::WGS84_f() * (4 - 3 * cosSqAlpha));

  flt64_t lam = lambda - (1 - c) * GeographicLib::Constants::WGS84_f() * sinAlpha * (sigma + c * sinSigma * (cos2SigmaM
    + c * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));

  flt64_t latitude = normalizeCyclical(RADIANS_TO_DEGREES_ * lat2, 0.00, 180);
  flt64_t longitude = normalizeCyclical(currentVehiclePosition.geodeticLongitude() +
    RADIANS_TO_DEGREES_ * lam, 0.00, 360);

  return  GeoPosition2D(latitude, longitude);
}

//! \brief Find the distance between two points using vincenty's inverse problem solution.
//! https://en.wikipedia.org/wiki/Vincenty%27s_formulae
//! \param p1 The first point.
//! \param p2 The second point.
//! \return GeoPosition2D.
static flt64_t vincentyInverse(const GeoPosition2D& p1, const GeoPosition2D& p2) {
  if (p1.geodeticLatitude() == p2.geodeticLatitude()) {
    // Use haversine if the points are on the same line of latitude
    // Vincenty cannot solve or solves very slowly when this is the case
    return arlcore::getHaversineDistance(p1, p2);
  }

  const flt64_t SEMIMINOR_AXIS = GeographicLib::Constants::WGS84_a() * (1 - GeographicLib::Constants::WGS84_f());
  const flt64_t L = (p2.geodeticLongitude() * DEGREES_TO_RADIANS_) -
    (p1.geodeticLongitude() * DEGREES_TO_RADIANS_);

  const flt64_t oF = 1 - GeographicLib::Constants::WGS84_f();
  const flt64_t U1 = atan(oF * tan(p1.geodeticLatitude() * DEGREES_TO_RADIANS_));
  const flt64_t U2 = atan(oF * tan(p2.geodeticLatitude() * DEGREES_TO_RADIANS_));
  const flt64_t sU1 = sin(U1);
  const flt64_t cU1 = cos(U1);
  const flt64_t sU2 = sin(U2);
  const flt64_t cU2 = cos(U2);

  flt64_t sigma;
  flt64_t sinSigma;
  flt64_t cosSigma;
  flt64_t sinAlpha;
  flt64_t cos2Alpha;
  flt64_t cos2SigmaM;
  flt64_t lambda = L;
  flt64_t lambdaP;
  flt64_t iters = 100;
  flt64_t sinLambda;
  flt64_t cosLambda;
  flt64_t c;

  do {
      sinLambda = sin(lambda);
      cosLambda = cos(lambda);
      sinSigma = sqrt((cU2 * sinLambda) * (cU2 * sinLambda) + pow(cU1 * sU2 - sU1 * cU2 * cosLambda, 2));
      if (sinSigma == 0) {
        UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Distance between points are two small to calculate")
        return 0;
      }

      cosSigma = sU1 * sU2 + cU1 * cU2 * cosLambda;
      sigma = atan2(sinSigma, cosSigma);
      sinAlpha = cU1 * cU2 * sinLambda / sinSigma;
      cos2Alpha = 1.00 - sinAlpha * sinAlpha;
      cos2SigmaM = cosSigma - 2.00 * sU1 * sU2 / cos2Alpha;

      c = GeographicLib::Constants::WGS84_f() /
        16.00 * cos2Alpha * (4 + GeographicLib::Constants::WGS84_f() * (4.00 - 3.00 * cos2Alpha));

      lambdaP = lambda;
      lambda = L + (1.00 - c) * GeographicLib::Constants::WGS84_f() * sinAlpha * (sigma + c * sinSigma * (cos2SigmaM
        + c * cosSigma * (-1.00 + 2.00 * cos2SigmaM * cos2SigmaM)));

      --iters;
  } while (std::fabs(lambda - lambdaP) > 1E-12 && iters > 0);

  if (iters == 0) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Lambda was unable to converge within 100 iterations")
    return 0;
  }

  const flt64_t uSq = cos2Alpha * (pow(GeographicLib::Constants::WGS84_a(), 2) - pow(SEMIMINOR_AXIS, 2)) /
    (pow(SEMIMINOR_AXIS, 2));

  const flt64_t A = 1 + uSq / 16384.00 * (4096.00 + uSq * (-768.00 + uSq * (320.00 - 175.00 * uSq)));
  const flt64_t B = uSq / 1024.00 * (256.00 + uSq * (-128 + uSq * (74.00 - 47.00 * uSq)));

  const flt64_t deltaSigma = B * sinSigma * (cos2SigmaM + B /
    4.00 * (cosSigma * (-1.00 + 2.00 * cos2SigmaM * cos2SigmaM) - B /
    6 * cos2SigmaM * (-3.00 + 4.00 * sinSigma * sinSigma) * (-3.00 + 4.00 * cos2SigmaM * cos2SigmaM)));

  return (SEMIMINOR_AXIS * A * (sigma - deltaSigma));
}

//! \brief Checks if a GeoPosition2D position is within a UMAA::MM::BaseType::EllipseVariantType.
//! \param ellipse The UMAA::MM::BaseType::EllipseVariantType area
//! \param pos The position to check whether or not is within the ellipse.
//! \return True: The point is within the ellipse. False: The point is outside the ellipse.
static bool isInsideEllipse(const EllipseVariantType &ellipse, const GeoPosition2D &pos) {
  GeoPosition2D center = ellipse.centerPosition();

  flt64_t distFromCenter = arlcore::getHaversineDistance(center.geodeticLatitude(), center.geodeticLongitude(),
        pos.geodeticLatitude(), pos.geodeticLongitude());

  // If the distance from the center is greater than the largest radius, we are outside the ellipse
  if (distFromCenter > ellipse.semiMajorRadius()) {
    return false;
  }

  // If the distance from the center is less than or equal to the smallest radius, we are inside the ellipse
  if (distFromCenter <= ellipse.semiMinorRadius()) {
    return true;
  }

  // Assuming direction is the angle of major axis
  flt64_t fociDist_m = sqrt(abs(ellipse.semiMinorRadius() * ellipse.semiMinorRadius() - ellipse.semiMajorRadius()
    * ellipse.semiMajorRadius()));

  // Approximate change in degrees for small distances
  flt64_t dLat_m = fociDist_m * cos(ellipse.direction());
  flt64_t dLon_m = fociDist_m * sin(ellipse.direction());

  // foci coordinates of the ellipse
  GeoPosition2D f1 = calculateShift(center, dLat_m, dLon_m);
  GeoPosition2D f2 = calculateShift(center, -dLat_m, -dLon_m);

  // definition of ellipse is the points whose sum of distances to the foci is constant, any distance less than
  // that is in the ellipse
  flt64_t maxSumOfDist = 2 * ellipse.semiMajorRadius();
  flt64_t sumOfDist = getHaversineDistance(pos.geodeticLatitude(), pos.geodeticLongitude(),
    f1.geodeticLatitude(), f1.geodeticLongitude()) + getHaversineDistance(pos.geodeticLatitude(),
    pos.geodeticLongitude(), f2.geodeticLatitude(), f2.geodeticLongitude());


  return sumOfDist <= maxSumOfDist;
}

}  // namespace arlcore
#endif  // INCLUDE_ALGORITHMS_GEOGRAPHICUTILS_H_
