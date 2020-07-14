//
// Created by nbaiot@126.com on 2020/7/7.
//

#ifndef NBAIOT_AV_H264_SEI_EXTEND_H
#define NBAIOT_AV_H264_SEI_EXTEND_H

#include <list>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>

namespace nbaiot {

namespace H264SEI {

using NALU = struct NALU {
  uint8_t* data{nullptr};    /// 内存起始
  uint32_t offset{0};    /// 偏移
  uint32_t size{0};    /// 大小
  uint32_t codeSize{0};  /// 标志头大小
  uint8_t type{0};    /// 类型
};

using SeiContent = struct SeiContent {
  uint8_t* uuid{nullptr};
  uint8_t* data{nullptr};
  uint32_t size{0};
  uint32_t payloadSize{0};
};

enum H264AnnexbType {
  kAnnexbType_0 = 0,
  kAnnexbType_4,
  kAnnexbType_3,
};

/// 检测是否是标准 H264 码率
bool CheckIsAnnexb(const uint8_t* packet, uint32_t size);

///  获取标准 H264 类型
H264AnnexbType GetAnnexbType(const uint8_t* packet, uint32_t size);

/// 起始码长度
uint32_t GetAnnexbStartCodeSize(const uint8_t* packet, uint32_t size);

/// 包含防竞争字段长度
uint32_t GetContentSizeWithComplete(const uint8_t* data, uint32_t size);

/// 防竞争字段外长度
uint32_t GetContentSizeWithoutComplete(const uint8_t* data, uint32_t size);

/// 删除防竞争字段
void DeleteCompleteContent(const SeiContent& content, uint8_t* data, uint32_t size);

/// 查找标准 H264 码流起始码
int FindAnnexb(const uint8_t* packet, uint32_t size);

/// 查找 sei nalu
int32_t FindSEINalu(const std::vector<NALU>& nalus, const uint8_t* uuid);

/// 获取 sei nalu 长度
uint32_t GetSEINaluSize(const uint8_t* data, uint32_t size);

/// 获取 sei 包长度
uint32_t GetSEIPacketSize(const uint8_t* data, uint32_t size, H264AnnexbType annexbType);

/// 解析 SEI
int32_t GetSEIContent(uint8_t* packet, uint32_t size, SeiContent& content);

/// TODO: 简化，若 sei 数量已知，当获取全部 sei 后，停止解析
/// 获取标准 H264 码流
int32_t GetAnnexbSEIContent(uint8_t* packet, uint32_t size, const uint8_t* uuid, uint8_t** pdata, uint32_t* psize);

int32_t GetAnnexbAllSEIContent(uint8_t* packet, uint32_t size,
                               std::vector<std::vector<uint8_t>>& uuidList,
                               std::vector<std::string>& seiPayloads);

/// 获取 AVCC 标准 H264 码流
int32_t GetAVCCSEIContent(uint8_t* packet, uint32_t size, const uint8_t* uuid, uint8_t** pdata, uint32_t* psize);

int32_t GetAVCCAllSEIContent(uint8_t* packet, uint32_t size,
                               std::vector<std::vector<uint8_t>>& uuidList,
                               std::vector<std::string>& seiPayloads);

/// 填充 sei packet
bool FillSEIPacket(uint8_t* packet, uint32_t annexbType, const uint8_t* uuid, const uint8_t* data, uint32_t size);

/// 释放 sei 资源
void FreeSEIContent(uint8_t** pdata);

}

}

#endif //NBAIOT_AV_H264_SEI_EXTEND_H
