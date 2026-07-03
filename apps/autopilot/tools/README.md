# Autopilot tools

## `mission_runner`

End-to-end waypoint mission driver: the UMAA *consumer* side of the autopilot's Global
Waypoint control service. It publishes a `GlobalWaypointCommandType` addressed to the
autopilot's waypoint provider (`identity.waypoint_source_id` from the YAML) together with its
large-list route, then records the vehicle's Global Pose track and the command status until
the mission reaches a terminal state (COMPLETED / FAILED / CANCELED, or a 20-minute timeout).

The route is a closed loop with turns in both directions, laid out relative to the sim
vehicle's configured start position.

```bash
# terminal 1 (from the build directory): the autopilot with the sim vehicle
./apps/autopilot/autopilot autopilot.yaml

# terminal 2: run the built-in mission and record outputs
./apps/autopilot/mission_runner autopilot.yaml mission-out

# ... or fly a custom route (local tangent-plane CSV, one waypoint per line:
# east_m,north_m,speed_mps,capture_radius_m[,arrival_yaw_rad][,elev_value_m,elev_frame]
# where elev_frame is `depth` or `asf`)
./apps/autopilot/mission_runner autopilot.yaml mission-out my-mission.csv
```

Outputs in the chosen directory:

- `track.csv` — `elapsed_s, lat_deg, lon_deg, yaw_rad, speed_mps, depth_m, alt_asf_m`
  sampled from the SA Global Pose / Speed reports.
- `waypoints.csv` — the commanded route: `index, lat_deg, lon_deg, capture_radius_m,
  arrival_yaw_rad, elev_value_m, elev_frame`.
- `planned_path.csv` — the ideal planned Dubins route (`lat_deg, lon_deg` samples), for
  comparing the executed track against the plan.
- `status.log` — command status transitions and the final state.

Exit code 0 iff the command COMPLETED.

## `plot_mission.py`

Renders the recorded mission (requires python3 + matplotlib):

```bash
python3 ../apps/autopilot/tools/plot_mission.py mission-out mission-out/mission_plot.png
```

Produces a figure with the ground track vs the planned waypoints (with capture radii), the
north/east position components over time, and speed over ground over time.
