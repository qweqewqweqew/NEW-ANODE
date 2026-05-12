# Algorithm Interface Overview

本文档整理了雷达算法库对外开放的接口、关键结构体以及在项目代码中的位置，便于在 Qt/其它框架中进行二次封装。

---

## 1. 数据读取与转换 (`CHVisionAdvX`)

文件：`Anode/CHVisionAdvX.h`

```cpp
int ReadLidarDevDataX(Double3DPointVec& d3dPoints,
                      int mapWidth,
                      std::string sFullPathName);

int TransLidarDevData2Lidar3ds(Double3DPointVec d3dPoints,
                               s_LidarTrans sLidarTrans,
                               s_Lidar3d &imageT);
```

- **ReadLidarDevDataX**  
  - 输入：PLY 文件路径、点云宽度 `mapWidth`（每行采样点数）。  
  - 输出：`Double3DPointVec`（即 `std::vector<std::vector<Point3D>>`）。  
  - 注意：内部使用 Windows ANSI API，封装层需要先把 `QString` 转成 GBK/ACP 字符串。

- **TransLidarDevData2Lidar3ds**  
  - 作用：将原始点云按 `s_LidarTrans` 指定的轴交换、镜像、旋转规则，转换为算法统一使用的 `s_Lidar3d`（Halcon 图像结构）。  
  - `s_LidarTrans` 字段见 `Anode/DataX.h` 第 818 行附近。

---

## 2. 核心算法类 `CUnloadPlateA`

文件：`Anode/CUnloadPlateA.h`

```cpp
s_Rtnf OnPreProcess(s_Lidar3d imageT,
                    s_PreProcessLidar3dPara params,
                    s_PreProcessLidar3dRts& result);

s_Rtnf OnPreAD(s_Lidar3d image3d,
               s_PreADPlateAPara params,
               s_PreADPlateARtsPara& result);
```

- **OnPreProcess**
  - 输入：原始 `s_Lidar3d`、预处理参数 `s_PreProcessLidar3dPara`。
  - 输出：`s_PreProcessLidar3dRts`，其中 `sImage3dPro` 是过滤/重采样后的点云，供后续检测使用。

- **OnPreAD**
  - 输入：通常为 `s_PreProcessLidar3dRts::sImage3dPro`，以及检测参数 `s_PreADPlateAPara`。
  - 输出：`s_PreADPlateARtsPara`，包含铜垛区域、缺陷区域、各类判定结果。

所有接口都返回 `s_Rtnf`（`Anode/DataX.h` 第 1604 行）：`iCode` 表示错误码，`strInfo` 为 GBK 编码的错误描述。

---

## 3. 关键数据结构

定义位置：`Anode/DataX.h`

### 3.1 点云与坐标转换

- **`s_Lidar3d`**（第 702 行）  
  - 成员：`HObject Intensity/X/Y/Z/NX/NY/NZ`，`HTuple ObjectModel3D`，`long ID`。  
  - 方法：`GetMapWidth()`、`GetMapHeight()` 用于获取灰度图尺寸。

- **`s_LidarTrans`**（第 818 行）  
  - 控制坐标系调整：`Y_Z_Change`、`X_Y_Change`、`X_mirro`、`Y_mirro`、`Map_Rotate`。

### 3.2 预处理

- **`s_PreProcessLidar3dPara`**（第 862 行）  
  - 主要字段：ROI 范围 (`fRoiZ/X/Y{Min,Max}`)、分割阈值 (`fThSeg0Dis` 等)、重采样分辨率 (`fProSetResolutionX/Y`)。

- **`s_PreProcessLidar3dRts`**（第 939 行）  
  - 字段：`bool bTJG`（综合判定）、`double time`、`s_Lidar3d sImage3dPro`。

### 3.3 检测

- **`s_PreADPlateAPara`**（第 968 行）  
  - 铜垛/栏板/空间异物的筛选条件，如：  
    - `fThCbSelL/W/Pts/Z{Min,Max}`：铜垛尺寸、点数、Z 高度范围；  
    - `bDefectSpaceObj` 与 `fDefectSpaceTh...`：空间异物识别参数；  
    - `fThSafeDisTbNear`、`fThSafeDisToCbOuter`：安全距离阈值等。

- **`s_PreADPlateARtsPara`**（第 1140 行）  
  - 结果字段：  
    - `TbUpRtsList`：每个铜垛的中心点、四角坐标（像素 + 三维）。  
    - `RegionTbsPlus`、`RectTbsPlus`：Halcon 区域/矩形，便于可视化。  
    - `RegionDefect*`：栏板、空间异物、铜垛接触、安全距离等缺陷区域。  
    - 各类布尔标识：`bExistDefectSpace`、`bExitTbClose` 等。

---

## 4. 典型调用顺序

1. **数据准备**
   - 通过雷达 SDK 或 `ReadLidarDevDataX` 获取 `Double3DPointVec`。
   - 配置 `s_LidarTrans`（轴交换、镜像）并调用 `TransLidarDevData2Lidar3ds` 得到 `s_Lidar3d`。

2. **预处理**
   - 填充 `s_PreProcessLidar3dPara`（ROI、分辨率、阈值）。
   - 调用 `OnPreProcess`，结果 `s_PreProcessLidar3dRts::sImage3dPro` 作为下游输入。

3. **检测**
   - 设置 `s_PreADPlateAPara`（铜垛筛选、缺陷判定、间距等参数）。
   - 调用 `OnPreAD`，解析 `s_PreADPlateARtsPara` 中的铜垛信息与缺陷区域。

4. **结果使用**
   - `TbUpRtsList` 可输出坐标、角点；  
   - `RegionTbsPlus`、`RegionDefect*` 可用于 UI 层叠加显示或导出图像；  
   - 若 `s_Rtnf::iCode` 非 0，需要把 `strInfo`（GBK）转换为 UTF-8/Unicode 提示给用户。

---

## 5. 注意事项

- **编码**：算法内部使用 GBK 读写文件、拼接错误信息。封装层必须在传路径/读取错误消息前做好编码转换。
- **`mapWidth`**：必须与点云真实一行的数据点数匹配，否则 `ReadLidarDevDataX` 会解包失败。
- **线程安全**：`CUnloadPlateA` 不是线程安全的，建议与 `AnodeAlgorithmWorker` 一样在专用线程中顺序调用。
- **参数一致性**：若要复现实地测试结果，`s_PreProcessLidar3dPara`、`s_PreADPlateAPara`、`s_LidarTrans` 必须与 MFC 原项目保持一致。

---

如需扩展/调整算法行为，可在 `Anode/anodealgorithmworker.cpp` 中查看我们目前如何初始化这些结构，并据此修改。***











