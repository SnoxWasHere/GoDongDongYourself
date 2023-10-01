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
1. Place your .player file in the same directory as the executable
2. Run `(1) - PlayerToPng`
3. Move all images you don't wish to change to `./unchanged/`
4. Run `(2) - PngToGrid`
5. Replace all of the images in the generated spritesheets with your own
    - Keep your sprites within the size of the original (same size as the template generated)
    - Save grid image to same file without template background
    - *No color/alpha reduction is necessary*
6. Run `(3) - GridToPng` and `(4) - PngToPlayer`
7. Replace the original .player file with the generated one in the game files

## Examples
![One row of an exported grid](/docs/examplegrid.png)  
<img src="/docs/examplecharacter.png" alt="drawing" height="330"/>
## *Upcoming Features*
- Sound effect replacement (coming soon!)
- Universal support for other 2D Fighter Maker games
- Stage editing

