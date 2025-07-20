# YouHaveToFinishTheGame

My first 2D platformer inspired by "You have to win the game"

## How to Build
 
1. Make sure you have Raylib installed.
2. Run 'build.sh' in the terminal.
3. Run the executable in the 'out/' folder.
 
## Controls
 
- Arrow Keys, WASD, or Joystick D-Pad to move
- Press SpaceBar, or Joystick A or X button to Jump 
- Hold Shift or Joystick Square or X button to Run
- Joystick Circle Button or B button shows tile coordinates
 
## Features
 
- Multiple Levels
- Basic enemy AI
- Collectible Coins
- Projectiles and Obstacles
- Checkpoints
- Sound Effects
- Player can make levels using MSPaint to manipulate the colors in the .png files in the 'out/' folder

    * rgb (  0,   0, 255) the player tile.
    * rgb (  0,   0,   0) empty space
    * rgb (  0, 162, 232) vertical monsters
    * rgb (  0, 255,   0) checkpoints
    * rgb (255, 100,   0) horizontal monsters
    * rgb (150, 150,   0) coins (called 'treasure' in the src file)
    * rgb (255,   0,   0) spikes
    * rgb (255,   0, 255) projectiles
    * rgb (  0, 128, 128) rotating pillars

    * rgb ( 20,  20,  20) wall type 1
    * rgb ( 40,  40,  40) wall type 2
    * rgb ( 60,  60,  60) wall type 3
    * rgb ( 80,  80,  80) wall type 4
    * rgb (255, 255, 255) wall type 5

## TODO

- Add music
- Add more stages
- Every stage should have its own time
- Add variable jump height
- Make enemies and projectiles collide with walls instead of passing through them
- Add Leader Board
- More or Better animations and graphics
- 
    
## Known Issues

- Enemies, projectiles and pillars get initialized to the top left corner of the screen

## Licence

MIT License -- feel free to use and modify!
