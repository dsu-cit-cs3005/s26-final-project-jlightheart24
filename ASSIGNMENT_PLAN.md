# RobotWarz Implementation Checklist

This file turns the assignment into a build order. The goal is to finish each section with working code before moving to the next one.

## Files To Own

- `Arena.h`
  - Own the arena data model.
  - Keep `Config`, `RobotEntry`, `Cell`, and helper method declarations here.
- `Arena.cpp`
  - Own config parsing, board setup, obstacle placement, robot loading, turn execution, radar, movement, shooting, and printing.
- `RobotWarz.cpp`
  - Keep this small. It should only parse `argv`, load config, initialize the arena, and run the game.
- `Makefile`
  - Build `RobotWarz`, `test_robot`, and `RobotBase.o`.
- `robots/`
  - Put all arena-loadable robot sources here.
  - Move or copy `Robot_Ratboy.cpp` and `Robot_Flame_e_o.cpp` here when you are ready to test dynamic loading.

## Phase 1: Buildable Skeleton

- [x] Add a `RobotWarz` entry point.
- [x] Replace the placeholder `Arena` files with a real class shell.
- [x] Parse the assignment config format.
- [x] Initialize the board and place obstacles.
- [x] Print the board and robot status area.
- [x] Create a sample `config.txt` for local testing.

## Phase 2: Robot Discovery And Loading

- [x] Create a `robots/` subdirectory.
- [ ] Scan `robots/` for files named `Robot_*.cpp`.
- [ ] Compile each source file into a shared object using `RobotBase.o`.
- [ ] Load each `.so` with `dlopen`.
- [ ] Resolve `create_robot` with `dlsym`.
- [ ] Instantiate each robot and store it in `m_robots`.
- [ ] Assign a display character to each robot.
- [ ] Call `set_boundaries(height, width)` on each robot.
- [ ] Randomly place each robot on an empty non-obstacle cell.
- [ ] Add cleanup for robot instances and shared library handles.

## Phase 3: Turn Loop

- [ ] Loop until one robot remains alive or `Max_Rounds` is reached.
- [ ] Skip dead robots.
- [ ] Print round header and board each turn.
- [ ] Check for winner before processing the current robot.
- [ ] Add per-turn logging text similar to the sample output.
- [ ] Respect `Sleep_interval` when `Game_State_Live:true`.

## Phase 4: Radar

- [ ] Implement adjacent scan for direction `0`.
- [ ] Implement 3-cell-wide ray scans for directions `1` through `8`.
- [ ] Stop scans at arena boundaries.
- [ ] Exclude the scanning robot itself.
- [ ] Encode visible items as `RadarObj` values with `R`, `X`, `M`, `P`, and `F`.
- [ ] Pass the results to `process_radar_results`.

## Phase 5: Movement

- [ ] Call `get_move_direction` only if the robot did not shoot.
- [ ] Cap requested distance to the robot's current move speed.
- [ ] Move one cell at a time.
- [ ] Stop before mounds.
- [ ] Stop before live robots.
- [ ] Stop before dead robots.
- [ ] Trap robots in pits by calling `disable_movement()`.
- [ ] Allow robots to move through flamethrower cells while taking damage.
- [ ] Preserve the flamethrower tile unless a robot dies on it.

## Phase 6: Weapons And Damage

- [ ] Centralize random damage rolls by weapon type.
- [ ] Apply armor reduction before health loss.
- [ ] Reduce armor by 1 after each hit, without going below 0.
- [ ] Mark robots dead when health reaches 0.
- [ ] Railgun:
  - [ ] Trace the line to the board edge.
  - [ ] Hit every robot in that line.
- [ ] Flamethrower:
  - [ ] Build a 3-cell-wide, 4-cell-long attack area.
  - [ ] Hit every robot inside the area.
- [ ] Grenade launcher:
  - [ ] Allow any target cell on the board.
  - [ ] Damage the 3x3 area centered on the target.
  - [ ] Consume grenades and respect the grenade limit behavior from `RobotBase`.
- [ ] Hammer:
  - [ ] Choose a short-range interpretation and document it in code comments or README notes.

## Phase 7: Verification

- [ ] Build `test_robot` and confirm sample robots still work with the starter harness.
- [ ] Build `RobotWarz`.
- [ ] Run with a tiny config and no sleep.
- [ ] Verify robot spawn never lands on obstacles.
- [ ] Verify pits trap robots permanently.
- [ ] Verify mounds block movement.
- [ ] Verify dead robots block movement.
- [ ] Verify flamethrower obstacles damage moving robots.
- [ ] Verify railgun can hit multiple robots.
- [ ] Verify grenade damages a 3x3 area.
- [ ] Verify winner detection stops the game.

## Known Spec Gaps

- `RobotBase` starts grenade robots with `15` grenades, while the spec text says grenade launchers are limited to `10` shots.
- The spec text around move and armor constraints does not match the exact enforcement in `RobotBase`.
- Hammer range and pathing are not fully spelled out in the spec, so you should pick a consistent interpretation and keep it documented.
