
import aceutils

aceutils.cdToScript()

aceutils.cd(r'../')
aceutils.rm(r'bullet-2.82-r2704.zip')
aceutils.rmdir(r'bullet-2.82-r2704')

aceutils.wget(r'https://bullet.googlecode.com/files/bullet-2.82-r2704.zip')
aceutils.unzip(r'bullet-2.82-r2704.zip')

aceutils.editCmakeForACE(r'bullet-2.82-r2704/CMakeLists.txt','cp932')

aceutils.rmdir(r"bullet_bin")
aceutils.rmdir(r"bullet_bin_x64")

aceutils.mkdir(r"bullet_bin")
aceutils.mkdir(r"bullet_bin_x64")

aceutils.cd(r"bullet_bin")

if aceutils.isWin():
	aceutils.call(r'cmake -G "Visual Studio 12" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D BUILD_DEMOS:BOOL=OFF ../bullet-2.82-r2704/')
	aceutils.call(r'"C:\Program Files (x86)\MSBuild\12.0\Bin\msbuild" BULLET_PHYSICS.sln /p:configuration=Debug')
	aceutils.call(r'"C:\Program Files (x86)\MSBuild\12.0\Bin\msbuild" BULLET_PHYSICS.sln /p:configuration=Release')
elif aceutils.isMac():
	aceutils.call(r'cmake -G "Unix Makefiles" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D USE_INTERNAL_LOADER:BOOL=OFF "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ../bullet-2.82-r2704/')
	aceutils.call(r'make')
else:
	aceutils.call(r'cmake -G "Unix Makefiles" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF ../bullet-2.82-r2704/')
	aceutils.call(r'make')

aceutils.cd(r"../")

aceutils.cd(r"bullet_bin_x64")

if aceutils.isWin():
	aceutils.call(r'cmake -G "Visual Studio 12 Win64" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D BUILD_DEMOS:BOOL=OFF ../bullet-2.82-r2704/')
	aceutils.call(r'"C:\Program Files (x86)\MSBuild\12.0\Bin\msbuild" BULLET_PHYSICS.sln /p:configuration=Debug')
	aceutils.call(r'"C:\Program Files (x86)\MSBuild\12.0\Bin\msbuild" BULLET_PHYSICS.sln /p:configuration=Release')

aceutils.cd(r"../")


aceutils.copytreeWithExt(r'bullet-2.82-r2704/src/',r'Dev/include/',['.h'])

if aceutils.isWin():

	aceutils.mkdir(r'Dev/lib/x86/')
	aceutils.mkdir(r'Dev/lib/x86/Debug')
	aceutils.mkdir(r'Dev/lib/x86/Release')

	aceutils.mkdir(r'Dev/lib/x64/')
	aceutils.mkdir(r'Dev/lib/x64/Debug')
	aceutils.mkdir(r'Dev/lib/x64/Release')

	aceutils.copy(r'bullet_bin/lib/Debug/BulletCollision_Debug.lib', r'Dev/lib/x86/Debug/')
	aceutils.copy(r'bullet_bin/lib/Debug/LinearMath_Debug.lib', r'Dev/lib/x86/Debug/')

	aceutils.copy(r'bullet_bin/lib/Release/BulletCollision.lib', r'Dev/lib/x86/Release/')
	aceutils.copy(r'bullet_bin/lib/Release/LinearMath.lib', r'Dev/lib/x86/Release/')

	aceutils.copy(r'bullet_bin_x64/lib/Debug/BulletCollision_Debug.lib', r'Dev/lib/x64/Debug/')
	aceutils.copy(r'bullet_bin_x64/lib/Debug/LinearMath_Debug.lib', r'Dev/lib/x64/Debug/')

	aceutils.copy(r'bullet_bin_x64/lib/Release/BulletCollision.lib', r'Dev/lib/x64/Release/')
	aceutils.copy(r'bullet_bin_x64/lib/Release/LinearMath.lib', r'Dev/lib/x64/Release/')

else:

	aceutils.copy(r'bullet_bin/src/BulletCollision/libBulletCollision.a', r'Dev/lib/')
	aceutils.copy(r'bullet_bin/src/LinearMath/libLinearMath.a', r'Dev/lib/')
