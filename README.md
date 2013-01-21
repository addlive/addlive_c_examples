Sample application for AddLive desktop SDK on Qt
==================

Build instructions:

- clone this repo
- download https://s3.amazonaws.com/api.addlive.com/beta/AddLive\_sdk-win.zip and unpack to cloned repo root ( as is, i.e. it should be a directory in the repo root)
- install Qt SDK with QtCreator
- open cdo\_sample\_app/cdo\_sample\_app.pro in QtCreator
- download boost library (boost.org)
- adjust boost headers in cdo\_sample\_app.pro
- build the project
- copy AddLive\_sdk-win directory from SDK package to output directory (where cdo\_sample\_app.exe resides)
- copy or move AddLive\_sdk-win/adl\_sdk.dll to the directory where cdo\_sample\_app.exe resides
- copy QtCore4.dll and QtGui4.dll to the executable directory
- now you can run sample application

Currently works only on Windows XP, Vista, 7, 8.

