# bymlplus
### A lightweight C++ library for parsing Nintendo BYML or BYAML

* **Supports v2 and v3 files**. These versions are respectively used by *The Legend of Zelda: Breath of the Wild* and *Super Mario Odyssey*.
* **Supports 64-bit node types** which are used in Super Mario Odyssey.
* **Supports both endianness**. The little-endian format is used on the Switch.
* Low overhead; no dynamic memory allocation.
* API is similar to Nintendo's official BYML parser; conversions work exactly the same.

## Quick usage
### Integration
```c++
#include <byml/byml.h>
```

and link to the `byml::byml` interface target.

### Reader
```c++
const byml::Reader r{byml::Buffer{ptr, size}};
```
*The data pointer must stay valid as long as the reader exists.*

It is strongly recommended to validate the BYML by calling `isValid()` before doing anything else
to avoid crashing because of malformed data.

### Containers
Use `getArray` or `getHash` to obtain the root container:
```c++
std::optional<byml::Array> array = r.getArray();
// or
std::optional<byml::Hash> hash = r.getHash();
```
*The reader must outlive any container or item.*

You can iterate over containers and get items from them:
```c++
size_t size = container.numItems();

std::optional<ItemType> item = container.getByIndex(idx);

for (const ItemType& item : container) {
  // ...
}
```

For arrays, ItemType is byml::ItemData. For hashes, ItemType is byml::HashItem (name + item data).

#### Arrays
Arrays also have a subscript operator. *The index is assumed to be valid.*
```c++
byml::ItemData item = array[idx];
```

#### Hashes
Hashes have the following extra functions:
```c++
const char* key = "actors";

std::optional<byml::ItemData> item = hash.getByKey(key);
// WARNING: the key is assumed to be valid.
byml::ItemData item = hash[key];

bool containsActors = hash.contains(key);
```

Hashes can be iterated on directly, with `keys()` or with `values()`. You get ranges of `byml::HashItem`, `const char*` and `ItemData` respectively.

### Items
For a Hash node:
```c++
std::optional<Hash> value = item.getHash();
```
For an Array node:
```c++
std::optional<Array> value = item.getArray();
```
For a String node:
```c++
const char* value = item.getString();
```
For a Bool node, use `getBool`. This returns true if the BYML value is non zero and false otherwise.
```c++
std::optional<bool> value = item.getBool();
```
For an Int node, use `getInt`.
```c++
std::optional<s32> value = item.getInt();
```
To get an unsigned integer, use `getUInt`. Int and UInt nodes are accepted. For Int nodes, the value must be greater than or equal to 0.
```c++
std::optional<u32> value = item.getUInt();
```
For a Float node:
```c++
std::optional<f32> value = item.getFloat();
```
To get a 64-bit integer, use `getInt64`. Int and Int64 nodes are accepted.
```c++
std::optional<s64> value = item.getInt64();
```
To get a 64-bit unsigned integer, use `getUInt64`. UInt, UInt64, Int and Int64 nodes are accepted. For Int and Int64 nodes, the value must be greater than or equal to 0.
```c++
std::optional<u64> value = item.getUInt64();
```
For a Double node:
```c++
std::optional<f64> value = item.getDouble();
```

The value can also be returned as a std::variant. The type of the contained value is determined by the node type.
```c++
byml::ItemData::Variant value = item.val();
```

## Python bindings
Python bindings are also available thanks to pybind11. They can be installed by running `pip3 install pybind11/`.

The API is pretty much the same, though there are some minor differences. You do not need to care about lifetime; the library will ensure that objects stay alive as long as needed.

### Reader
```python
import bymlplus
r = bymlplus.Reader(bymlplus.Buffer(byteslike))
```

It is strongly recommended to validate the BYML by calling `isValid()` before doing anything else
to avoid crashing because of malformed data.

### Containers
Use `getArray` or `getHash` to obtain the root container:
```python
container: typing.Optional[byml.Array] = r.getArray()
# or
container: typing.Optional[byml.Hash] = r.getHash()
```

You can iterate over containers and get items from them:
```python
size = len(container)

# IndexError is raised for bad indexes.
item = container[idx]

for item in container:
    ...
```

Hashes have a overloaded subscript operator that takes strings:
```python
# KeyError is raised for bad keys.
item = container["actors"]
```

Hashes also support iteration and some standard dict functions: \_\_contains\_\_, keys, values, items.

### Items
The `getXXX` functions work exactly the same as in the C++ API.

Alternatively, just use `val` to get the value:
```python
value = item.val()
# value is an Array, Hash, int, etc. depending on the node type
```

## License
This software is licensed under the terms of the GNU General Public License, version 2 or later.
