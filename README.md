# YouHaveToFinishTheGame

My first -incomplete- Raylib 2D platformer inspired by "You have to win the game". 
It turns out I'm really bad a level design, and I'm learning a lot about what 
not to do when programming; it's all here as a time capsule of game dev mistakes.

## How to Build
 
1. Make sure you have Raylib installed.
2. Run 'build.sh' in the terminal.
3. Run the executable in the 'out/' folder.
 
## Controls
 
- Arrow Keys, WASD, or Joystick D-Pad to move.
- Press SpaceBar, or Joystick A button or X button to Jump.
- Hold Shift, or Joystick X button or Square button to Run.
 
## Features
 
- Multiple Levels.
- Basic enemy AI.
- Collectible Coins.
- Projectiles and Obstacles.
- Checkpoints.
- Sound Effects.
- Player can make levels that are of multiples of 40x24 pixels in size and save them as a .png.
- Player can make levels using MSPaint to manipulate the colors in the .png files in the 'out/' folder.

    * rgb (  0,   0, 255) the player tile.
    * rgb (  0,   0,   0) empty space.
    * rgb (  0, 162, 232) vertical monsters tile.
    * rgb (255, 100,   0) horizontal monsters tile.
    * rgb (  0, 255,   0) checkpoints tile.
    * rgb (150, 150,   0) coins tile (called 'treasure' in the src file).
    * rgb (255,   0,   0) spikes.
    * rgb (255,   0, 255) projectiles.
    * rgb (  0, 128, 128) rotating pillars.

    * rgb ( 20,  20,  20) wall tile 1.
    * rgb ( 40,  40,  40) wall tile 2.
    * rgb ( 60,  60,  60) wall tile 3.
    * rgb ( 80,  80,  80) wall tile 4.
    * rgb (255, 255, 255) wall tile 5.

## TODO

- Add music.
- Add more stages.
- Every stage should have its own time.
- Add variable jump height.
- Make enemies and projectiles collide with walls instead of passing through them.
- Add Leader Board.
- More or Better animations and graphics.
- Separate the game objects into their own modules instead of having a single main file doing everything.
- Add a start screen and end game screen.
    
## Known Issues

- None so far

## Licence

MIT License -- feel free to use and modify!
