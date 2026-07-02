#!/usr/bin/env python3
# ---------------------------------------------------------------------------
# Copyright 2025 Pennsylvania State University
#
# Applied Research Laboratory
# Pennsylvania State University
# P.O. Box 30
# State College, PA 16804-0030
#
# DISTRIBUTION STATEMENT A. Approved for public release.
# Distribution is unlimited.
# This software was developed by the Department of the Navy,
# NAVSEA Unmanned and Small Combatants. It is provided under the terms of
# use found in the LICENSE file at the source code root directory.
# ---------------------------------------------------------------------------
"""Plot a recorded autopilot mission: the vehicle track vs the planned waypoints.

Usage: plot_mission.py <mission-out-dir> [output.png]

Reads track.csv (elapsed_s, lat_deg, lon_deg, yaw_rad, speed_mps) and waypoints.csv
(index, lat_deg, lon_deg, capture_radius_m) as written by mission_runner, and renders:
  - the ground track with the planned waypoints and their capture radii,
  - north/east position components over time,
  - speed over ground over time.
"""
import csv
import math
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402


def read_csv(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def main():
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "mission-out")
    out_png = Path(sys.argv[2] if len(sys.argv) > 2 else out_dir / "mission_plot.png")

    track = read_csv(out_dir / "track.csv")
    waypoints = read_csv(out_dir / "waypoints.csv")
    if not track:
        sys.exit("track.csv is empty")

    # Local equirectangular projection about the first fix (meters east/north).
    lat0 = float(track[0]["lat_deg"])
    lon0 = float(track[0]["lon_deg"])
    m_per_deg_lat = 111132.0
    m_per_deg_lon = 111320.0 * math.cos(math.radians(lat0))

    def to_local(lat, lon):
        return (lon - lon0) * m_per_deg_lon, (lat - lat0) * m_per_deg_lat

    t = [float(r["elapsed_s"]) for r in track]
    east, north = zip(*[to_local(float(r["lat_deg"]), float(r["lon_deg"])) for r in track])
    speed = [float(r["speed_mps"]) for r in track]
    wp_e, wp_n, wp_r = [], [], []
    for r in waypoints:
        e, n = to_local(float(r["lat_deg"]), float(r["lon_deg"]))
        wp_e.append(e)
        wp_n.append(n)
        wp_r.append(float(r["capture_radius_m"]))

    fig = plt.figure(figsize=(14, 9), constrained_layout=True)
    grid = fig.add_gridspec(2, 2, width_ratios=[1.4, 1.0])
    fig.suptitle("Autopilot waypoint mission — simulated vehicle", fontsize=14)

    # Ground track vs planned waypoints.
    ax = fig.add_subplot(grid[:, 0])
    ax.plot(east, north, color="#4269d0", linewidth=1.6, label="vehicle track", zorder=3)
    ax.plot(east[0], north[0], marker="^", color="#3ca951", markersize=10,
            linestyle="none", label="start", zorder=5)
    ax.plot(east[-1], north[-1], marker="s", color="#ff725c", markersize=8,
            linestyle="none", label="end", zorder=5)
    ax.plot(wp_e, wp_n, linestyle="--", color="#9c6b4e", linewidth=1.0, alpha=0.7,
            label="planned route (waypoint order)", zorder=2)
    for i, (e, n, r) in enumerate(zip(wp_e, wp_n, wp_r)):
        ax.add_patch(plt.Circle((e, n), r, facecolor="none", edgecolor="#efb118",
                                linewidth=1.4, zorder=4))
        ax.annotate(f"WP{i + 1}", (e, n), textcoords="offset points", xytext=(8, 8),
                    fontsize=10, color="#6f6f6f")
    ax.plot([], [], color="#efb118", label="capture radius")
    ax.set_xlabel("east (m)")
    ax.set_ylabel("north (m)")
    ax.set_title("Ground track vs planned waypoints")
    ax.set_aspect("equal", adjustable="datalim")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", fontsize=9)

    # Position components over time.
    ax = fig.add_subplot(grid[0, 1])
    ax.plot(t, north, color="#4269d0", linewidth=1.4, label="north (m)")
    ax.plot(t, east, color="#ff725c", linewidth=1.4, label="east (m)")
    ax.set_xlabel("elapsed time (s)")
    ax.set_ylabel("position (m)")
    ax.set_title("Position over time")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", fontsize=9)

    # Speed over time.
    ax = fig.add_subplot(grid[1, 1])
    ax.plot(t, speed, color="#3ca951", linewidth=1.4)
    ax.set_xlabel("elapsed time (s)")
    ax.set_ylabel("speed over ground (m/s)")
    ax.set_title("Speed over time")
    ax.grid(True, alpha=0.3)

    fig.savefig(out_png, dpi=150)
    print(f"wrote {out_png}")


if __name__ == "__main__":
    main()
