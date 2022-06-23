#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    
class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using JsonValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : JsonValue {
public: 
    using JsonValue::JsonValue;
     
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const std::string& AsString() const;
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    bool operator==(Node const& rhs ) const {
        return static_cast<const JsonValue&>(*this) 
            == static_cast<const JsonValue&>(rhs);
    }
    
    bool operator!=(Node const & rhs ) const {
        return !(*this == rhs);
    }
};
    
class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator==(Document const & rhs ) const {
        return root_ == rhs.root_;
    }
    
    bool operator!=(Document const & rhs ) const {
        return !(*this == rhs);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output, 
    int indent_size = 2, int indent_step = 1);

}  // namespace json