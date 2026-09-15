#include "Json.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace wintangle::json {
namespace {

const std::string kEmptyString;
const Array kEmptyArray;
const Object kEmptyObject;
const Value kNullValue;

void EscapeTo(std::string& out, std::string_view s) {
    out += '"';
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;  // UTF-8 wird unveraendert durchgereicht
                }
        }
    }
    out += '"';
}

void NewlineIndent(std::string& out, int indent, int depth) {
    if (indent <= 0) return;
    out += '\n';
    out.append(static_cast<size_t>(indent * depth), ' ');
}

class Parser {
public:
    Parser(std::string_view text) : text_(text) {}

    bool Run(Value& out, std::string& error) {
        SkipWs();
        if (!ParseValue(out)) {
            error = Error();
            return false;
        }
        SkipWs();
        if (pos_ != text_.size()) {
            error = "Unerwartete Zeichen nach dem JSON-Wert (" + Position() + ")";
            return false;
        }
        return true;
    }

private:
    std::string Position() const {
        size_t line = 1, col = 1;
        for (size_t i = 0; i < pos_ && i < text_.size(); ++i) {
            if (text_[i] == '\n') { ++line; col = 1; } else { ++col; }
        }
        return "Zeile " + std::to_string(line) + ", Spalte " + std::to_string(col);
    }
    std::string Error() const {
        return (message_.empty() ? std::string("Ungueltiges JSON") : message_) +
               " (" + Position() + ")";
    }
    bool Fail(std::string msg) {
        if (message_.empty()) message_ = std::move(msg);
        return false;
    }

    void SkipWs() {
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++pos_;
            } else if (c == '/' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '/') {
                // Zeilenkommentare sind kein JSON, aber in einer von Hand
                // gepflegten Konfigdatei zu nuetzlich, um sie zu verbieten.
                while (pos_ < text_.size() && text_[pos_] != '\n') ++pos_;
            } else {
                return;
            }
        }
    }

    bool Consume(char c) {
        if (pos_ < text_.size() && text_[pos_] == c) { ++pos_; return true; }
        return false;
    }

    bool Literal(std::string_view lit, Value v, Value& out) {
        if (text_.compare(pos_, lit.size(), lit) != 0) return Fail("Unbekanntes Literal");
        pos_ += lit.size();
        out = std::move(v);
        return true;
    }

    bool ParseValue(Value& out) {
        if (pos_ >= text_.size()) return Fail("Vorzeitiges Ende");
        switch (text_[pos_]) {
            case '{': return ParseObject(out);
            case '[': return ParseArray(out);
            case '"': {
                std::string s;
                if (!ParseString(s)) return false;
                out = Value(std::move(s));
                return true;
            }
            case 't': return Literal("true", Value(true), out);
            case 'f': return Literal("false", Value(false), out);
            case 'n': return Literal("null", Value(), out);
            default: return ParseNumber(out);
        }
    }

    bool ParseObject(Value& out) {
        ++pos_;  // '{'
        Object obj;
        SkipWs();
        if (Consume('}')) { out = Value(std::move(obj)); return true; }
        while (true) {
            SkipWs();
            std::string key;
            if (!ParseString(key)) return Fail("Objektschluessel erwartet");
            SkipWs();
            if (!Consume(':')) return Fail("':' erwartet");
            SkipWs();
            Value v;
            if (!ParseValue(v)) return false;
            obj.emplace_back(std::move(key), std::move(v));
            SkipWs();
            if (Consume(',')) continue;
            if (Consume('}')) break;
            return Fail("',' oder '}' erwartet");
        }
        out = Value(std::move(obj));
        return true;
    }

    bool ParseArray(Value& out) {
        ++pos_;  // '['
        Array arr;
        SkipWs();
        if (Consume(']')) { out = Value(std::move(arr)); return true; }
        while (true) {
            SkipWs();
            Value v;
            if (!ParseValue(v)) return false;
            arr.push_back(std::move(v));
            SkipWs();
            if (Consume(',')) continue;
            if (Consume(']')) break;
            return Fail("',' oder ']' erwartet");
        }
        out = Value(std::move(arr));
        return true;
    }

    bool ParseString(std::string& out) {
        if (!Consume('"')) return Fail("'\"' erwartet");
        out.clear();
        while (pos_ < text_.size()) {
            const char c = text_[pos_++];
            if (c == '"') return true;
            if (c != '\\') { out += c; continue; }
            if (pos_ >= text_.size()) break;
            const char esc = text_[pos_++];
            switch (esc) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u': {
                    if (pos_ + 4 > text_.size()) return Fail("Unvollstaendige \\u-Sequenz");
                    const unsigned cp = static_cast<unsigned>(
                        std::strtoul(std::string(text_.substr(pos_, 4)).c_str(), nullptr, 16));
                    pos_ += 4;
                    AppendUtf8(out, cp);
                    break;
                }
                default: return Fail("Unbekannte Escape-Sequenz");
            }
        }
        return Fail("Zeichenkette nicht beendet");
    }

    static void AppendUtf8(std::string& out, unsigned cp) {
        if (cp < 0x80) {
            out += static_cast<char>(cp);
        } else if (cp < 0x800) {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    bool ParseNumber(Value& out) {
        const size_t start = pos_;
        if (pos_ < text_.size() && (text_[pos_] == '-' || text_[pos_] == '+')) ++pos_;
        bool digits = false;
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if ((c >= '0' && c <= '9')) { digits = true; ++pos_; }
            else if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') { ++pos_; }
            else break;
        }
        if (!digits) return Fail("Zahl erwartet");
        out = Value(std::strtod(std::string(text_.substr(start, pos_ - start)).c_str(), nullptr));
        return true;
    }

    std::string_view text_;
    size_t pos_ = 0;
    std::string message_;
};

}  // namespace

const std::string& Value::AsString() const {
    return type_ == Type::String ? string_ : kEmptyString;
}
const Array& Value::AsArray() const { return type_ == Type::Array ? array_ : kEmptyArray; }
const Object& Value::AsObject() const { return type_ == Type::Object ? object_ : kEmptyObject; }

const Value& Value::operator[](std::string_view key) const {
    if (type_ == Type::Object) {
        for (const auto& [k, v] : object_) {
            if (k == key) return v;
        }
    }
    return kNullValue;
}

bool Value::Has(std::string_view key) const { return !(*this)[key].IsNull(); }

void Value::Set(std::string key, Value v) {
    if (type_ != Type::Object) { type_ = Type::Object; object_.clear(); }
    for (auto& [k, existing] : object_) {
        if (k == key) { existing = std::move(v); return; }
    }
    object_.emplace_back(std::move(key), std::move(v));
}

void Value::Push(Value v) {
    if (type_ != Type::Array) { type_ = Type::Array; array_.clear(); }
    array_.push_back(std::move(v));
}

void Value::DumpTo(std::string& out, int indent, int depth) const {
    switch (type_) {
        case Type::Null: out += "null"; break;
        case Type::Bool: out += bool_ ? "true" : "false"; break;
        case Type::Number: {
            if (number_ == std::floor(number_) && std::fabs(number_) < 1e15) {
                out += std::to_string(static_cast<long long>(number_));
            } else {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%g", number_);
                out += buf;
            }
            break;
        }
        case Type::String: EscapeTo(out, string_); break;
        case Type::Array: {
            if (array_.empty()) { out += "[]"; break; }
            out += '[';
            for (size_t i = 0; i < array_.size(); ++i) {
                if (i) out += ',';
                NewlineIndent(out, indent, depth + 1);
                array_[i].DumpTo(out, indent, depth + 1);
            }
            NewlineIndent(out, indent, depth);
            out += ']';
            break;
        }
        case Type::Object: {
            if (object_.empty()) { out += "{}"; break; }
            out += '{';
            for (size_t i = 0; i < object_.size(); ++i) {
                if (i) out += ',';
                NewlineIndent(out, indent, depth + 1);
                EscapeTo(out, object_[i].first);
                out += ':';
                if (indent > 0) out += ' ';
                object_[i].second.DumpTo(out, indent, depth + 1);
            }
            NewlineIndent(out, indent, depth);
            out += '}';
            break;
        }
    }
}

std::string Value::Dump(int indent) const {
    std::string out;
    DumpTo(out, indent, 0);
    return out;
}

bool Parse(std::string_view text, Value& out, std::string& error) {
    Parser p(text);
    return p.Run(out, error);
}

}  // namespace wintangle::json
