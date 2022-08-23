@echo off
setlocal

pushd %~dp0\..

rmdir /s /q build

popd

endlocal

exit /b %ErrorLevel%
