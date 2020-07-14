//
// Created by nbaiot@126.com on 2020/7/7.
//

#include "av_h264_sei_extend.h"

#include <cmath>
#include <cstring>

namespace nbaiot {

/// SEI NALU 数据格式

/// 1. NALU 类型 1 字节: 0x06
/// 2. SEI 负载类型 1 字节L 0x05 （用户自定义数据）
/// 3. 负载大小（uuid+自定义数据），如果大小 size 大于 255，前 int(size / 255) 个字节都是 FF，最后一个字节是剩余部分
/// 4. 负载的唯一标志 uuid 16 字节
/// 5. 自定义数据


namespace H264SEI {

static const uint8_t ANNEXB_CODE_LOW[] = {0x00, 0x00, 0x01};

static const uint8_t ANNEXB_CODE[] = {0x00, 0x00, 0x00, 0x01};

static const int32_t UUID_SIZE = 16;

static const uint8_t TYPE_MASK = 0x1F;

static const uint8_t SEI_TYPE = 6;

static const uint8_t SEI_PAYLOAD_TYPE = 5;

static const uint32_t AVCC_START_CODE_SIZE = 4;

/// 大端转小端
static uint32_t ReverseBytes(uint32_t value) {
  return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
         (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}


bool CheckIsAnnexb(const uint8_t* packet, uint32_t size) {
  if (!packet)
    return false;
  return (size > 3 && std::memcmp(packet, ANNEXB_CODE_LOW, 3) == 0) ||
         (size > 4 && std::memcmp(packet, ANNEXB_CODE, 4) == 0);
}

H264AnnexbType GetAnnexbType(const uint8_t* packet, uint32_t size) {
  if (!packet)
    return kAnnexbType_0;
  if (size > 3 && std::memcmp(packet, ANNEXB_CODE_LOW, 3) == 0) {
    return kAnnexbType_3;
  } else if (size > 4 && std::memcmp(packet, ANNEXB_CODE, 4) == 0) {
    return kAnnexbType_4;
  }
  return kAnnexbType_0;
}

uint32_t GetAnnexbStartCodeSize(const uint8_t* packet, uint32_t size) {
  if (!packet) {
    return 0;
  }
  if (size > 3 && std::memcmp(packet, ANNEXB_CODE_LOW, 3) == 0) {
    return 3;
  } else if (size > 4 && std::memcmp(packet, ANNEXB_CODE, 4) == 0) {
    return 4;
  }
  return 0;
}

uint32_t GetContentSizeWithComplete(const uint8_t* data, uint32_t size) {
  if (data == nullptr)
    return 0;

  uint32_t zero_count = 0;
  uint32_t zero_prevention = 0;
  for (uint32_t i = 0; i < size; i++) {
    if (zero_count >= 2) {
      zero_prevention++;
      zero_count = 0;
      continue;
    }
    if (data[i] == 0x00) {
      zero_count++;
    } else {
      zero_count = 0;
    }
  }
  return size + zero_prevention;
}

uint32_t GetContentSizeWithoutComplete(const uint8_t* data, uint32_t size) {
  if (data == nullptr)
    return 0;

  uint32_t zero_count = 0;
  uint32_t uncompete_size = size;
  for (uint32_t i = 0; i < size; i++) {
    if (zero_count >= 2) {
      if (data[i] == 0x03) {
        uncompete_size -= 1;
      }
      zero_count = 0;
    } else if (data[i] == 0x00) {
      zero_count++;
    } else {
      zero_count = 0;
    }
  }
  return uncompete_size;
}

void DeleteCompleteContent(const SeiContent& content, uint8_t* data, uint32_t size) {
  if (size == content.size) {
    memcpy(data, content.data, size);
    return;
  }
  int zero_count = 0;
  for (uint32_t i = 0; i < content.size; i++) {
    if (zero_count >= 2) {
      if (content.data[i] == 0x03) {
        zero_count = 0;
      } else {
        *data++ = content.data[i];
        size--;
      }
    } else if (content.data[i] == 0x00) {
      zero_count++;
      *data++ = content.data[i];
      size--;
    } else {
      zero_count = 0;
      *data++ = content.data[i];
      size--;
    }
    if (size <= 0)
      break;
  }
}

int FindAnnexb(const uint8_t* packet, uint32_t size) {
  if (size <= 0)
    return -1;
  int32_t index = 0;
  while (size - index > 0) {
    if ((size - index > 3) && packet[index] == 0x00 && packet[index + 1] == 0x00) {
      if (packet[index + 2] == 0x01) {
        return index;
      } else if ((size - index > 4) && packet[index + 2] == 0x00 && packet[index + 3] == 0x01) {
        return index;
      }
    }
    index += 1;
  }

  return -1;
}

int32_t FindSEINalu(const std::vector<NALU>& nalus, const uint8_t* uuid) {
  if (nalus.empty())
    return -1;

  for (int i = 0; i < nalus.size(); i++) {
    if (nalus[i].type != SEI_TYPE) {
      continue;
    }

    uint8_t* sei_data = nalus[i].data + nalus[i].offset + nalus[i].codeSize + 1;
    int32_t sei_data_length = nalus[i].size - nalus[i].codeSize - 1;
    SeiContent content;
    int32_t ret = GetSEIContent(sei_data, sei_data_length, content);
    if (ret != -1) {
      if (std::memcmp(uuid, content.uuid, UUID_SIZE) == 0) {
        return i;
      }
    }
  }
  return -1;
}

uint32_t GetSEINaluSize(const uint8_t* data, uint32_t size) {
  uint32_t content_size = GetContentSizeWithComplete(data, size);
  /// 计算空间的
  uint32_t payload_size = size + UUID_SIZE;
  /// SEI payload size
  uint32_t sei_payload_size = content_size + UUID_SIZE;
  /// NALU + payload类型 + 数据长度 + 数据
  uint32_t sei_size = 1 + 1 + (payload_size / 0xFF + (payload_size % 0xFF != 0 ? 1 : 0)) + sei_payload_size;
  /// 截止码
  uint32_t tail_size = 2;
  if (sei_size % 2 == 1) {
    tail_size -= 1;
  }
  sei_size += tail_size;

  return sei_size;
}

uint32_t GetSEIPacketSize(const uint8_t* data, uint32_t size, H264AnnexbType annexbType) {
  if (annexbType == kAnnexbType_3) {
    return GetSEINaluSize(data, size) + 3;
  }
  return GetSEINaluSize(data, size) + 4;
}

int32_t GetSEIContent(uint8_t* packet, uint32_t size, SeiContent& content) {
  if (!packet || size <= 0)
    return -1;

  uint8_t* sei = packet;
  uint32_t sei_type = 0;
  uint32_t sei_size = 0;
  /// payload type
  do {
    sei_type += *sei;
  } while (*sei++ == 0xff);
  /// 数据长度
  do {
    sei_size += *sei;
  } while (*sei++ == 0xff);

  /// 检查UUID
  if (sei_size >= UUID_SIZE && sei_size <= (packet + size - sei) && sei_type == SEI_PAYLOAD_TYPE) {
    content.uuid = sei;

    sei += UUID_SIZE;
    sei_size -= UUID_SIZE;

    content.data = sei;
    content.size = (uint32_t) (packet + size - sei);
    content.payloadSize = sei_size;

    return sei_size;
  }
  return -1;
}

int32_t GetAnnexbAllSEIContent(uint8_t* packet, uint32_t size,
                               std::vector<std::vector<uint8_t>>& uuidList,
                               std::vector<std::string>& seiPayloads) {
  uint8_t* data = packet;
  uint32_t data_size = size;
  uint8_t* nalu_element = nullptr;
  int32_t nalu_element_size = 0;

  uuidList.clear();
  seiPayloads.clear();

  while (data < packet + size) {
    int32_t index = FindAnnexb(data, data_size);
    int32_t second_index = 0;
    if (index != -1) {
      uint32_t startCodeSize = GetAnnexbStartCodeSize(data + index, data_size - index);
      second_index = FindAnnexb(data + index + startCodeSize, data_size - index - startCodeSize);
      if (second_index >= 0) {
        second_index += startCodeSize;
      }
    }

    if (index == -1)
      return -1;

    if (second_index == -1) {
      second_index = data_size;
    }

    nalu_element = data + index;
    nalu_element_size = second_index;
    data += second_index;
    data_size -= second_index;

    if (nalu_element != nullptr && nalu_element_size != 0) {
      if ((int32_t) (packet + size - nalu_element) < nalu_element_size) {
        nalu_element_size = (int32_t) (packet + size - nalu_element);
      }

      uint32_t startCodeSize = GetAnnexbStartCodeSize(nalu_element, nalu_element_size);
      if (startCodeSize == 0) {
        continue;
      }
      /// SEI
      if ((nalu_element[startCodeSize] & TYPE_MASK) == SEI_TYPE) {
        uint8_t* sei_data = nalu_element + startCodeSize + 1;
        int32_t sei_data_length = nalu_element_size - startCodeSize - 1;
        SeiContent content;
        int32_t ret = GetSEIContent(sei_data, sei_data_length, content);
        if (ret != -1) {
          std::string seiPayload;
          std::vector<uint8_t> uuid;
          uuid.resize(UUID_SIZE);
          std::memcpy(uuid.data(), content.uuid, UUID_SIZE);
          int32_t uncompete_size = GetContentSizeWithoutComplete(content.data, content.size);
          if (uncompete_size) {
            uncompete_size = std::min((int32_t) content.payloadSize, uncompete_size);
            auto outData = (uint8_t*) malloc(uncompete_size + 1);
            std::memset(outData, 0, uncompete_size + 1);
            DeleteCompleteContent(content, outData, uncompete_size);
            seiPayload = (char*) outData;
            free(outData);
          }
          uuidList.push_back(uuid);
          seiPayloads.push_back(seiPayload);
        }
      }
    }
  }
  return 0;
}

int32_t GetAnnexbSEIContent(uint8_t* packet, uint32_t size, const uint8_t* uuid, uint8_t** pdata, uint32_t* psize) {
  uint8_t* data = packet;
  uint32_t data_size = size;
  uint8_t* nalu_element = nullptr;
  int32_t nalu_element_size = 0;

  while (data < packet + size) {
    int32_t index = FindAnnexb(data, data_size);
    int32_t second_index = 0;
    if (index != -1) {
      uint32_t startCodeSize = GetAnnexbStartCodeSize(data + index, data_size - index);
      second_index = FindAnnexb(data + index + startCodeSize, data_size - index - startCodeSize);
      if (second_index >= 0) {
        second_index += startCodeSize;
      }
    }

    if (index == -1)
      return -1;

    if (second_index == -1) {
      second_index = data_size;
    }

    nalu_element = data + index;
    nalu_element_size = second_index;
    data += second_index;
    data_size -= second_index;

    if (nalu_element != nullptr && nalu_element_size != 0) {
      if ((int32_t) (packet + size - nalu_element) < nalu_element_size) {
        nalu_element_size = (int32_t) (packet + size - nalu_element);
      }

      uint32_t startCodeSize = GetAnnexbStartCodeSize(nalu_element, nalu_element_size);
      if (startCodeSize == 0) {
        continue;
      }
      /// SEI
      if ((nalu_element[startCodeSize] & TYPE_MASK) == SEI_TYPE) {
        uint8_t* sei_data = nalu_element + startCodeSize + 1;
        int32_t sei_data_length = nalu_element_size - startCodeSize - 1;
        SeiContent content;
        int32_t ret = GetSEIContent(sei_data, sei_data_length, content);
        if (ret != -1) {
          if (memcmp(uuid, content.uuid, UUID_SIZE) == 0) {
            int32_t uncompete_size = GetContentSizeWithoutComplete(content.data, content.size);
            if (uncompete_size > 0 && pdata != nullptr) {
              uncompete_size = std::min((int32_t) content.payloadSize, uncompete_size);
              auto outData = (uint8_t*) malloc(uncompete_size + 1);
              std::memset(outData, 0, uncompete_size + 1);
              DeleteCompleteContent(content, outData, uncompete_size);
              *pdata = outData;
            }
            if (psize != nullptr) *psize = uncompete_size;
            return uncompete_size;
          }
        }
      }
    }
  }
  return -1;
}

int32_t GetAVCCAllSEIContent(uint8_t* packet, uint32_t size,
                             std::vector<std::vector<uint8_t>>& uuidList,
                             std::vector<std::string>& seiPayloads) {
  uuidList.clear();
  seiPayloads.clear();
  uint8_t* data = packet;
  /// 当前NALU
  while (data < packet + size) {
    /// 长度
    auto nalu_length = (uint32_t*) data;
    uint32_t nalu_size = ReverseBytes(*nalu_length);
    /// NALU header
    if ((*(data + AVCC_START_CODE_SIZE) & TYPE_MASK) == SEI_TYPE) {
      /// SEI
      uint8_t* sei_data = data + AVCC_START_CODE_SIZE + 1;
      uint32_t sei_data_length = std::min(nalu_size, (uint32_t) (packet + size - sei_data));
      SeiContent content;
      int ret = GetSEIContent(sei_data, sei_data_length, content);
      if (ret != -1) {
        std::string seiPayload;
        std::vector<uint8_t> uuid;
        uuid.reserve(UUID_SIZE);
        std::memcpy(uuid.data(), content.uuid, UUID_SIZE);
        int32_t uncompete_size = GetContentSizeWithoutComplete(content.data, content.size);
        if (uncompete_size) {
          uncompete_size = std::min((int32_t) content.payloadSize, uncompete_size);
          auto outData = (uint8_t*) malloc(uncompete_size + 1);
          std::memset(outData, 0, uncompete_size + 1);
          DeleteCompleteContent(content, outData, uncompete_size);
          seiPayload = (char*) outData;
          free(outData);
        }
        uuidList.push_back(uuid);
        seiPayloads.push_back(seiPayload);
      }
    }
    data += 4 + nalu_size;
  }
  return 0;
}

int32_t GetAVCCSEIContent(uint8_t* packet, uint32_t size, const uint8_t* uuid, uint8_t** pdata, uint32_t* psize) {
  uint8_t* data = packet;
  /// 当前NALU
  while (data < packet + size) {
    /// 长度
    auto nalu_length = (uint32_t*) data;
    uint32_t nalu_size = ReverseBytes(*nalu_length);
    /// NALU header
    if ((*(data + AVCC_START_CODE_SIZE) & TYPE_MASK) == SEI_TYPE) {
      /// SEI
      uint8_t* sei_data = data + AVCC_START_CODE_SIZE + 1;
      uint32_t sei_data_length = std::min(nalu_size, (uint32_t) (packet + size - sei_data));
      SeiContent content;
      int ret = GetSEIContent(sei_data, sei_data_length, content);
      if (ret != -1) {
        if (memcmp(uuid, content.uuid, UUID_SIZE) == 0) {
          uint32_t uncompete_size = GetContentSizeWithoutComplete(content.data, content.size);
          if (uncompete_size > 0 && pdata != nullptr) {
            uncompete_size = std::min(content.payloadSize, uncompete_size);
            auto outData = (uint8_t*) malloc(uncompete_size + 1);
            memset(outData, 0, uncompete_size + 1);
            DeleteCompleteContent(content, outData, uncompete_size);
            *pdata = outData;
          }
          if (psize != nullptr) {
            *psize = uncompete_size;
          }
          return uncompete_size;
        }
      }
    }
    data += 4 + nalu_size;
  }
  return -1;
}

bool FillSEIPacket(uint8_t* packet, uint32_t annexbType, const uint8_t* uuid, const uint8_t* data, uint32_t size) {
  if (!packet || !uuid) {
    return false;
  }

  uint8_t* nalu_data = packet;
  uint32_t nalu_size = GetSEINaluSize(data, size);
  uint32_t sei_size = nalu_size;
  /// 大端转小端
  nalu_size = ReverseBytes(nalu_size);

  /// NALU 起始码
  uint32_t* size_ptr = &nalu_size;
  if (annexbType == 2) {
    memcpy(nalu_data, ANNEXB_CODE_LOW, sizeof(unsigned char) * 3);
    nalu_data += sizeof(unsigned char) * 3;
  } else if (annexbType == 1) {
    memcpy(nalu_data, ANNEXB_CODE, sizeof(uint32_t));
    nalu_data += sizeof(uint32_t);
  } else {
    memcpy(nalu_data, size_ptr, sizeof(uint32_t));
    nalu_data += sizeof(uint32_t);
  }

  uint8_t* sei_nalu = nalu_data;
  /// SEI NAL
  *nalu_data++ = SEI_TYPE;
  /// sei payload
  *nalu_data++ = SEI_PAYLOAD_TYPE;
  uint32_t sei_payload_size = size + UUID_SIZE;
  /// 数据长度
  while (true) {
    *nalu_data++ = (sei_payload_size >= 0xFF ? 0xFF : (char) sei_payload_size);
    if (sei_payload_size < 0xFF) {
      break;
    }
    sei_payload_size -= 0xFF;
  }

  /// UUID
  std::memcpy(nalu_data, uuid, UUID_SIZE);
  nalu_data += UUID_SIZE;
  uint32_t content_size = GetContentSizeWithComplete(data, size);
  /// 数据
  if (data != nullptr) {
    if (content_size == size) {
      std::memcpy(nalu_data, data, size);
      nalu_data += size;
    } else {
      int zero_count = 0;
      for (uint32_t i = 0; i < size; i++) {
        if (zero_count >= 2) {
          *nalu_data++ = 0x03;
          *nalu_data++ = data[i];
          zero_count = 0;
        } else {
          *nalu_data++ = data[i];
          if (data[i] == 0x00) {
            zero_count++;
          } else {
            zero_count = 0;
          }
        }
      }
    }
  }

  /// tail 截止对齐码
  if (sei_nalu + sei_size - nalu_data == 1) {
    *nalu_data = 0x80;
  } else if (sei_nalu + sei_size - nalu_data == 2) {
    *nalu_data++ = 0x00;
    *nalu_data++ = 0x80;
  }

  return true;
}

void FreeSEIContent(uint8_t** pdata) {
  if (pdata == nullptr)
    return;
  if (*pdata != nullptr) {
    free(*pdata);
    *pdata = nullptr;
  }
}

}

}