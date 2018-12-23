// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+
#pragma once

#include <cstdint>
#include <optional>
#include <range/v3/core.hpp>
#include <range/v3/view/transform.hpp>
#include <variant>

#include <byml/binary_format.h>
#include <byml/types.h>

namespace byml {

class Reader;

class ContainerBase {
public:
  ContainerBase(const Reader& reader, u32 offset);
  /// Get the number of items in the container.
  size_t numItems() const { return mNumItems; }

protected:
  const Reader& mReader;
  u32 mOffset;
  u32 mNumItems;
};

/// Generic interface for a BYML container.
template <typename T, typename ItemType>
class Container : public ContainerBase, public ranges::view_facade<Container<T, ItemType>> {
public:
  using ContainerBase::ContainerBase;

  /// Get an item by its index.
  std::optional<ItemType> getByIndex(size_t idx) const {
    return static_cast<const T*>(this)->getByIndexImpl(idx);
  }

private:
  friend ranges::range_access;
  struct Cursor {
    Cursor() = default;
    Cursor(const Container& container, size_t idx) : mContainer{container}, mIdx{idx} {}
    ItemType read() const { return *mContainer.get().getByIndex(mIdx); }
    bool equal(const Cursor& other) const { return mIdx == other.mIdx; }
    void next() { advance(1); }
    void prev() { advance(-1); }
    void advance(std::ptrdiff_t n) { mIdx = mIdx + n; }

  private:
    ranges::reference_wrapper<const Container> mContainer;
    size_t mIdx;
  };
  Cursor begin_cursor() const { return {*this, 0}; }
  Cursor end_cursor() const { return {*this, numItems()}; }
};

class Array;
class Hash;

struct RawItemData {
  /// Raw node data. Already byteswapped if necessary.
  u32 raw;
  /// Node type.
  NodeType type;

  operator u32() const { return raw; }
};

/// BYML container item data.
struct ItemData {
  const Reader& reader;
  const RawItemData raw;

  std::optional<Hash> getHash() const;
  std::optional<Array> getArray() const;
  const char* getString() const;
  std::optional<bool> getBool() const;
  std::optional<s32> getInt() const;
  std::optional<u32> getUInt() const;
  std::optional<f32> getFloat() const;
  std::optional<s64> getInt64() const;
  std::optional<u64> getUInt64() const;
  std::optional<f64> getDouble() const;

  using Variant = std::variant<Hash, Array, const char*, bool, s32, u32, f32, s64, u64, f64>;
  /// Get the value as a variant. This is more convenient in some cases but less efficient.
  Variant val() const;
};

/// BYML array.
class Array : public Container<Array, ItemData> {
public:
  using Container::Container;

  /// Get an item by its index (assumed to be valid).
  ItemData operator[](size_t idx) const { return *getByIndex(idx); }

private:
  friend Container<Array, ItemData>;
  std::optional<ItemData> getByIndexImpl(size_t idx) const;
};

struct HashItem {
  const char* name;
  ItemData data;
};
/// BYML hash (aka dictionary or map).
class Hash : public Container<Hash, HashItem> {
public:
  using Container::Container;

  /// Get an item by its key.
  std::optional<ItemData> getByKey(const char* key) const;
  /// Get an item by its key (assumed to be valid).
  ItemData operator[](const char* key) const { return *getByKey(key); }
  /// Prevents implicit conversions from 0 to const char* and other mistakes.
  ItemData operator[](int key) const = delete;

  /// Checks if the hash contains an element with the specified key.
  bool contains(const char* key) const { return getByKey(key).has_value(); }

  auto keys() const {
    return *this | ranges::view::transform([](const HashItem& item) { return item.name; });
  }

  auto values() const {
    return *this | ranges::view::transform([](const HashItem& item) { return item.data; });
  }

private:
  friend Container<Hash, HashItem>;
  std::optional<HashItem> getByIndexImpl(size_t idx) const;
};

}  // namespace byml
