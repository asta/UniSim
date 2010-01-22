@set QTENV="C:\Qt\2009.05\bin\qtenv.bat"

@if exist %QTENV% goto :ok
@echo .
@echo Batch file (build_all.bat) could not find %QTENV%
@echo Edit build_all.bat to fix this and try again
@goto :end

:ok
@call %QTENV%
@call clean_ephemerals.bat
@call clean_targets.bat
qmake build_all.pro
@echo .
@echo *** Finished qmake ***
@echo .

..\vendor\gnu_make\make
@echo .
@echo *** Finished make ***
@echo .

@call clean_ephemerals.bat
@echo .
@echo *** Finished build ***
@echo .

:end
pause