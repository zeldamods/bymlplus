// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+
#pragma once

#include <array>

#include <byml/types.h>

namespace byml {

struct ResHeader {
  /// “BY” (big endian) or “YB” (little endian).
  std::array<char, 2> magic;
  /// Format version (2 or 3).
  u16 version;
  /// Offset to the hash key table, relative to start (usually 0x010)
  /// May be 0 if no hash nodes are used. Must be a string table node (0xc2).
  u32 hashKeyTableOffset;
  /// Offset to the string table, relative to start. May be 0 if no strings are used.
  /// Must be a string table node (0xc2).
  u32 stringTableOffset;
  /// Offset to the root node, relative to start. May be 0 if the document is totally empty.
  /// Must be either an array node (0xc0) or a hash node (0xc1).
  u32 rootNodeOffset;
};
static_assert(sizeof(ResHeader) == 0x10);

enum class NodeType : u8 {
  String = 0xa0,
  Array = 0xc0,
  Hash = 0xc1,
  StringTable = 0xc2,
  Bool = 0xd0,
  Int = 0xd1,
  Float = 0xd2,
  UInt = 0xd3,
  Int64 = 0xd4,
  UInt64 = 0xd5,
  Double = 0xd6,
  Null = 0xff,
};

constexpr bool isContainerType(NodeType type) {
  return type == NodeType::Array || type == NodeType::Hash;
}

constexpr bool isValueType(NodeType type) {
  return type == NodeType::String || type == NodeType::Null ||
         (NodeType::Bool <= type && type <= NodeType::UInt);
}

}  // namespace byml
