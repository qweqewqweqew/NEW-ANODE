# API 通讯与数据流详解

版本：当前实现（AlignMax=5，拍照延时=5s）。涵盖本地 HTTP 接口、对外 WMS 调用、数据流向、关键数据结构与调试要点。

---

## 1. 本地 HTTP 接口（ApiServer 提供）
端口：`config/scan_config.json` 中 `serverConfig.port`。

### 1.1 GET /api/status
- 请求：无
- 返回：`system / camera1 / camera2 / radar / algorithm` 状态 JSON。

### 1.2 GET /api/config
- 请求：无
- 返回：配置 JSON（至少包含 `radarScanTimeout`、`cameraCaptureTimeout`，可能含 `fineTolerance`、`shrinkValue` 等）。

### 1.3 POST /api/config
- 头：`Content-Type: application/json`
- 体示例：`{"radarScanTimeout":40,"cameraCaptureTimeout":15}`
- 返回：`{"success": true, "message": "参数设置成功"}`。

### 1.4 POST /api/scan
- 头：`Content-Type: application/json`
- 体示例：`{"type":"1","uniqueCode":"TEST001"}`
- 行为：记录 `uniqueCode`，立即触发雷达扫描（流程见第4节）。
- 返回：`{"success": true, "message": "...", "uniqueCode": "...", "phase": "scanning"}`。

### 1.5 POST /api/upload-radar-file
- 头：`Content-Type: application/json`
- 体示例：`{"filePath":"C:/path/to/file.ply"}`
- 行为：校验 `.ply` 存在，送算法线程处理（代替实扫）。
- 返回：`{"success": true, "message": "文件已接收，正在处理: ...", "filePath": "..." }`。

### 1.6 POST /api/position-ready
- 体示例：`{ "status": "ready" }`
- 行为：通知“位置已就绪”，按内部队列推进精定位（忽略 blockIndex）。
- 返回：`{"code": 1, "message": "已收到通知"}`。

---

## 2. 外部 WMS 接口（系统主动调用，`api/wmsclient.cpp`）
配置：`config/scan_config.json -> wmsConfig`。

### 2.1 请求精定位移动（拍照位/微调位）
- URL：`adjustPositionUrl`（默认占位 `http://<host>:8099/api/OpenApi/RequstMoveForActualLocation`）
- 体：现场定义的坐标 JSON（常见 x/y/z/angle|deg；代码现已发送 angle）
- 期望成功：`{"code":1,"status":"success","message":""}`

### 2.2 批量上报抓取位
- URL：`reportResultUrl`（默认占位 `http://<host>:8099/api/OpenApi/ReceiveUnloadPositionData`）
- 体：抓取位数组，字段示例：`x / y / z / direction / angle`
- 期望成功：`{"code":1,"status":"success","message":""}`

### 2.3 WmsClient 上报/移动实现要点
- Content-Type：`application/json`
- 精定位移动：`adjustPosition` 发送 `x/y/z/angle`（整数化 qRound，angle 对应算法 deg）。
- 抓取位上报：根是 JSON 数组；仅上报 `flag=true` 的点；`x/y/z/angle` 先 `qRound` 成整数，`direction=1`。
- URL 解析：优先 `wmsReportResultUrl()`；为空则从 `wmsUrl` 推导默认 `ReceiveUnloadPositionData`；URL 无效或无点则 `reportFailed`。

---

## 3. 关键数据结构
- `PointCloud`：`std::vector<std::vector<Point3D>>`（雷达行×列）。
- `ApiTypes::Point3D`：`x, y, z, deg, flag, pointType, blockIndex`。
- `ApiTypes::ScanResult`：`code`(0/1)、`type`("1"雷达/"2"相机)、`uniqueCode`、`message`、`data`(Point3D 数组)。
- 算法结果：
  - `s_PreADPlateARtsPara`：雷达检测主结果。
  - `s_CalcPreAlignRtsPara`：包含拍照/抓取位列表 `fCamaraPosCrane`。

---

## 4. 作业与数据流（type=1 雷达任务）
1) **接单**：`POST /api/scan` → 记录 `uniqueCode`，启动雷达扫描。  
2) **雷达扫描**：`RadarController` 采集 → `Widget::onRadarPointCloudUpdated` → 可选落盘 PLY。  
3) **算法粗定位**：`Widget::startAlgorithmProcessing` → `AnodeAlgorithmWorker::processPointCloud`：  
   点云 → `convertPointCloudToLidar3d` → Halcon/`CUnloadPlateA` → `executeAlgorithm` → 生成 `fCamaraPosCrane`。  
4) **精定位与微调**（每拍照位，顺序，最多 AlignMax=5）：  
   - 调 WMS `RequstMoveForActualLocation` 发送拍照位  
   - 等 `POST /api/position-ready` (ready)  
   - 延时拍照（默认 5s）→ 精定位  
   - `iStatus`：1=成功产出抓取位；5=需微调再发移动；2/超限=失败跳过  
5) **上报结果**：收集成功抓取位 → `ReceiveUnloadPositionData`（数组上报）；返回 `code=1/status=success` 视为完成。

---

## 5. 坐标/点云的流向
- **入口**
  - 实时点云：`RadarController::pointCloudUpdated` → `Widget::startAlgorithmProcessing`
  - 文件点云：`POST /api/upload-radar-file` → `AnodeAlgorithmWorker::processLocalFile(mapWidth)`
- **算法线程**（`AnodeAlgorithmWorker`）
  - 预处理/检测/精定位 → `s_PreADPlateARtsPara` + `s_CalcPreAlignRtsPara`
  - 信号 `processingFinished(result, lidar3d, preAlignValid, preAlignResult, preAlignError)` 回主线程
- **主线程消费**（`Widget::onAlgorithmProcessingFinished`）
  - 日志打印全部抓取位
  - 需要上报时：`buildRadarScanResult` → `WmsClient::reportResult`
- **落盘**
  - 点云：`saveRadarPointCloudSnapshot` → `data/radar/<date>/<task>_<stage>_<timestamp>.ply`
  - Markdown 结果：已注释，默认不再生成。

---

## 6. 配置与参数
- `config/scan_config.json`：`serverConfig`（端口）、`wmsConfig`（外部 URL/超时）、算法阈值。
- UI 参数：雷达宽度、安全距离；相机拍照位（X/Y/Z/Deg）变更后调用 `AnodeAlgorithmWorker::setCameraPositionParameters`。
- 参数持久化：`QSettings` 读写宽度/安全距/窗口尺寸。

---

## 7. 调试提示
- 本地接口：`curl`/Postman 调 `POST /api/scan` 或 `POST /api/upload-radar-file`。
- WMS 测试：`test_server/testwmsserver` 可在本机 9090 接收上报。
- 日志：算法完成会打印全部抓取位；`processLocalFile` 会在宽度错误或点云为空时报错。
- 宽度：实时点云无需硬编码；文件模式 `mapWidth` 需正确（可在文件名加入 `Wxxxx` 供解析）。

