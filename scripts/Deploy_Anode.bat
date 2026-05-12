@echo off
chcp 65001 >nul

:: 青海阳极卸车视觉系统 一键部署脚本（Anode）
:: 请在仓库根目录运行（包含 Anode 目录）
cd /d "%~dp0"
echo 当前工作目录: %CD%

:: 基本变量
set PROJECT_DIR=Anode
set PRODUCT_NAME=Anode.exe
set REMOTE_DIR=deploy_anode_release
set QT_DIR=C:\Qt\6.7.3\msvc2019_64

echo.
echo ============================
echo Anode 部署脚本
echo ============================
echo.

echo 步骤1: 查找 Release 构建产物...
set BUILD_DIR=

:: 常见路径 1（推荐）
echo 检查路径: %PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release\release\%PRODUCT_NAME%
if exist "%PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release\release\%PRODUCT_NAME%" (
    set BUILD_DIR=%PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release\release
    echo 找到构建目录: %BUILD_DIR%
    goto :found_build
) else (
    echo 路径不存在
)

:: 常见路径 2（无 release 子目录）
echo 检查路径: %PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release\%PRODUCT_NAME%
if exist "%PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release\%PRODUCT_NAME%" (
    set BUILD_DIR=%PROJECT_DIR%\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release
    echo 找到构建目录: %BUILD_DIR%
    goto :found_build
) else (
    echo 路径不存在
)

:: 兜底：在项目内所有 *Release* 目录中搜索
echo 未找到固定路径，尝试通配搜索...
for /d %%i in (%PROJECT_DIR%\build\*Release*) do (
    if exist "%%i\%PRODUCT_NAME%" (
        set BUILD_DIR=%%i
        echo 找到构建目录: %BUILD_DIR%
        goto :found_build
    )
    if exist "%%i\release\%PRODUCT_NAME%" (
        set BUILD_DIR=%%i\release
        echo 找到构建目录: %BUILD_DIR%
        goto :found_build
    )
)

echo.
echo 未找到 Release 产物，请手动输入构建目录（包含 %PRODUCT_NAME%）
set /p BUILD_DIR=请输入构建目录路径: 
if not exist "%BUILD_DIR%\%PRODUCT_NAME%" (
    echo 错误: 在指定路径中未找到 %PRODUCT_NAME%
    pause
    exit /b 1
)

:found_build
echo.
echo Release 目录: %BUILD_DIR%

echo.
echo 步骤2: 准备部署目录 %REMOTE_DIR% ...
if exist "%REMOTE_DIR%" (
    echo 清理旧目录...
    rmdir /s /q "%REMOTE_DIR%"
)
mkdir "%REMOTE_DIR%"
if %ERRORLEVEL% neq 0 (
    echo 错误: 创建部署目录失败
    pause
    exit /b 1
)

echo.
echo 步骤3: 复制主程序...
copy "%BUILD_DIR%\%PRODUCT_NAME%" "%REMOTE_DIR%\%PRODUCT_NAME%" >nul
if %ERRORLEVEL% neq 0 (
    echo 错误: 复制主程序失败
    echo 源: %BUILD_DIR%\%PRODUCT_NAME%
    echo 目标: %REMOTE_DIR%\%PRODUCT_NAME%
    pause
    exit /b 1
)

echo.
echo 步骤4: 使用 windeployqt 部署 Qt 运行库...
if not exist "%QT_DIR%\bin\windeployqt.exe" (
    echo 错误: 未找到 windeployqt.exe: %QT_DIR%\bin\windeployqt.exe
    echo 可修改脚本中的 QT_DIR 或手动输入
    set /p QT_DIR=请输入Qt安装路径(包含 bin\windeployqt.exe): 
    if not exist "%QT_DIR%\bin\windeployqt.exe" (
        echo 错误: 仍未找到 windeployqt.exe
        pause
        exit /b 1
    )
)
pushd "%REMOTE_DIR%"
"%QT_DIR%\bin\windeployqt.exe" %PRODUCT_NAME%
if %ERRORLEVEL% neq 0 (
    echo 警告: windeployqt 执行失败，请稍后手动补齐Qt依赖
)
popd

echo.
echo 步骤5: 复制项目自带第三方 DLL（Anode\lib）...
if exist "%PROJECT_DIR%\lib" (
    for %%f in ("%PROJECT_DIR%\lib\*.dll") do (
        copy "%%~ff" "%REMOTE_DIR%\" >nul
    )
    echo 已复制 Anode\lib 下的 DLL
) else (
    echo 未找到 %PROJECT_DIR%\lib，跳过
)

echo.
echo 步骤6: 复制配置文件...
if exist "bin\config" (
    mkdir "%REMOTE_DIR%\config" 2>nul
    xcopy "bin\config\*" "%REMOTE_DIR%\config\" /E /Y /Q >nul
    echo 已复制 bin\config
) else if exist "%PROJECT_DIR%\bin\config" (
    mkdir "%REMOTE_DIR%\config" 2>nul
    xcopy "%PROJECT_DIR%\bin\config\*" "%REMOTE_DIR%\config\" /E /Y /Q >nul
    echo 已复制 Anode\bin\config
) else (
    echo 未找到配置目录，创建空的 config 目录
    mkdir "%REMOTE_DIR%\config" 2>nul
)

echo.
echo 步骤7: 创建日志与数据目录...
mkdir "%REMOTE_DIR%\logs" 2>nul
mkdir "%REMOTE_DIR%\data" 2>nul

echo.
echo 步骤8: 打包压缩...
if exist "deploy_anode_release.zip" del "deploy_anode_release.zip"
powershell -command "Compress-Archive -Path '%REMOTE_DIR%' -DestinationPath 'deploy_anode_release.zip' -Force" >nul
if %ERRORLEVEL% neq 0 (
    echo 警告: 压缩失败，请手动压缩 %REMOTE_DIR% 目录
) else (
    echo 压缩完成: deploy_anode_release.zip
)

echo.
echo ============================
echo 部署完成
echo ============================
echo 部署目录: %REMOTE_DIR%
echo 压缩包:   deploy_anode_release.zip
echo.
echo 包含内容:
echo - %PRODUCT_NAME% (主程序)
echo - Qt运行库 (windeployqt)
echo - Anode\lib 下所有 DLL
echo - config 配置
echo - logs / data 目录

echo.
pause
