Sample application for AddLive desktop SDK on Qt
==================

Build instructions:

- clone this repo
- get SDK archive (AddLive\_sdk-win.zip) and unpack to cloned repo root (as is, i.e. it should be a directory in the repo root)
- install Qt SDK with QtCreator
- open cdo\_sample\_app/cdo\_sample\_app.pro in QtCreator
- adjust AddLive key, application ID and streamer address in addlivesdkparams.h
- build the project
- copy or move AddLive\_sdk-win/adl\_sdk.dll (libadl\_sdk.so, libadl\_sdk.dylib) to the directory where adl\_sample\_app.exe resides
- now you can run sample application from QtCreator


