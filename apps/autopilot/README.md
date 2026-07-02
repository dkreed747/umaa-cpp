# Autopilot Application

An autopilot built on the umaa-cpp SDK. It consumes UMAA MO **Global Vector** and **Global
Waypoint** driving commands, deconflicts them over a single driving resource, and drives a
swappable vehicle-control strategy off the latest SA navigation data.

## Architecture

Three layers (namespace `arlcore::autopilot`):

1. **`AutopilotApp`** — aggregator/main class. Owns the DDS participant, the three SA nav
   consumers (Global Pose / Speed / Velocity), the two MO command providers, the autopilot
   brain, the vehicle-control strategy, and the platform report providers. Runs one control
   loop (`step()` per tick).
2. **`AutopilotBrain` (`IAutopilot`)** — the shared driving controller. Holds the active mode
   and setpoint, recomputes a `ControlVector` from the latest nav on every pose update, and
   pushes it to the vehicle. Owns the `DrivingResourceArbiter`.
3. **`IVehicleControl` strategy** — hardware abstraction (`SimVehicleControl` provided). Sends
   the control vector and serves the platform specs/capabilities. The sim strategy keeps an
   internal kinematic vehicle (limits from the platform capabilities), integrates it on its
   own thread at `vehicle_control.sim.cycle_rate_hz` acting on the latest setpoint, and
   publishes the three SA navigation reports (Global Pose / Speed / Velocity) — closing the
   control loop exactly as a real vehicle's navigation suite would.

Key components:

- **`DrivingResourceArbiter`** — priority deconfliction. Vector is high priority and preempts
  an active waypoint route (-> `FAILED`/`INTERRUPTED`); a waypoint arriving while a vector
  drives is rejected (-> `FAILED`/`RESOURCE_REJECTED`). Priorities are configurable.
- **`VectorControlServiceProvider` / `WaypointControlServiceProvider`** — UMAA command
  providers (on `CommandProviderBase`). Vector is mostly pass-through with validation against
  the platform speed limit. Waypoint reads its route from the large-list element topic, plans a
  Dubins path, and reports capture progress.
- **`DubinsPathPlanner`** — a true Dubins planner. Every leg (previous waypoint or the
  plan/replan pose, to the next waypoint) is solved as the shortest curvature-bounded Dubins
  path over all six words (LSL/RSR/LSR/RSL/RLR/LRL, closed forms in `DubinsPath`), using the
  platform turn radius (speed / max turn rate). The vehicle follows the planned path with a
  pure-pursuit carrot at `lead_distance_m`, arriving at each waypoint on its commanded
  attitude (waypoints without one get a natural fly-through heading). Elevation and speed are
  passed through per waypoint. Capture is evaluated continuously inside the capture zone;
  exiting the zone without a clean capture — or overflying the planned path without ever
  entering it — counts as a miss and replans the leg from the live pose (a loop-back/spiral),
  bounded by `max_misses_per_waypoint` and `max_replans`. On completion the planner commands
  zero speed.

Navigation drives the control tick: the pose observer fires inside the nav consumer's
`cycle()` and triggers the brain's recompute, so control always uses the latest fix. The
design is single-threaded (one control loop); the mutexes are defensive.

## Configuration

All parameters — including the platform specs/capabilities (which nothing else currently
publishes) — are loaded from YAML (see `config/autopilot.yaml`). On startup the platform
specs and capabilities reports are published once. Capabilities also feed the planner
(turn radius = speed / max turn rate).

## Build

The app builds as part of the SDK when `BUILD_AUTOPILOT_APP=ON` (default). It requires the
SDK's toolchain (CycloneDDS-CXX, GeographicLib, log4cxx, yaml-cpp, and the generated
`umaa-cyclone-cxx-types`), which is provided by the SDK development container.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_AUTOPILOT_APP=1 -DTEST_COMMON=1
make -j"$(nproc)"
ctest --verbose            # runs autopilot_test among others
./autopilot autopilot.yaml # run the app
```

## Tests

Unit tests live under `test/` (GoogleTest), enabled with `-DTEST_COMMON=1`:

- `DubinsPathTest` — Dubins solver: word selection, degenerate cases, and randomized
  endpoint correctness (2000 configurations).
- `DubinsPathPlannerTest` — route following to completion, arrival-attitude capture,
  loop-around for waypoints behind the vehicle, miss/replan budgets, progress metrics.
- `SimVehicleControlTest` — kinematic limits (turn rate, acceleration, speed caps) and the
  three nav reports, using the SDK's `LocalReaderSender` loopback IO.
- `DrivingResourceArbiterTest` — priority/preempt/reject/release behavior.
- `YamlConfigLoaderTest` — config parsing and defaults.

The full suite (solver randomized endpoint checks, planner follow/replan/miss behavior with a
kinematic sim vehicle, `SimVehicleControl` kinematics + report publishing, arbiter, config)
runs green with `ctest`. An end-to-end waypoint mission over Cyclone DDS is exercised by
`tools/mission_runner` (see `tools/README.md`), which publishes a `GlobalWaypointCommandType`
plus its large-list route, records the vehicle track from the Global Pose reports, and exits
when the command completes.

A recorded end-to-end run (5-waypoint closed loop, sim vehicle at 3 m/s over Cyclone DDS on
one host, command reached COMPLETED in 597 s with no misses or replans) lives in
`docs/mission-results/`: the track/waypoint CSVs, the command status log, and the rendered
plot below.

![Recorded waypoint mission](docs/mission-results/mission_plot.png)
