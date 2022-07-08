#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

// У меня в моей реализации либы json методы не были встроенны, 
// решил оставить версию от авторов в тренажере, которая была предложена 
// в заданиях json builder, вернул обратно свою + перенёс реализацию операторов сравнения

namespace json {
    
class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using JsonValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : JsonValue {
public: 
    using JsonValue::JsonValue;
     
    const Array& AsArray() const;
    const Dict& AsDict() const;
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
    bool IsDict() const;
    
    bool operator==(const Node& rhs ) const;
    bool operator!=(const Node& rhs ) const;
};
    
class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator==(const Document& rhs ) const;
    bool operator!=(const Document& rhs ) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output, 
    int indent_size = 2, int indent_step = 1);

}  // namespace json