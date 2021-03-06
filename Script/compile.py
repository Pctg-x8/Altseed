import os.path
import aceutils

aceutils.cdToScript()
aceutils.cd(r'../')

aceutils.rmdir(r'Dev/cmake')
aceutils.mkdir(r'Dev/cmake')

aceutils.cd(r'Dev/cmake')

if aceutils.isWin():
	aceutils.call(r'cmake -G "Visual Studio 12" -D BUILD_SHARED_LIBS:BOOL=OFF -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF ../')
else:
	if aceutils.isMac():
		aceutils.call(r'cmake -G "Unix Makefiles" -D BUILD_SHARED_LIBS:BOOL=OFF -D CMAKE_INSTALL_PREFIX:PATH=../ "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ../')
	else:
		aceutils.call(r'cmake -G "Unix Makefiles" -D BUILD_SHARED_LIBS:BOOL=OFF -D CMAKE_INSTALL_PREFIX:PATH=../ ../')
	aceutils.call(r'make install')
	aceutils.cd(r'../../')
	aceutils.copytree(r'Dev/bin/Data',r'Dev/cmake/bin/Data')

