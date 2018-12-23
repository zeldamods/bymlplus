// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+

#include "byml/byml.h"

#include <cstring>

#include "byml/binary_format.h"
#include "byml/container_util.h"
#include "byml/value.h"
#include "common/binary_reader.h"
#include "common/log.h"

namespace byml {

namespace {
struct NodeCheckContext {
  common::BinaryReader br;
  u32 bufferSize = 0;
  u32 hashKeyTableLen = 0;
  u32 stringTableLen = 0;
};

bool checkStringTable(const NodeCheckContext& ctx, u64 offset, u32* numItems) {
  DEBUG_LOG("Checking string table node at offset 0x{:x}", offset);

  if (ctx.bufferSize < offset + 4)
    return false;

  const auto type = NodeType(ctx.br.read<u8>(offset));
  if (type != NodeType::StringTable)
    return false;

  *numItems = ctx.br.readU24(offset + 1);
  if (ctx.bufferSize < offset + 4 + 4 * (*numItems + 1))
    return false;

  for (u32 i = 0; i < *numItems; ++i) {
    const u64 stringOffset = util::getStringOffset(ctx.br, offset, i);
    if (ctx.bufferSize <= stringOffset)
      return false;

    // Verify that all strings are null terminated.
    const size_t maxLen = ctx.bufferSize - stringOffset;
    const size_t len = strnlen(ctx.br.getString(stringOffset), maxLen);
    if (len == maxLen) {
      ERR_LOG("String at 0x{:x} is too long", stringOffset);
      return false;
    }
  }

  return true;
}

bool checkNode(const NodeCheckContext& ctx, u64 data, NodeType type);

bool checkArrayNode(const NodeCheckContext& ctx, u64 offset) {
  DEBUG_LOG("Checking array node at offset 0x{:x}", offset);

  if (ctx.bufferSize < offset + 4) {
    ERR_LOG("Buffer is too small: 0x{:x} < 0x{:x}", ctx.bufferSize, offset + 4);
    return false;
  }

  if (NodeType(ctx.br.read<u8>(offset)) != NodeType::Array) {
    ERR_LOG("Unexpected node type");
    return false;
  }

  const u32 numItems = util::readContainerSize(ctx.br, offset);
  const u64 typesOffset = util::getArrayTypesOffset(offset);
  const u64 valuesOffset = util::getArrayValuesOffset(offset, numItems);
  if (ctx.bufferSize < valuesOffset + 4 * numItems) {
    ERR_LOG("Buffer is too small: 0x{:x} < 0x{:x}", ctx.bufferSize, valuesOffset + 4 * numItems);
    return false;
  }

  for (u32 i = 0; i < numItems; ++i) {
    const auto item = util::readArrayItem(ctx.br, typesOffset, valuesOffset, i);
    if (!checkNode(ctx, item.raw, item.type)) {
      ERR_LOG("Node check failed for array @ 0x{:x}, child {} with type 0x{:x} and data 0x{:x}",
              offset, i, int(item.type), item.raw);
      return false;
    }
  }

  return true;
}

bool checkHashNode(const NodeCheckContext& ctx, u64 offset) {
  DEBUG_LOG("Checking hash node at offset 0x{:x}", offset);

  if (ctx.bufferSize < offset + 4) {
    ERR_LOG("Buffer is too small: 0x{:x} < 0x{:x}", ctx.bufferSize, offset + 4);
    return false;
  }

  if (NodeType(ctx.br.read<u8>(offset)) != NodeType::Hash) {
    ERR_LOG("Unexpected node type");
    return false;
  }

  const u32 numItems = util::readContainerSize(ctx.br, offset);

  const u64 itemsOffset = util::getHashItemsOffset(offset);
  if (ctx.bufferSize < itemsOffset + 8 * numItems) {
    ERR_LOG("Buffer is too small: 0x{:x} < 0x{:x}", ctx.bufferSize, itemsOffset + 8 * numItems);
    return false;
  }

  for (u32 i = 0; i < numItems; ++i) {
    const auto item = util::readHashItemWithItemOffset(ctx.br, util::getHashItemOffset(offset, i));

    if (ctx.hashKeyTableLen <= item.keyIndex) {
      ERR_LOG("Key index is out of bounds: keyIndex={}, hashKeyTableLen={}", item.keyIndex,
              ctx.hashKeyTableLen);
      return false;
    }

    if (!checkNode(ctx, item.data.raw, item.data.type)) {
      ERR_LOG("Node check failed for hash @ 0x{:x}, child {} with type 0x{:x} and data 0x{:x}",
              offset, i, int(item.data.type), item.data.raw);
      return false;
    }
  }

  return true;
}

bool checkNode(const NodeCheckContext& ctx, u64 data, NodeType type) {
  switch (type) {
  case NodeType::String:
    // data is an index into the string table.
    return data < ctx.stringTableLen;
  case NodeType::Array:
    // data is an offset to the node.
    return checkArrayNode(ctx, data);
  case NodeType::Hash:
    // data is an offset to the node.
    return checkHashNode(ctx, data);
  case NodeType::Bool:
  case NodeType::Int:
  case NodeType::Float:
  case NodeType::UInt:
    // Simple value types. Nothing to check.
    return true;
  case NodeType::Int64:
  case NodeType::UInt64:
  case NodeType::Double:
    // "Big" value types. data is an offset to a 64-bit value.
    return data + 8 < ctx.bufferSize;
  case NodeType::Null:
    // Another simple value type. Nothing to do.
    return true;
  default:
    ERR_LOG("Unknown node type: 0x{:x}", int(type));
    return false;
  }
}

}  // end of anonymous namespace

Reader::Reader(Buffer buffer) : mBuffer{buffer} {
  if (mBuffer.size() < sizeof(ResHeader))
    return;

  const bool isBigEndian = buffer[0] == 'B' && buffer[1] == 'Y';
  const bool isLittleEndian = buffer[0] == 'Y' && buffer[1] == 'B';
  if (!isBigEndian && !isLittleEndian)
    return;
  mBigEndian = isBigEndian;

  const common::BinaryReader br{mBuffer, mBigEndian};

  const u16 version = br.read<u16>(offsetof(ResHeader, version));
  if (version != 2 && version != 3) {
    ERR_LOG("Unknown version: {}", version);
    return;
  }

  mHashKeyTableOffset = br.read<u32>(offsetof(ResHeader, hashKeyTableOffset));
  mStringTableOffset = br.read<u32>(offsetof(ResHeader, stringTableOffset));
  mRootNodeOffset = br.read<u32>(offsetof(ResHeader, rootNodeOffset));
  mHasValidHeader = true;
}

Reader::~Reader() = default;

bool Reader::isValid() const {
  if (!mHasValidHeader)
    return false;

  const common::BinaryReader br{mBuffer, mBigEndian};

  if (mBuffer.size() <= mHashKeyTableOffset || mBuffer.size() <= mStringTableOffset ||
      mBuffer.size() <= mRootNodeOffset) {
    return false;
  }

  NodeCheckContext ctx{br};
  ctx.bufferSize = mBuffer.size();
  if (mHashKeyTableOffset && !checkStringTable(ctx, mHashKeyTableOffset, &ctx.hashKeyTableLen)) {
    ERR_LOG("Hash key table check failed");
    return false;
  }
  if (mStringTableOffset && !checkStringTable(ctx, mStringTableOffset, &ctx.stringTableLen)) {
    ERR_LOG("String table check failed");
    return false;
  }

  if (mRootNodeOffset) {
    // Note: uint64s are used for all user controlled offsets to avoid possible wraparounds.
    if (mBuffer.size() < u64(mRootNodeOffset) + 1)
      return false;

    const auto type = NodeType(br.read<u8>(mRootNodeOffset));
    if (type != NodeType::Array && type != NodeType::Hash) {
      ERR_LOG("Invalid root node type");
      return false;
    }

    if (!checkNode(ctx, mRootNodeOffset, type)) {
      ERR_LOG("Root node check failed");
      return false;
    }
  }

  return true;
}

static bool checkRootNodeType(const u8* data, u32 offset, NodeType type) {
  return offset && NodeType(data[offset]) == type;
}

bool Reader::isArray() const {
  return checkRootNodeType(mBuffer, mRootNodeOffset, NodeType::Array);
}

bool Reader::isHash() const {
  return checkRootNodeType(mBuffer, mRootNodeOffset, NodeType::Hash);
}

u16 Reader::getVersion() const {
  return common::BinaryReader{mBuffer, mBigEndian}.read<u16>(offsetof(ResHeader, version));
}

std::optional<Array> Reader::getArray() const {
  if (!isArray())
    return {};
  return Array{*this, mRootNodeOffset};
}

std::optional<Hash> Reader::getHash() const {
  if (!isHash())
    return {};
  return Hash{*this, mRootNodeOffset};
}

}  // namespace byml
