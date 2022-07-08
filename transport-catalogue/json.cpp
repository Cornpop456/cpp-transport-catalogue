#include <sstream>
#include <iomanip>

#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);
    
Node LoadArray(istream& input) {
    Array result;
    char c;

    while (input >> c) {
        if ( c == ']') {
            break;
        }
        
        if (c != ',') {
            input.putback(c);
        }
        
        result.push_back(LoadNode(input));
    }
    
    if (c != ']') {
        throw ParsingError{"no closing bracket in array"s};
    }
    
    return Node(move(result));
}

string LoadLexeme(istream& input) {
    stringstream dst;
    char c;
    
    while (input.peek() != '}' 
        && input.peek() != ']' 
        && input.peek() != ' '
        && input.peek() != ','
        && input.peek() != '\r'
        && input.peek() != '\n'
        && input.peek() != '\t'
        && input.peek() != EOF) {
        
        c = input.get();
        dst << c;
    }
    
    return dst.str();
}
    
Node LexemeToNode(const string& lex) {
    if (lex == "null"s) {
        return Node{nullptr};
    }
    
    if (lex == "true"s) {
        return Node{true};
    }
    
    if (lex == "false"s) {
        return  Node{false};
    }
    
    bool is_double = false;
    double double_res;
    
    if (lex.find('.') != lex.npos 
        || lex.find('E') != lex.npos 
        || lex.find('e') != lex.npos) {
        
        try {
            double_res = stod(lex);
            is_double = true;
        } catch (...) {
            throw ParsingError{"undefined lexeme"s};
        }
    }
    
    if (is_double) {
        return Node{double_res};
    }
    
    int int_res;
    
    try {
        int_res = stoi(lex);
    } catch (...) {
        throw ParsingError{"undefined lexeme"s};
    }
    
    return Node{int_res};
}

Node LoadString(istream& input) {
    
    stringstream dst;
    bool escape = false;
    char c;
    
    while (input.peek() != '"' || escape) {
        c = input.get();
        
        if (input.eof()) {
            throw ParsingError{"no ending quote in string"s};
        }
        
        if (c == '\\' && !escape) {
            escape = true;
            continue;
        }
        
        if (escape && c == 'n') {
            dst << '\n';
        } else if (escape && c == 'r') {
            dst << '\r';
        } else if (escape && c == 't') {
            dst << '\t';
        } else if (escape && c == '"') {
            dst << '"';
        } else if (escape && c == '\\') {
            dst << '\\';
        } else {
            if (escape) {
               throw ParsingError{"wrong escape sequence"s};
            }
            
            dst <<  c;
        }
        
        escape = false;
    }
    
    input.get(); //remove closing quote 
    
    return Node(move(dst.str()));
}

Node LoadDict(istream& input) {
    Dict result;
    char c;

    while (input >> c) {
        if (c == '}') {
            break;
        }
        
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    
    if (c !=  '}') {
        throw ParsingError{"no closing bracket in dict"s};
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {
        input.putback(c);
        return LexemeToNode(LoadLexeme(input));
    }
}
    
std::string escape(std::string_view src) {
    std::stringstream dst;
    for (char ch : src) {
        switch (ch) {
            case '\n': dst << '\\' << 'n'; break;
            case '\r': dst << '\\' << 'r'; break;
            case '"': dst << '\\' << '"'; break;
            case '\\': dst << '\\' << '\\'; break;
            default: dst << ch; break;
        }
    }
    return dst.str();
}
    
void PrintArray(const Array& arr, std::ostream& output, 
    int indent_size, int indent_step) {
    
    output << "[\n"s;
        
    size_t index = 0;
        
    for (const Node& node : arr) {
        for (int i = 0; i < indent_size * indent_step; ++i) {
            output << ' '; 
        }

        Print(Document{node},  
              output, 
              indent_size, 
              indent_step + 1);

        if (index < arr.size() - 1) {
            output << ',';
        }

        output << '\n';
        ++index;
    }
        
    for (int i = 0; i < indent_size * (indent_step - 1); ++i) {
        output << ' '; 
    }
    
    output << ']';
}
    
void PrintDict(const Dict& dict, std::ostream& output, 
    int indent_size, int indent_step) {
    
    output << "{\n"s;
        
    size_t index = 0;
        
    for (const auto& [key, node] : dict) {
        for (int i = 0; i < indent_size * indent_step; ++i) {
            output << ' '; 
        }
        
        output << '"' << key << '"' << ": ";

        Print(Document{node},  
              output, 
              indent_size, 
              indent_step + 1);

        if (index < dict.size() - 1) {
            output << ',';
        }

        output << '\n';
        ++index;
    }
        
    for (int i = 0; i < indent_size * (indent_step - 1); ++i) {
        output << ' '; 
    }
    
    output << '}';
}

}  // namespace

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(*this);
}    
    
bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}
    
bool Node::IsDouble() const {
    return holds_alternative<double>(*this) || 
           holds_alternative<int>(*this);
}
    
bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}
    
bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}
    
bool Node::IsDict() const {
    return holds_alternative<Dict>(*this);
}
    
bool Node::IsString() const {
    return holds_alternative<string>(*this);
}
    
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw  logic_error("not an array"s);
    }
    return  get<Array>(*this);
}

const Dict& Node::AsDict() const {
     if (!IsDict()) {
        throw  logic_error("not a dict"s);
    }
    return get<Dict>(*this);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw  logic_error("not an int"s);
    }
    
    return get<int>(*this);
}
    
bool Node::AsBool() const {
     if (!IsBool()) {
        throw  logic_error("not a bool"s);
    }
    
    return get<bool>(*this);
}
    
double Node::AsDouble() const {
    if (!IsInt() && !IsDouble()) {
        throw  logic_error("not a double"s);
    }
    
    if (IsInt()) {
        return double(get<int>(*this));
    }
    
    return get<double>(*this);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw  logic_error("not a string"s);
    }
    return get<string>(*this);
}

bool Node::operator==(const Node& rhs ) const {
    return static_cast<const JsonValue&>(*this) 
        == static_cast<const JsonValue&>(rhs);
}

bool Node::operator!=(const Node& rhs ) const {
    return !(*this == rhs);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool Document::operator==(const Document& rhs ) const {
    return root_ == rhs.root_;
}

bool Document::operator!=(const Document& rhs ) const {
    return !(*this == rhs);
}
        
void Print(const Document& doc, std::ostream& output, 
    int indent_size, int indent_step) {

    const Node& root = doc.GetRoot();
    
    if (root.IsNull()) {
        output << "null"s;
    } else if (root.IsString()) {
        output << '"' << escape(root.AsString()) << '"';
    } else if (root.IsInt()) {
        output << root.AsInt();
    } else if (root.IsDouble()) {
        output << root.AsDouble();
    } else if (root.IsBool()) {
        output << boolalpha << root.AsBool();
    } else if (root.IsArray()) {
        const Array& arr = root.AsArray();
        PrintArray(arr, output, indent_size, indent_step);
    } else if (root.IsDict()) {
        const Dict& dict = root.AsDict();
        PrintDict(dict, output, indent_size, indent_step);
    }
}

}  // namespace json