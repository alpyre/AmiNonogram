# AmiNonogram
Nonograms, also known as Hanjie, Paint by Numbers, Picross, Griddlers, and
Pic-a-Pix, and by various other names, are picture logic puzzles in which
cells in a grid must be colored or left blank according to numbers at the side
of the grid to reveal a hidden picture (https://en.wikipedia.org/wiki/Nonogram).

AmiNonogram is an open source Workbench Nonogram game for Amiga Computers (and variants).
AmiNonogram forces the players to solve the puzzles in less than three mistakes (lives) and rewards them accordingly.

### Screenshots
![profiles](https://hosting.photobucket.com/images/r610/Alpyre55/AmiNonogram_Profiles.gif)</br>
![gameplay](https://hosting.photobucket.com/images/r610/Alpyre55/AmiNonogram_GifAnim.gif)
![gamewin](https://hosting.photobucket.com/images/r610/Alpyre55/WinScreen.gif)

### Requirements
- AmigaOS 2.0+
- MUI v3.8+

### Features
- Supports square nonogram puzzles from 5x5 upto 50x50 (sizes are always multiple of 5).
- Puzzles come in puzzle packs and users can add more packs to the game in the future.
- Puzzle packs are designed to contain thousands of puzzles.
- Puzzles are introduced in random order.
- Supports multiple player profiles and keeps their progress on disk.
- Fully localizable (comes with a catalog descriptor file).
- A puzzle pack editor/creator (will be published separately in the future).

### Platforms
AmiNonogram source is ready to be built for AmigaOS3.x, AmigaOS4.x and MorphOS.
It utilizes SDI_Headers (http://aminet.net/package/dev/c/SDI_headers) so an AROS
built should be pretty straightforward.

### Binaries
Binaries for OS3.x, OS4.x and MorphOS will be published on Aminet (http://aminet.net/).
http://aminet.net/package/game/think/AmiNonogram

### Build
AmiNonogram is ready to be built using the toolchains below:
- for OS3/OS4: https://github.com/jens-maus/amigaos-cross-toolchain
- for MorphOS: https://github.com/AmigaPorts/morphos-cross-toolchain

Contributions are greatly appreciated. Please fork this repository and open a pull request to add new features and/or bugfixes.
