// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+

#include "byml/value.h"

#include <cstring>

#include "byml/binary_format.h"
#include "byml/byml.h"
#include "byml/container_util.h"
#include "byml/types.h"
#include "common/binary_reader.h"
#include "common/swap.h"

namespace byml {

static common::BinaryReader getBinaryReader(const Reader& reader) {
  return {reader.getBuffer(), reader.isBigEndian()};
}

ContainerBase::ContainerBase(const Reader& reader, u32 offset) : mReader{reader}, mOffset{offset} {
  mNumItems = util::readContainerSize(getBinaryReader(mReader), mOffset);
}

std::optional<ItemData> Array::getByIndexImpl(size_t idx) const {
  const common::BinaryReader br{getBinaryReader(mReader)};

  if (numItems() <= idx)
    return {};
  const u64 typesOffset = util::getArrayTypesOffset(mOffset);
  const u64 valuesOffset = util::getArrayValuesOffset(mOffset, numItems());
  return ItemData{mReader, util::readArrayItem(br, typesOffset, valuesOffset, idx)};
}

namespace {
inline HashItem hashGetByIndex(const Reader& reader, common::BinaryReader br, u32 offset,
                               u32 hashKeyTableOffset, size_t idx) {
  const auto item = util::readHashItem(br, offset, idx);
  const char* key = br.getString(util::getStringOffset(br, hashKeyTableOffset, item.keyIndex));
  return {key, {reader, item.data}};
}
}  // end of anonymous namespace

std::optional<HashItem> Hash::getByIndexImpl(size_t idx) const {
  const common::BinaryReader br{getBinaryReader(mReader)};

  if (numItems() <= idx)
    return {};
  return hashGetByIndex(mReader, br, mOffset, mReader.getHashKeyTableOffset(), idx);
}

std::optional<ItemData> Hash::getByKey(const char* key) const {
  const common::BinaryReader br{getBinaryReader(mReader)};

  // Since all items are lexicographically sorted, a binary search can be performed here.
  // Holding the indexes in signed 32-bit integers is fine
  // since a BYML container can only contain up to 2**24 items.
  s32 a = 0;
  s32 b = numItems() - 1;
  while (a <= b) {
    s32 m = (a + b) / 2;
    const HashItem item = hashGetByIndex(mReader, br, mOffset, mReader.getHashKeyTableOffset(), m);
    const int cmp = std::strcmp(item.name, key);
    if (cmp < 0)
      a = m + 1;
    else if (cmp > 0)
      b = m - 1;
    else
      return item.data;
  }
  return {};
}

std::optional<Hash> ItemData::getHash() const {
  if (raw.type != NodeType::Hash)
    return {};
  return Hash{reader, raw};
}

std::optional<Array> ItemData::getArray() const {
  if (raw.type != NodeType::Array)
    return {};
  return Array{reader, raw};
}

const char* ItemData::getString() const {
  if (raw.type != NodeType::String)
    return {};
  const common::BinaryReader br{getBinaryReader(reader)};
  return br.getString(util::getStringOffset(br, reader.getStringTableOffset(), raw));
}

std::optional<bool> ItemData::getBool() const {
  if (raw.type != NodeType::Bool)
    return {};
  return raw != 0;
}

std::optional<s32> ItemData::getInt() const {
  if (raw.type != NodeType::Int)
    return {};
  return static_cast<s32>(raw);
}

std::optional<u32> ItemData::getUInt() const {
  switch (raw.type) {
  case NodeType::Int:
    return static_cast<s32>(raw) >= 0 ? raw : std::optional<u32>{};
  case NodeType::UInt:
    return raw;
  default:
    return {};
  }
}

std::optional<f32> ItemData::getFloat() const {
  if (raw.type != NodeType::Float)
    return {};
  float value;
  std::memcpy(&value, &raw.raw, sizeof(value));
  return value;
}

std::optional<s64> ItemData::getInt64() const {
  switch (raw.type) {
  case NodeType::Int:
    return static_cast<s32>(raw);
  case NodeType::UInt:
    return raw;
  case NodeType::Int64:
    return getBinaryReader(reader).read<s64>(raw);
  default:
    return {};
  }
}

std::optional<u64> ItemData::getUInt64() const {
  if (auto value = getUInt())
    return value;

  if (raw.type != NodeType::Int64 && raw.type != NodeType::UInt64)
    return {};

  const u64 value = getBinaryReader(reader).read<u64>(raw);
  if (raw.type == NodeType::Int64 && static_cast<s64>(value) < 0)
    return {};
  return value;
}

std::optional<f64> ItemData::getDouble() const {
  if (auto value = getFloat())
    return value;

  if (raw.type != NodeType::Double)
    return {};

  const u64 rawValue = getBinaryReader(reader).read<u64>(raw);
  double value;
  std::memcpy(&value, &rawValue, sizeof(value));
  return value;
}

ItemData::Variant ItemData::val() const {
  switch (raw.type) {
  case NodeType::Hash:
    return *getHash();
  case NodeType::Array:
    return *getArray();
  case NodeType::String:
    return getString();
  case NodeType::Bool:
    return *getBool();
  case NodeType::Int:
    return *getInt();
  case NodeType::UInt:
    return *getUInt();
  case NodeType::Float:
    return *getFloat();
  case NodeType::Int64:
    return *getInt64();
  case NodeType::UInt64:
    return *getUInt64();
  case NodeType::Double:
    return *getDouble();
  default:
    // Should never happen -- all nodes are checked by the reader.
    return 0x0badbadbadbadbad;
  }
}

}  // namespace byml
