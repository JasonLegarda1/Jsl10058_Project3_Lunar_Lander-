# Project 3: Soccer Lander

A soccer-themed Lunar Lander game built with C++ and raylib. Control the falling soccer ball, conserve fuel, and land on the moving Messi platform while avoiding the opponent.

## Controls

- **A / D:** Apply left or right acceleration
- **Q:** Quit

## Game Rules

- Land on top of Messi to complete the mission.
- Hitting Messi incorrectly, touching the opponent, or leaving the map fails the mission.
- Applying horizontal acceleration consumes fuel.

The game uses an `Entity` class, delta time, fixed-timestep physics, gravity, acceleration-based movement, and a sprite-sheet flame animation.

## Build and Run

```sh
make
make run
```
