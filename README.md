# OSG Chessboard
A working chess board implemented in OpenSceneGraph.

![OSG_Chessboard](https://user-images.githubusercontent.com/4536448/94378038-53e49e80-00e3-11eb-83ea-acfc21301dd0.gif)

## Summary
Some years back, a good friend introduced me to OpenSceneGraph.  As a weekend
"get-to-know-you" project, I wrote a quick OSG-based program that use some
slightly customized 3D objects to display a chess board, complete with pieces.

It wasn't long before I decided to add some behavior to it, and ended up with
a "active" chess board that not only reacted to chess moves, but also assisted
in the actual play.

The code in this repository is the original code, refactored into full C++11.

## Possible Improvements
There's no AI in this code that would allow you to play against the computer.

If you're up to the challenge, possible improvements would be to add such AI,
or to implement network communication to allow two people to play against each
other over the Internet.

## Dependencies
This new version of the program was improved using Qt Creator.  As such, it
uses the Qt build system to produce an executable.  Within the code itself,
however, only OpenSceneGraph is required, so you can replace the build system
with whatever you like.

This updated version was tested with the most current release of
[OSG](http://www.openscenegraph.org/) (v3.6.5) as of the time of this writing.

## Documentation
None really needed.
