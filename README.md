## THE ULKA GAME - Our take on classic game ASTEROID (Raylib)
Supervisor : Md. Imtiaz Kabir
Members : 
- 1. Kasita Nusaiba Takee (2505130) 
- 2. Adreeta Afia Anam (2505144)

## Overview:
An Asteroid-style shooter game with a few updates and our own twists. There are a few stages in this game which will give you the feel of a whole journey in the space hopefully. This was wholly written in C using Raylib library

## Features:
- Frame-rate-independent shooter spaceship with rotation and thrust with a drag with screen-wrapping 
- Animated bullets
- Random alien appearances
- Storyline
- Power-ups - Shield, Extra lives, Destroy all, Mega shot
- Space backgrounds

## Controls:
- Rotate : Left/Right arrows
- Thrust: Up arrow
- Shoot: Space
- Pause : P
- Sound On/Off : S
- Return to Menu : M
- Quit: Esc

## Dependencies:
Windows 10/11 (64-bit)
w64devkit (GCC, 64-bit) installed at C:\w64devkit
Raylib 6.0 (win64_mingw-w64 build), placed at raylib/raylib-6.0_win64_mingw-w64/

## Setup:
1. Extract the ZIP.
2. Download Raylib 6.0 "win64_mingw-w64" from https://github.com/raysan5/raylib/releases and extract it into `raylib/` so the folder is `raylib/raylib-6.0_win64_mingw-w64/` (skip if already included).
3. Keep the `resources/` folder next to `spaceship.c`.
4. Keep the `audio/` folder next to `spaceship.c`, as the game loads music and sound effects from this folder.
5. Keep the `hall_of_fame.txt` file in the project root, as it is used to store Hall of Fame scores.
6. If w64devkit is installed elsewhere, change the path below.

## Compile:
C:\w64devkit\bin\gcc.exe spaceship.c -o spaceship.exe -Iraylib/raylib-6.0_win64_mingw-w64/include raylib/raylib-6.0_win64_mingw-w64/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm

## Run:
Run `spaceship.exe` from the project root so that the relative paths to the `resources/` and `audio/` folders work correctly.

## Credits:
## Sound and Music :
PIXABAY ARTISTS: ATLASAUDIO, CLAVIER MUSIC, DELOSOUND, DRAGON-STUDIO, FINNTASTICO, FREESOUND_COMMUNITY, MONDAMUSIC, NR-MUSIC, POORARTISTT ("VIDEOGAME-POWER-UP-SOUND-EFFECT-01"), TOKYORIFFT
TRACKS:
"SIGN OF THE TIMES" — WRITTEN BY HARRY STYLES, JEFF BHASKER, ALEX SALIBIAN, TYLER JOHNSON, MITCH ROWLAND, & RYAN NASCI. ℗ COLUMBIA RECORDS / ERSKINE RECORDS. COURTESY OF SONY MUSIC PUBLISHING / GLOBAL MUSIC RIGHTS.
"BLUEBERRY" — COMPOSED & PRODUCED BY VIKTOR KRAUS
SPACE MUSIC PACK (ALIEN TALK & SCREAM): GOOSE NINJA

## Art and Visuals :
SPACE BACKGROUNDS:SCREAMING BRAIN STUDIOS (CC0)
SPACE ASSETS: SCRATCHBATTLE
BACKGROUNDS: MAGNIFIC, VENTUREBIT , ROCKY
ARTISTS: FERRET, NORMA2D
