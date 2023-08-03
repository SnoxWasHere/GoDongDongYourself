# GoDongDongYourself
You've probably never wanted to put yourself (or anyone else) into the cult classic Chinese 2D Fighter Maker 2002 game Dong Dong Never Die. Now, you can.

## Features
- Tools to extract and repackage your own images into 2D Fighter Maker 2002 character files
- Creates templated spritesheets from exported images

## How to Use
### Prerequisites
- Place selected .player file in the same folder as the program files
- Install [ImageMagick](https://imagemagick.org/script/download.php#windows) for Windows
- Have a suitable raster graphics editor like [GIMP](https://www.gimp.org/downloads/) or Photoshop

### Usage
1. Run `1_PlayerToPng.exe` (admin permissions might be required to open the .player file)
2. Move all of the images that you don't need to change to `./unchanged/`
3. Run `2_PngToGrid.exe`
4. Replace all of the images in the generated spritesheets with your own
    - Keep your sprites within the size of the original (same size as the template generated)
    - Save grid back with same name without template background
    - *No color/alpha reduction is necessary*
5. Run `3_GridToPng.exe` and `4_PngToPlayer.exe`
6. Replace the original .player file with the generated one in the game files

## Examples
![One row of an exported grid](/docs/examplegrid.png)  <img src="/docs/examplecharacter.png" alt="drawing" height="256"/>
## *Upcoming Features*
- Sound effect replacement
- Universal support for other 2D Fighter Maker games
- Stage editing

