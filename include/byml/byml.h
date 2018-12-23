// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+
#pragma once

#include <cstdint>
#include <optional>

#include <byml/types.h>
#include <byml/value.h>

namespace byml {

/// Non-owning buffer for binary data.
class Buffer {
public:
  Buffer(const u8* data, size_t size) : mData{data}, mSize{size} {}

  const u8* data() const { return mData; }
  size_t size() const { return mSize; }
  operator const u8*() const { return data(); }

private:
  const u8* mData;
  size_t mSize;
};

/// BYML reader.
class Reader {
public:
  Reader(Buffer buffer);
  ~Reader();

  /// Returns whether the BYML is well-formed. This should be checked before doing anything else.
  bool isValid() const;
  /// Returns whether the root node is an array.
  bool isArray() const;
  /// Returns whether the root node is a hash (aka a dictionary or map).
  bool isHash() const;

  u16 getVersion() const;

  /// Get the root array node. Returns nullopt if root node does not have the correct type.
  std::optional<Array> getArray() const;
  /// Get the root hash node. Returns nullopt if root node does not have the correct type.
  std::optional<Hash> getHash() const;

  Buffer getBuffer() const { return mBuffer; }
  bool isBigEndian() const { return mBigEndian; }
  u32 getHashKeyTableOffset() const { return mHashKeyTableOffset; }
  u32 getStringTableOffset() const { return mStringTableOffset; }

private:
  Buffer mBuffer;

  u32 mHashKeyTableOffset = 0;
  u32 mStringTableOffset = 0;
  u32 mRootNodeOffset = 0;
  bool mHasValidHeader = false;

  bool mBigEndian = false;
};

}  // namespace byml
