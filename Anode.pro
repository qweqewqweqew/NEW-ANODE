QT       += core gui widgets sql serialbus concurrent
QT       += httpserver network  # HTTP服务器和网络请求支持（API模块）

CONFIG += c++17

# 定义项目根目录
PROJECT_ROOT = $$PWD/..

# 定义SDK路径
SICK_SDK_PATH = $$PROJECT_ROOT/卸车项目参考/3D_Platform_SDK_V331/SDK/C++/Windows/PlatformScanDemo/TestPlatformDLL/SDK
HALCON_SDK_PATH = $$PWD/lib/halcon22X64
PHOXI_SDK_PATH = $$(PHOXI_CONTROL_PATH)/API
# OPENCV_SDK_PATH = $$PWD/lib  # 暂时不使用OpenCV

# 定义头文件路径
INCLUDEPATH += \
    $$PHOXI_SDK_PATH/include \
    $$PWD/include \
    $$PWD/include/halcon22X64 \
    $$PWD/include/halconc \
    $$PWD/include/halconcpp \
    $$SICK_SDK_PATH/include

# 定义库文件路径
LIBS += -L$$HALCON_SDK_PATH
LIBS += -L$$PHOXI_SDK_PATH/lib
LIBS += -L$$PHOXI_SDK_PATH/bin
# LIBS += -L$$OPENCV_SDK_PATH

# SICK雷达SDK库文件
LIBS += -L$$PWD/lib
CONFIG(debug, debug|release) {
    # 调试版本
    LIBS += -lSICK_3DPlatform_x64_331d
} else {
    # 发布版本
    LIBS += -lSICK_3DPlatform_x64_331
}

# Halcon机器视觉SDK库文件
LIBS += -lhalcon
LIBS += -lhalconcpp
#PhoXi SDK库文件
LIBS += -lPhoXi_API

# OpenCV库文件（动态链接，只使用DLL）
# 注意：由于只有DLL文件，我们通过运行时加载的方式使用OpenCV
# 如果需要静态链接，需要下载OpenCV的完整版本包含.lib文件

# 预处理器定义
DEFINES += WIN32
DEFINES += _WINDOWS
DEFINES += _CRT_SECURE_NO_WARNINGS

# 编译器特定设置
msvc {
    # Visual Studio编译器设置
    QMAKE_CXXFLAGS += /bigobj
    QMAKE_CXXFLAGS += /MP
    
    # 禁用一些警告
    QMAKE_CXXFLAGS += /wd4251
    QMAKE_CXXFLAGS += /wd4275
    # 禁用编码警告 C4828（算法文件是 GBK 编码，编译器使用 UTF-8）
    QMAKE_CXXFLAGS += /wd4828
    
    # 使用UTF-8编码（Qt 6.7.3默认添加-utf-8标志）
    # 算法文件是 GBK 编码，会产生编码警告，已通过 /wd4828 禁用
}

# 源文件
SOURCES += \
    main.cpp \
    widget.cpp \
    radarcontroller.cpp \
    heatmapgenerator.cpp \
    heatmapwidget.cpp \
    camerawidget.cpp \
    phoxicontroller.cpp \
    heatmapcontroldialog.cpp \
    CHVisionAdvX.cpp \
    CPhoxiCamera.cpp \
    CUnloadPlateA.cpp \
    anodealgorithmworker.cpp \
    api/apiserver.cpp \
    api/wmsclient.cpp \
    api/batchfinepositioningmanager.cpp \
    utils/imageutils.cpp \
    parametersettingsdialog.cpp \
    cameratestdialog.cpp

# 测试程序（可选）
# SOURCES += test_heatmap.cpp

# 头文件
HEADERS += \
    widget.h \
    radarcontroller.h \
    heatmapgenerator.h \
    heatmapwidget.h \
    camerawidget.h \
    phoxicontroller.h \
    heatmapcontroldialog.h \
    DataX.h \
    CHVisionAdvX.h \
    CUnloadPlateA.h \
    anodealgorithmworker.h \
    api/apiserver.h \
    api/apitypes.h \
    api/apiconfig.h \
    api/wmsclient.h \
    api/batchfinepositioningmanager.h \
    utils/imageutils.h \
    parametersettingsdialog.h \
    CPhoxiCamera.h\
    cameratestdialog.h

# 资源文件
RESOURCES += \
    images.qrc

# UI文件
FORMS += \
    widget.ui \
    parametersettingsdialog.ui

# 运行时库文件复制
win32 {
    # 复制SICK雷达动态库
    CONFIG(debug, debug|release) {
        SICK_DLL = $$SICK_SDK_PATH/x64_debug/SICK_3DPlatform_x64_331d.dll
    } else {
        SICK_DLL = $$SICK_SDK_PATH/x64_release/SICK_3DPlatform_x64_331.dll
    }
    
    # 复制PhoXi动态库（v1.16.2）
    PHOXI_DLLS = $$PHOXI_SDK_PATH/bin/PhoXi_API.dll
    
    # 复制Halcon动态库
    HALCON_DLLS = $$HALCON_SDK_PATH/halcon.dll \
                  $$HALCON_SDK_PATH/halconcpp.dll
    
    # 复制OpenCV动态库（暂时不使用）
    # OPENCV_DLLS = $$OPENCV_SDK_PATH/opencv_core3420.dll \
    #               $$OPENCV_SDK_PATH/opencv_imgproc3420.dll \
    #               $$OPENCV_SDK_PATH/opencv_imgcodecs3420.dll \
    #               $$OPENCV_SDK_PATH/opencv_highgui3420.dll
    
    # 复制Visual C++运行时库
    CONFIG(debug, debug|release) {
        VC_RUNTIME_DLLS = $$SICK_SDK_PATH/accessory/x64 debug/msvcp140d.dll \
                          $$SICK_SDK_PATH/accessory/x64 debug/vcruntime140d.dll \
                          $$SICK_SDK_PATH/accessory/x64 debug/vcruntime140_1d.dll
    } else {
        VC_RUNTIME_DLLS = $$SICK_SDK_PATH/accessory/x64 release/msvcp140.dll \
                          $$SICK_SDK_PATH/accessory/x64 release/vcruntime140.dll \
                          $$SICK_SDK_PATH/accessory/x64 release/vcruntime140_1.dll
    }
    
    # QModbus是Qt自带的库，无需复制额外的动态库
    
    # 设置DESTDIR
    # 方案1: 输出到项目根目录的 bin (便于部署)
    # DESTDIR = $$PROJECT_ROOT/bin
    # 方案2: 输出到构建目录的 release (标准Qt构建输出)
    DESTDIR = $$OUT_PWD
    
    # 复制所有动态库到输出目录
    # 注意：路径中包含中文字符，使用 PowerShell 处理以避免编码问题
    
    # 复制SICK DLL（使用PowerShell处理中文路径）
    QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_DLL' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    
    # 复制PhoXi DLL
    QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$PHOXI_DLLS' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    
    # 复制Halcon DLL（逐个文件）
    QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$HALCON_SDK_PATH\\halcon.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$HALCON_SDK_PATH\\halconcpp.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    
    # 复制VC运行时库
    CONFIG(debug, debug|release) {
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 debug\\msvcp140d.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 debug\\vcruntime140d.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 debug\\vcruntime140_1d.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 release\\msvcp140.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 release\\vcruntime140.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += powershell -NoProfile -ExecutionPolicy Bypass -Command \"& { Copy-Item -Path '$$SICK_SDK_PATH\\accessory\\x64 release\\vcruntime140_1.dll' -Destination '$$DESTDIR' -Force -ErrorAction SilentlyContinue }\" $$escape_expand(\\n\\t)
    }
    
    # QModbus无需复制额外的动态库
}

# 默认部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
