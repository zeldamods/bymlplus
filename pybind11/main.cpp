// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <byml/binary_format.h>
#include <byml/byml.h>
#include <byml/value.h>

namespace py = pybind11;

using namespace pybind11::literals;

namespace {

template <typename T>
py::class_<T> registerBymlContainerClass(const py::module& m, const char* name) {
  return py::class_<T>(m, name)
      .def("__len__", &T::numItems)
      .def("__getitem__",
           [](const T& ct, std::size_t idx) {
             if (auto value = ct.getByIndex(idx))
               return *value;
             throw py::index_error{std::to_string(idx)};
           },
           "idx"_a, py::keep_alive<0, 1>())
      .def("__repr__", [name](const T& ct) {
        return py::str("<byml.{} size={}>").format(name, ct.numItems());
      });
}

template <typename T>
auto rangeToIter(const T& range) {
  return py::make_iterator(range.begin(), range.end());
}

}  // namespace

PYBIND11_MODULE(bymlplus, m) {
  using namespace byml;

  // binary_format.h
  py::enum_<NodeType>(m, "NodeType")
      .value("String", NodeType::String)
      .value("Array", NodeType::Array)
      .value("Hash", NodeType::Hash)
      .value("StringTable", NodeType::StringTable)
      .value("Bool", NodeType::Bool)
      .value("Int", NodeType::Int)
      .value("Float", NodeType::Float)
      .value("UInt", NodeType::UInt)
      .value("Int64", NodeType::Int64)
      .value("UInt64", NodeType::UInt64)
      .value("Double", NodeType::Double)
      .value("Null", NodeType::Null);

  // byml.h
  py::class_<Buffer>(m, "Buffer")
      .def(py::init([](py::buffer b) -> Buffer {
             py::buffer_info info = b.request();
             if (info.itemsize != 1 || info.ndim != 1 || info.size <= 0)
               throw std::runtime_error("needs a non-empty unsigned char* like buffer");
             return {static_cast<u8*>(info.ptr), static_cast<size_t>(info.size)};
           }),
           "buf"_a, py::keep_alive<1, 2>())
      .def("__len__", [](const Buffer& buffer) { return buffer.size(); })
      .def("__repr__", [](const Buffer& buffer) {
        return py::str("<byml.Buffer len={} bytes>").format(buffer.size());
      });

  py::class_<Reader>(m, "Reader")
      .def(py::init<Buffer>(), "buffer"_a, py::keep_alive<1, 2>())
      .def("isValid", &Reader::isValid)
      .def("isArray", &Reader::isArray)
      .def("isHash", &Reader::isHash)
      .def("getVersion", &Reader::getVersion)
      .def("getArray", &Reader::getArray, py::keep_alive<0, 1>())
      .def("getHash", &Reader::getHash, py::keep_alive<0, 1>())
      .def("__repr__", [](const Reader& reader) {
        const char* type = "???";
        if (reader.isArray())
          type = "array";
        else if (reader.isHash())
          type = "hash";
        return py::str("<byml.Reader type={}>").format(type);
      });

  // value.h
  registerBymlContainerClass<Array>(m, "Array")
      .def("__iter__", [](const Array& a) { return rangeToIter(a); }, py::keep_alive<0, 1>());

  registerBymlContainerClass<Hash>(m, "Hash")
      .def("__getitem__",
           [](const Hash& h, const char* key) {
             if (auto value = h.getByKey(key))
               return *value;
             throw py::key_error{key};
           },
           "key"_a, py::keep_alive<0, 1>())
      .def("__contains__", [](const Hash& h, const char* k) { return h.contains(k); }, "key"_a)
      .def("__iter__", [](const Hash& h) { return rangeToIter(h.keys()); }, py::keep_alive<0, 1>())
      .def("keys", [](const Hash& h) { return rangeToIter(h.keys()); }, py::keep_alive<0, 1>())
      .def("values", [](const Hash& h) { return rangeToIter(h.values()); }, py::keep_alive<0, 1>())
      .def("items", [](const Hash& h) { return rangeToIter(h); }, py::keep_alive<0, 1>());

  py::class_<RawItemData>(m, "RawItemData")
      .def_readonly("raw", &RawItemData::raw)
      .def_readonly("type", &RawItemData::type);

  py::class_<ItemData>(m, "ItemData")
      .def_readonly("raw", &ItemData::raw)
      .def("getHash", &ItemData::getHash, py::keep_alive<0, 1>())
      .def("getArray", &ItemData::getArray, py::keep_alive<0, 1>())
      .def("getString", &ItemData::getString)
      .def("getBool", &ItemData::getBool)
      .def("getInt", &ItemData::getInt)
      .def("getUInt", &ItemData::getUInt)
      .def("getFloat", &ItemData::getFloat)
      .def("getInt64", &ItemData::getInt64)
      .def("getUInt64", &ItemData::getUInt64)
      .def("getDouble", &ItemData::getDouble)
      .def("val",
           [](const ItemData& i) {
             auto v = i.val();
             // Forbid using val() to get containers because of lifetime issues.
             if (std::holds_alternative<Hash>(v) || std::holds_alternative<Array>(v))
               throw std::invalid_argument{"use getHash or getArray for container items"};
             return v;
           })
      .def("valu", &ItemData::val,
           "Unsafe variant: same as val() but assumes that the user will keep the reader instance "
           "valid as long as necessary.")
      .def("__repr__",
           [](const ItemData& i) { return py::str("<byml.ItemData: {}>").format(i.val()); });

  py::class_<HashItem>(m, "HashItem")
      .def_readonly("name", &HashItem::name)
      .def_readonly("data", &HashItem::data)
      .def("__repr__", [](const HashItem& i) {
        return py::str("<byml.HashItem: {} = {}>").format(i.name, i.data.val());
      });
}
