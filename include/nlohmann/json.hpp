// Minimal embedded nlohmann::json-compatible header.
// NOTE: This is NOT the full upstream library. It implements only the subset
// needed by FitMindAI (objects/arrays, basic types, parsing, and serialization).
// The interface is compatible with `nlohmann::json` for the used operations.
//
// If you later want full compliance, replace this file with the official
// single-header distribution from https://github.com/nlohmann/json.

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace nlohmann {

class json {
public:
  using object_t = std::map<std::string, json>;
  using array_t = std::vector<json>;

  using value_t = std::variant<std::nullptr_t, bool, double, std::string, object_t, array_t>;

  json() : data_(nullptr) {}
  json(std::nullptr_t) : data_(nullptr) {}
  json(bool b) : data_(b) {}
  json(int v) : data_(static_cast<double>(v)) {}
  json(long long v) : data_(static_cast<double>(v)) {}
  json(double v) : data_(v) {}
  json(const char* s) : data_(std::string(s)) {}
  json(std::string s) : data_(std::move(s)) {}
  json(object_t obj) : data_(std::move(obj)) {}
  json(array_t arr) : data_(std::move(arr)) {}

  static json object() { return json(object_t{}); }
  static json array() { return json(array_t{}); }

  [[nodiscard]] bool is_null() const { return std::holds_alternative<std::nullptr_t>(data_); }
  [[nodiscard]] bool is_boolean() const { return std::holds_alternative<bool>(data_); }
  [[nodiscard]] bool is_number() const { return std::holds_alternative<double>(data_); }
  [[nodiscard]] bool is_string() const { return std::holds_alternative<std::string>(data_); }
  [[nodiscard]] bool is_object() const { return std::holds_alternative<object_t>(data_); }
  [[nodiscard]] bool is_array() const { return std::holds_alternative<array_t>(data_); }

  [[nodiscard]] size_t size() const {
    if (is_object()) return std::get<object_t>(data_).size();
    if (is_array()) return std::get<array_t>(data_).size();
    return 0;
  }

  // object access
  [[nodiscard]] json& operator[](const std::string& key) {
    if (!is_object()) data_ = object_t{};
    return std::get<object_t>(data_)[key];
  }

  [[nodiscard]] const json& operator[](const std::string& key) const {
    static const json null_json{};
    if (!is_object()) return null_json;
    const auto& obj = std::get<object_t>(data_);
    auto it = obj.find(key);
    if (it == obj.end()) return null_json;
    return it->second;
  }

  // array access
  [[nodiscard]] json& operator[](size_t idx) {
    if (!is_array()) data_ = array_t{};
    auto& arr = std::get<array_t>(data_);
    if (idx >= arr.size()) arr.resize(idx + 1);
    return arr[idx];
  }

  [[nodiscard]] const json& operator[](size_t idx) const {
    static const json null_json{};
    if (!is_array()) return null_json;
    const auto& arr = std::get<array_t>(data_);
    if (idx >= arr.size()) return null_json;
    return arr[idx];
  }

  template <typename T>
  [[nodiscard]] T value(const std::string& key, const T& default_value) const {
    const json& j = (*this)[key];
    if (j.is_null()) return default_value;
    return j.get<T>();
  }

  [[nodiscard]] bool contains(const std::string& key) const {
    if (!is_object()) return false;
    const auto& obj = std::get<object_t>(data_);
    return obj.find(key) != obj.end();
  }

  template <typename T>
  void set(const std::string& key, const T& v) {
    (*this)[key] = json(v);
  }

  // conversions
  template <typename T>
  [[nodiscard]] T get() const {
    if constexpr (std::is_same_v<T, std::string>) {
      if (!is_string()) throw type_error("not a string");
      return std::get<std::string>(data_);
    } else if constexpr (std::is_same_v<T, bool>) {
      if (!is_boolean()) throw type_error("not a boolean");
      return std::get<bool>(data_);
    } else if constexpr (std::is_arithmetic_v<T>) {
      if (!is_number()) throw type_error("not a number");
      double d = std::get<double>(data_);
      if constexpr (std::is_integral_v<T>) return static_cast<T>(d);
      else return static_cast<T>(d);
    } else {
      static_assert(sizeof(T) == 0, "Unsupported get<> type");
    }
  }

  template <typename T>
  [[nodiscard]] std::vector<T> get_array() const {
    if (!is_array()) throw type_error("not an array");
    const auto& arr = std::get<array_t>(data_);
    std::vector<T> out;
    out.reserve(arr.size());
    for (const auto& el : arr) out.push_back(el.get<T>());
    return out;
  }

  // iteration (object)
  using object_iterator = object_t::iterator;
  using const_object_iterator = object_t::const_iterator;

  [[nodiscard]] object_iterator begin() {
    if (!is_object()) throw type_error("not an object");
    return std::get<object_t>(data_).begin();
  }
  [[nodiscard]] object_iterator end() {
    if (!is_object()) throw type_error("not an object");
    return std::get<object_t>(data_).end();
  }
  [[nodiscard]] const_object_iterator begin() const {
    if (!is_object()) throw type_error("not an object");
    return std::get<object_t>(data_).begin();
  }
  [[nodiscard]] const_object_iterator end() const {
    if (!is_object()) throw type_error("not an object");
    return std::get<object_t>(data_).end();
  }

  // serialization
  [[nodiscard]] std::string dump(int indent = -1) const {
    (void)indent;
    return dump_impl(*this);
  }

  // parsing
  static json parse(const std::string& s) {
    size_t i = 0;
    return parse_value(s, i);
  }

  struct type_error : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

private:
  value_t data_;

  static void skip_ws(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
  }

  static char peek(const std::string& s, size_t i) {
    if (i >= s.size()) return '\0';
    return s[i];
  }

  static json parse_value(const std::string& s, size_t& i) {
    skip_ws(s, i);
    char c = peek(s, i);
    if (c == '{') return parse_object(s, i);
    if (c == '[') return parse_array(s, i);
    if (c == '"') return parse_string(s, i);
    if (c == 't' || c == 'f') return parse_bool(s, i);
    if (c == 'n') return parse_null(s, i);
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number(s, i);
    throw std::runtime_error("json parse error: unexpected char");
  }

  static json parse_null(const std::string& s, size_t& i) {
    if (s.compare(i, 4, "null") != 0) throw std::runtime_error("json parse error: null");
    i += 4;
    return json(nullptr);
  }

  static json parse_bool(const std::string& s, size_t& i) {
    if (s.compare(i, 4, "true") == 0) {
      i += 4;
      return json(true);
    }
    if (s.compare(i, 5, "false") == 0) {
      i += 5;
      return json(false);
    }
    throw std::runtime_error("json parse error: bool");
  }

  static json parse_number(const std::string& s, size_t& i) {
    size_t start = i;
    if (peek(s, i) == '-') ++i;
    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
    if (i < s.size() && s[i] == '.') {
      ++i;
      while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
    }
    if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
      ++i;
      if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
      while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
    }
    double v = 0.0;
    try {
      v = std::stod(s.substr(start, i - start));
    } catch (...) {
      throw std::runtime_error("json parse error: number");
    }
    return json(v);
  }

  static json parse_string(const std::string& s, size_t& i) {
    if (peek(s, i) != '"') throw std::runtime_error("json parse error: string");
    ++i; // skip opening quote
    std::string out;
    while (i < s.size()) {
      char c = s[i++];
      if (c == '"') break;
      if (c == '\\') {
        if (i >= s.size()) throw std::runtime_error("json parse error: escape");
        char e = s[i++];
        switch (e) {
          case '"': out.push_back('"'); break;
          case '\\': out.push_back('\\'); break;
          case '/': out.push_back('/'); break;
          case 'b': out.push_back('\b'); break;
          case 'f': out.push_back('\f'); break;
          case 'n': out.push_back('\n'); break;
          case 'r': out.push_back('\r'); break;
          case 't': out.push_back('\t'); break;
          default: out.push_back(e); break;
        }
      } else {
        out.push_back(c);
      }
    }
    return json(out);
  }

  static json parse_object(const std::string& s, size_t& i) {
    if (peek(s, i) != '{') throw std::runtime_error("json parse error: object");
    ++i;
    object_t obj;
    skip_ws(s, i);
    if (peek(s, i) == '}') {
      ++i;
      return json(std::move(obj));
    }
    while (true) {
      skip_ws(s, i);
      json key = parse_string(s, i);
      skip_ws(s, i);
      if (peek(s, i) != ':') throw std::runtime_error("json parse error: object colon");
      ++i;
      json val = parse_value(s, i);
      obj[key.get<std::string>()] = std::move(val);
      skip_ws(s, i);
      char c = peek(s, i);
      if (c == ',') {
        ++i;
        continue;
      }
      if (c == '}') {
        ++i;
        break;
      }
      throw std::runtime_error("json parse error: object separator");
    }
    return json(std::move(obj));
  }

  static json parse_array(const std::string& s, size_t& i) {
    if (peek(s, i) != '[') throw std::runtime_error("json parse error: array");
    ++i;
    array_t arr;
    skip_ws(s, i);
    if (peek(s, i) == ']') {
      ++i;
      return json(std::move(arr));
    }
    while (true) {
      json val = parse_value(s, i);
      arr.push_back(std::move(val));
      skip_ws(s, i);
      char c = peek(s, i);
      if (c == ',') {
        ++i;
        continue;
      }
      if (c == ']') {
        ++i;
        break;
      }
      throw std::runtime_error("json parse error: array separator");
    }
    return json(std::move(arr));
  }

  static std::string escape_string(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
      switch (c) {
        case '"': oss << "\\\""; break;
        case '\\': oss << "\\\\"; break;
        case '\n': oss << "\\n"; break;
        case '\r': oss << "\\r"; break;
        case '\t': oss << "\\t"; break;
        default: oss << c; break;
      }
    }
    return oss.str();
  }

  static std::string dump_impl(const json& j) {
    if (j.is_null()) return "null";
    if (j.is_boolean()) return std::get<bool>(j.data_) ? "true" : "false";
    if (j.is_number()) {
      double d = std::get<double>(j.data_);
      std::ostringstream oss;
      if (std::isfinite(d)) oss << d;
      else oss << 0;
      return oss.str();
    }
    if (j.is_string()) return "\"" + escape_string(std::get<std::string>(j.data_)) + "\"";
    if (j.is_object()) {
      const auto& obj = std::get<object_t>(j.data_);
      std::ostringstream oss;
      oss << "{";
      bool first = true;
      for (const auto& kv : obj) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << escape_string(kv.first) << "\":" << dump_impl(kv.second);
      }
      oss << "}";
      return oss.str();
    }
    if (j.is_array()) {
      const auto& arr = std::get<array_t>(j.data_);
      std::ostringstream oss;
      oss << "[";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i) oss << ",";
        oss << dump_impl(arr[i]);
      }
      oss << "]";
      return oss.str();
    }
    return "null";
  }
};

} // namespace nlohmann

#endif

