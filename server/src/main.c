//
// Created by bysunlits on 2026/9/6.
//
#include <stdio.h>
/*
# 文件同步系统 — 架构设计说明

## 1. 定位与范围
- 单源推送、多订阅者拉取的文件同步系统。
- 服务端为 Linux，作为**可信中间人**：客户端经认证后即视为可信，**不处理恶意客户端**。
- 并发规模：**两位数以内**，不按大规模分发设计。

## 2. 拓扑与通信
- `Client (Windows / Linux) ←TLS→ Nginx ←明文内部通道→ App Server (Linux)`
- 基于 **TCP + TLS**，与 HTTP/HTTPS/WebSocket 无关；数据全程经过**用户态**，kTLS 可选。
- **TLS 在 Nginx 终止**；Nginx 同时承担慢速连接防护（`proxy_timeout`）。

## 3. 注册与声明
- 客户端通过 register 消息注册，服务端分配 **UUID**。
- 发送前先发一条**声明消息**：文件大小、目标客户端、文件名等元信息。

## 4. 帧格式（扁平化）
- **固定帧头 + 变长 payload**，解决流式粘包。
- 帧头字段：`Length`、`Type`（区分帧类型，如 `DATA` 与控制/元数据帧）。
- **帧头扁平，不嵌套**（不搞 header 套 header）。
- `DATA` 帧 payload 结构固定为：

  ```
  [ 帧头 (Len, Type=DATA) ] [ Chunk Index (4B) ] [ Offset (8B) ] [ Chunk Data Payload ]
  ```

- 分块方式**显式写入元数据**随流传递。

## 5. 分块与校验
- **固定大小分块**，暂定 **1MB**；不做内容定义分块（CDC）。
- **取消 diff 机制**：不做基于内容的文件切割与增量比对；插入/删除导致**全量重传**为已接受的 tradeoff。
- **Hash 树仅用于包级校验与重传**，不用于增量比对。
- 客户端在**检测到源文件变化后立即计算 hash 树**；同步时**先发 hash 树，再发数据**。
- **消费端自行通过 hash 树校验**，不依赖"发送完成"消息（避免 ACK-of-ACK 通信死锁）。
- 根 hash 来自客户端，认证通过即信任，**原样保存，永不从磁盘内容重算覆盖**。

## 6. 服务端发布与版本一致性
- 落盘：**写临时文件 + rename 原子发布**，rename 后不再改动。
- 版本标识以**内容根 hash**为准，而非 mtime。
- mtime 仅作"该去检查一下"的**触发信号**。
- 服务端在**落盘后校验根 hash**，并**定期校验**，保证文件未损坏。
- 对外可见的判定时刻：**push 完成 + 根校验通过**。

## 7. 客户端侧一致性
- **Linux**：读取期间通过 **CoW 快照**保证内容固定。
- **Windows**：**USN Journal** 检测变更；**机遇锁（oplock）**保护，检测到 break **立即放弃、稍后重试**；复制为**独占副本**（`FILE_SHARE_READ`）后再读/传。
- 客户端**只读不写**；**不使用逻辑时钟**（避免并发冲突，目标为零 conflict）。

## 8. 同步触发与完成判定
- 触发：① 服务端通知推送；② 客户端**周期性轮询（约 5 分钟）**。
- 完成判据：**客户端以收到服务端消息**作为同步成功标志。

## 9. 明确排除 / 暂缓
- 已放弃：`SCM_RIGHTS` handoff、接收端零拷贝。
- 暂缓：内容定义分块（CDC）、diff 机制。
- 不做：逻辑时钟。
- 当前形态：非端到端加密（TLS 在 Nginx 终止）。
*/
enum INFOMESSAGE
{
	START_MSG, IN_PROGRESS_MSG, END_MSG
};
typedef enum
{
	ACPT_SUCCESS,NET_ERR,DISK_ERR,UNKNOWN_ERR
}ACCEPT_DATA_RESULT;

ACCEPT_DATA_RESULT accept_data();

typedef enum
{
	HLD_SUCCESS
}HLD_HEAD_RET;

HLD_HEAD_RET hdl_head();

int main()
{
	load_config();
	accept_data();
	rd_meta_data_from_file();
	notify_target_client();


	{
		size_t buffer[256*1024];
		if (fread(buffer,1,sizeof(buffer),stdin)) {

		}
	}

}
