Sample application for AddLive desktop SDK on Qt
==================

Build instructions:

- clone this repo
- get SDK archive (AddLive\_sdk-win.zip) and unpack to cloned repo root (as is, i.e. it should be a directory in the repo root)
- install Qt SDK with QtCreator (Qt 4.8.x only is supported at the moment)
- open cdo\_sample\_app/cdo\_sample\_app.pro in QtCreator
- adjust AddLive key, application ID and streamer address in addlivesdkparams.h
- build the project
- copy AddLive\_sdk-win directory from SDK package to output directory (where cdo\_sample\_app.exe resides)
- copy or move AddLive\_sdk-win/adl\_sdk.dll to the directory where cdo\_sample\_app.exe resides
- now you can run sample application from QtCreator

Currently works only on Windows XP, Vista, 7, 8 and for two users in the scope maximum.

