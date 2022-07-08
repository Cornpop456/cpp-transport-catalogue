#include "json_builder.h"

using namespace std;

namespace json {
    
DictItemContext Builder::StartDict() {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if (path_.size() > 0 
        && path_.back()->IsDict()
        && !has_key) {
        
        throw logic_error("no key"s);
    }
   
    if (path_.size() == 0) {
        root_ = Dict{};
        path_.push_back(&root_);
    } else if (path_.back()->IsArray()) {
        Array& arr = const_cast<Array&>(path_.back()->AsArray());
        arr.push_back(Dict{});
        path_.push_back(&arr.back());
    } else if (path_.back()->IsDict()) {
        Dict& dict = const_cast<Dict&>(path_.back()->AsDict());
        dict[last_key_] = Dict{};
        path_.push_back(&dict[last_key_]);
        has_key = false;
    }
    
    ++opened_dicts_;
    
    return DictItemContext(this);
}

ArrayItemContext Builder::StartArray() {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if (path_.size() > 0 
        && path_.back()->IsDict()
        && !has_key) {
        
        throw logic_error("no key"s);
    }
    
    if (path_.size() == 0) {
        root_ = Array{};
        path_.push_back(&root_);
    } else if (path_.back()->IsArray()) {
        Array& arr = const_cast<Array&>(path_.back()->AsArray());
        arr.push_back(Array{});
        path_.push_back(&arr.back());
    } else if (path_.back()->IsDict()) {
        Dict& dict = const_cast<Dict&>(path_.back()->AsDict());
        dict[last_key_] = Array{};
        path_.push_back(&dict[last_key_]);
        has_key = false;
    }
    
    ++opened_arrays_;
    
    return ArrayItemContext(this);
}

Builder& Builder::EndDict() {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if ((path_.size() > 0 && !path_.back()->IsDict())
        || (path_.size() == 0)) {
        
        throw logic_error("no opened dict"s);
    }
    
    path_.pop_back();
    --opened_dicts_;
    if (opened_arrays_ == 0 && opened_dicts_ == 0) {
        ready_ = true;
    }
    return *this;
}
    
Builder& Builder::EndArray() {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if ((path_.size() > 0 && !path_.back()->IsArray())
        || (path_.size() == 0)) {
        
        throw logic_error("no opened array"s);
    }
    
    path_.pop_back();
    --opened_arrays_;
    if (opened_arrays_ == 0 && opened_dicts_ == 0) {
        ready_ = true;
    }
    return *this;
}
    
KeyItemContext Builder::Key(string key) {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if ((path_.size() > 0 && !path_.back()->IsDict())
        || (path_.size() == 0)) {
        
        throw logic_error("no dict"s);
    }
    
    if (has_key) {
        throw logic_error("already has a key with no value"s);
    }
    
    has_key = true;
    last_key_ = move(key);
    
    return KeyItemContext(this);
}

Builder& Builder::Value(JsonValue val) {
    if (ready_) {
        throw logic_error("ready object"s);
    }
    
    if (!has_key
        && (path_.size() > 0 && path_.back()->IsDict())) {
        throw logic_error("no key in dict"s);
    }
    
    Node value;
    
    if (holds_alternative<nullptr_t>(val)) {
        value = move(get<nullptr_t>(val));
    } else if (holds_alternative<Array>(val)) {
        value = get<Array>(val);
    } else if (holds_alternative<Dict>(val)) {
        value = move(get<Dict>(val));
    } else if (holds_alternative<bool>(val)) {
        value = get<bool>(val);
    } else if (holds_alternative<int>(val)) {
        value = move(get<int>(val));
    } else if (holds_alternative<double>(val)) {
        value = move(get<double>(val));
    } else if (holds_alternative<string>(val)) {
        value = move(get<string>(val));
    }
    
    if (path_.size() == 0) {
        root_ = move(value);
        ready_ = true;
    } else if (path_.back()->IsDict()) {
        Dict& dict = const_cast<Dict&>(path_.back()->AsDict());
        dict[last_key_] = move(value);
        has_key = false;
    } else {
        Array& arr = const_cast<Array&>(path_.back()->AsArray());
        arr.push_back(move(value));
    }
    
    return *this;
}
    
const Node& Builder::Build() {
    if (opened_arrays_ > 0 || opened_dicts_ > 0 || !ready_) {
        throw logic_error("not ready object"s);
    }
    
    return root_;
}
    
KeyItemContext ItemContext::Key(string key) {
    return builder_->Key(std::move(key));
}

DictItemContext ItemContext::StartDict() {
    return builder_->StartDict();
}

Builder& ItemContext::EndDict() {
    return builder_->EndDict();
}

ArrayItemContext ItemContext::StartArray() {
    return builder_->StartArray();
}

Builder& ItemContext::EndArray() {
    return builder_->EndArray();
}

DictItemContext KeyItemContext::Value(JsonValue val) {
    builder_->Value(std::move(val));
    return DictItemContext{builder_};
 
}
    
ArrayItemContext ArrayItemContext::Value(JsonValue val) {
    builder_->Value(std::move(val));
    return ArrayItemContext{builder_};
}
    
} // json