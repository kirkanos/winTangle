// A very small JSON reader and writer. Deliberately hand written rather than
// pulling in a library: WinTangle should work without a package manager and
// without a runtime dependency, and all it needs is a flat config format.
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace wintangle::json {

class Value;
using Array = std::vector<Value>;
// Objects keep their insertion order, so saving the config does not produce an
// arbitrarily reordered file (which would make diffs useless).
using Object = std::vector<std::pair<std::string, Value>>;

enum class Type { Null, Bool, Number, String, Array, Object };

class Value {
public:
    Value() = default;
    Value(std::nullptr_t) {}
    Value(bool b) : type_(Type::Bool), bool_(b) {}
    Value(double n) : type_(Type::Number), number_(n) {}
    Value(int n) : type_(Type::Number), number_(n) {}
    Value(std::string s) : type_(Type::String), string_(std::move(s)) {}
    Value(const char* s) : type_(Type::String), string_(s) {}
    Value(Array a) : type_(Type::Array), array_(std::move(a)) {}
    Value(Object o) : type_(Type::Object), object_(std::move(o)) {}

    Type GetType() const { return type_; }
    bool IsNull() const { return type_ == Type::Null; }
    bool IsObject() const { return type_ == Type::Object; }
    bool IsArray() const { return type_ == Type::Array; }

    bool AsBool(bool fallback = false) const {
        return type_ == Type::Bool ? bool_ : fallback;
    }
    double AsNumber(double fallback = 0.0) const {
        return type_ == Type::Number ? number_ : fallback;
    }
    int AsInt(int fallback = 0) const {
        return type_ == Type::Number ? static_cast<int>(number_) : fallback;
    }
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Object& AsObject() const;

    // Field access on objects. A missing field yields a null value, which
    // makes incomplete config files readable without special cases.
    const Value& operator[](std::string_view key) const;
    bool Has(std::string_view key) const;

    void Set(std::string key, Value v);
    void Push(Value v);

    std::string Dump(int indent = 2) const;

private:
    void DumpTo(std::string& out, int indent, int depth) const;

    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    Array array_;
    Object object_;
};

// Parses JSON. On failure it returns false and fills in `error` (line and
// column), so a broken config file can be pointed at.
bool Parse(std::string_view text, Value& out, std::string& error);

}  // namespace wintangle::json
