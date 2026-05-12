# API 与作业流程说明

版本：当前实现（AlignMax=5，拍照延时=5s）

## 本地 HTTP 接口（由 ApiServer 提供，默认端口见 config/scan_config.json 的 serverConfig.port）

### 1. GET /api/status
- 请求体：无
- 返回：200 JSON
  - system / camera1 / camera2 / radar / algorithm

### 2. GET /api/config
- 请求体：无
- 返回：200 JSON
  - 至少包含 radarScanTimeout / cameraCaptureTimeout（还会含 fineTolerance / shrinkValue 等扩展字段）

### 3. POST /api/config
- 头：Content-Type: application/json
- 请求体（最小示例）：
```json
{"radarScanTimeout":40,"cameraCaptureTimeout":15}
```
- 返回：200 JSON
  - {"success": true, "message": "参数设置成功"}

### 4. POST /api/scan
- 头：Content-Type: application/json
- 请求体（最小示例）：
```json
{"type":"1","uniqueCode":"TEST001"}
```
- 行为：记录 uniqueCode，立即触发一次雷达扫描（后续流程见“作业流程”）。
- 返回：200 JSON
  - {"success": true, "message": "任务已接收并开始雷达扫描 (雷达粗定位)", "uniqueCode": "...", "phase": "scanning"}

### 5. POST /api/upload-radar-file
- 头：Content-Type: application/json
- 请求体：
```json
{"filePath":"C:/path/to/file.ply"}
```
- 行为：校验文件存在且为 .ply，送入算法线程处理（点云替代雷达实扫）。
- 返回：200 JSON
  - {"success": true, "message": "文件已接收，正在处理: ...", "filePath": "..."}

### 6. POST /api/position-ready
- 请求体：
```json
{ "status": "ready" }
```
- 行为：忽略 blockIndex，按照当前内部拍照位顺序推进精定位。
- 返回：200 JSON
  - {"code": 1, "message": "已收到通知"}

## 外部 WMS 接口（由 WmsClient 主动调用，URL 取自 config/scan_config.json -> wmsConfig）

### A. 请求精定位移动（拍照位或微调位）
- URL：wmsConfig.adjustPositionUrl（默认占位 http://<host>:8099/api/OpenApi/RequstMoveForActualLocation）
- 请求体：坐标 JSON（按现场要求，可包含 x/y/z/deg 或文档的 x/y/z）
- 成功期望：{"code":1,"status":"success","message":""}

### B. 批量上报抓取位
- URL：wmsConfig.reportResultUrl（默认占位 http://<host>:8099/api/OpenApi/ReceiveUnloadPositionData）
- 请求体：抓取位数组（示例字段：x/y/z/direction/angle，或按现场要求）
- 成功期望：{"code":1,"status":"success","message":""}

## 作业流程（type=1 雷达任务）

1) WMS 下发任务：`POST /api/scan`，type=1。系统返回 success=true，并启动雷达扫描。  
2) 雷达扫描完成，点云进入算法 → 生成拍照位列表（按算法顺序 blockIndex=0..N-1）。  
3) 对每个拍照位（顺序处理）：  
   - 调用 `RequstMoveForActualLocation` 发送拍照位坐标。  
   - 等待 WMS 回调 `POST /api/position-ready` (status=ready)。  
   - 延时拍照（默认 5s，可配置）。  
   - 将图像送精定位算法，得到 iStatus：  
     - iStatus=1：可抓取，得到抓取位，缓存并继续下一个拍照位。  
     - iStatus=5：需要微调，算法给出调整坐标，再次调用 `RequstMoveForActualLocation`，循环“ready→拍照→精定位”。  
     - iStatus=2（或超过 AlignMax）：视为失败，不再重试，继续下一个拍照位。  
   - AlignMax=5：精定位微调最多 5 次，超过则判定失败。  
4) 全部拍照位处理完毕后，收集成功抓取位，按数组一次性调用 `ReceiveUnloadPositionData` 上报。返回 code=1/status=success/message="" 视为完成。

## 可调参数
- AlignMax：默认 5，可在代码中通过 BatchFinePositioningManager::setAlignMax 修改。  
- 拍照延时：默认 5000 ms，可通过 BatchFinePositioningManager::setCaptureDelayMs 修改。  
- WMS 地址与超时：`config/scan_config.json` 的 wmsConfig 节点。  
- 服务器监听：`serverConfig.port/host`。








