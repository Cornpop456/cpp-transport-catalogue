#pragma once

#include "json.h"

namespace json {
    
class ItemContext;
class KeyItemContext;
class KeyValueItemContext;
class DictItemContext;
class ArrayItemContext;
    
class Builder {
        
private:
    Node root_;
    std::vector<Node*> path_;
    int opened_arrays_ = 0;
    int opened_dicts_ = 0;
    std::string last_key_;
    bool has_key = false;
    bool ready_ = false;
public:
    DictItemContext StartDict();
    ArrayItemContext  StartArray();
    KeyItemContext  Key(std::string);

    Builder& Value(JsonValue);
    Builder& EndDict();
    Builder& EndArray();

    const Node& Build();
};
    
class ItemContext {
public:
    ItemContext(Builder* builder) : builder_{builder} {}
protected:
    KeyItemContext Key(std::string key);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

    Builder* builder_;
};
    
class DictItemContext : public ItemContext {
public:
    using ItemContext::ItemContext;
    using ItemContext::Key;
    using ItemContext::EndDict;
};

class ArrayItemContext : public ItemContext {
public:
    using ItemContext::ItemContext;
    ArrayItemContext Value(JsonValue);
    using ItemContext::StartDict;
    using ItemContext::StartArray;
    using ItemContext::EndArray;
};
    
class KeyItemContext : public ItemContext {
public:
    using ItemContext::ItemContext;
    DictItemContext Value(JsonValue);
    using ItemContext::StartDict;
    using ItemContext::StartArray;
};
    
} // json