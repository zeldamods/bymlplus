// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+
#pragma once

#include "byml/binary_format.h"
#include "byml/types.h"
#include "byml/value.h"
#include "common/align.h"
#include "common/binary_reader.h"

namespace byml::util {

inline u64 getStringOffset(common::BinaryReader br, u64 tableOffset, u32 idx) {
  return tableOffset + br.read<u32>(tableOffset + 4 + 4 * idx);
}

/// Get the number of items in a container.
inline u32 readContainerSize(common::BinaryReader br, u64 offset) {
  return br.readU24(offset + 1);
}

// Array utilities.

constexpr u64 getArrayTypesOffset(u64 offset) {
  return offset + 4;
}

constexpr u64 getArrayValuesOffset(u64 offset, u32 numItems) {
  return getArrayTypesOffset(offset) + common::AlignUp(numItems, 4);
}

/// Get an item (type + raw data) in an array.
inline RawItemData readArrayItem(common::BinaryReader br, u64 typesOffset, u64 valuesOffset,
                                 u32 idx) {
  return {br.read<u32>(valuesOffset + 4 * idx), NodeType(br.read<u8>(typesOffset + idx))};
}

// Hash utilities.

constexpr u64 getHashItemsOffset(u64 offset) {
  return offset + 4;
}

constexpr u64 getHashItemOffset(u64 offset, u32 idx) {
  return getHashItemsOffset(offset) + 8 * idx;
}

struct RawHashItem {
  u32 keyIndex;
  RawItemData data;
};

/// Get an item (key index + type + raw data) in a hash.
inline RawHashItem readHashItemWithItemOffset(common::BinaryReader br, u64 itemOffset) {
  const u32 keyIndex = br.readU24(itemOffset);
  const auto type = NodeType(br.read<u8>(itemOffset + 3));
  const u32 rawData = br.read<u32>(itemOffset + 4);
  return {keyIndex, {rawData, type}};
}
inline RawHashItem readHashItem(common::BinaryReader br, u64 offset, u32 idx) {
  return readHashItemWithItemOffset(br, getHashItemOffset(offset, idx));
}

}  // namespace byml::util
