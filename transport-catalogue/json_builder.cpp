#include "json_builder.h"

using namespace std::literals;

namespace json {

// "enum" VS "enum class" for auto conversion to INT
enum ValueType {
    VALUE,
    ARRAY,
    DICT,
    BOOL,
    INT,
    DOUBLE,
    STRING
};

Node Builder::GetNode(Node::Value val) {
    using namespace std;

    switch (val.index()) {
    case ValueType::ARRAY:
        return Node{get<Array>(val)};
    case ValueType::DICT:
        return Node{get<Dict>(val)};
    case ValueType::BOOL:
        return Node{get<bool>(val)};
    case ValueType::INT:
        return Node{get<int>(val)};
    case ValueType::DOUBLE:
        return Node{get<double>(val)};
    case ValueType::STRING:
        return Node{get<std::string>(val)};
    }
    return Node{};
}

State Builder::GetState() {
    if(root_ == nullptr) {
        return State::START;
    } else
    if(nodes_stack_.empty()){
        return State::FINAL;
    } else
    if(nodes_stack_.back()->IsArray()) {
        return State::ARRAY;
    } else
    if(nodes_stack_.back()->IsDict()) {
        return State::DICT;
    }

    return State::KEY;
}

Node Builder::Build() {

    if(State::FINAL != GetState()) {
        throw std::logic_error{"json::Builder::Build()"};
    }

    return root_;
}

Builder& Builder::EndArray() {

    if(State::ARRAY != GetState()) {
        throw std::logic_error{"json::Builder::EndArray()"};
    }

    nodes_stack_.pop_back();

    return *this;
}

Builder& Builder::EndDict() {

    if(State::DICT != GetState()) {
        throw std::logic_error{"json::Builder::EndDict()"};
    }

    nodes_stack_.pop_back();

    return *this;
}

KeyContext Builder::Key(Node::Value val) {

    if(State::DICT != GetState()) {
        throw std::logic_error{"json::Builder::Key()"};
    }

    auto key = std::get<std::string>(val);
    nodes_stack_.back()->AsDict()[key] = Node{key};
    nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key));

    return KeyContext{*this};
}

void Builder::AddNode(Node node, int type) {

    switch (GetState()) {
    case State::START: {
        root_ = move(node);
        if(type != ValueType::VALUE) {
            nodes_stack_.push_back(&root_);
        }
        break;
    }
    case State::ARRAY: {
        nodes_stack_.back()->AsArray().push_back(move(node));
        if(type != ValueType::VALUE) {
            nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
        }
        break;
    }
    case State::KEY: {
        auto key = nodes_stack_.back()->AsString();
        nodes_stack_.pop_back();
        nodes_stack_.back()->AsDict().at(key) = move(node);
        if(type != ValueType::VALUE) {
            nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key));
        }
        break;
    }
    default:
        std::string what = "json::Builder::"s + (
                type == ValueType::ARRAY ? "StartArray()"s :
                type == ValueType::DICT ?  "StartDict()"s :
                                 "Value()"s);
        throw std::logic_error{what};
    }

    return;
}

Builder& Builder::Value(Node::Value val) {

    AddNode(Node{GetNode(val)}, ValueType::VALUE);

    return *this;
}

ArrayContext Builder::StartArray(){

    AddNode(Node{Array{}}, ValueType::ARRAY);

    return ArrayContext{*this};
}

DictContext Builder::StartDict(){

    AddNode(Node{Dict{}}, ValueType::DICT);

    return DictContext(*this);
}

// class Context

Builder& Context::Value(Node::Value v) {
    builder_.Value(v);
    return this->builder_;
}

ArrayContext Context::StartArray() {
    builder_.StartArray();
    return ArrayContext(this->builder_);
}

Builder& Context::EndArray() {
    builder_.EndArray();
    return this->builder_;
}

KeyContext Context::Key(Node::Value v) {
    builder_.Key(v);
    return KeyContext{this->builder_};
}

DictContext Context::StartDict() {
    builder_.StartDict();
    return DictContext{this->builder_};
}

Builder& Context::EndDict() {
    builder_.EndDict();
    return this->builder_;
}

KeyValueContext KeyContext::Value(Node::Value v) {
    builder_.Value(v);
    return KeyValueContext{this->builder_};
}

ArrayValueContext ArrayContext::Value(Node::Value v) {
    builder_.Value(v);
    return ArrayValueContext{this->builder_};
}

ArrayValueContext ArrayValueContext::Value(Node::Value v) {
    builder_.Value(v);
    return ArrayValueContext{this->builder_};
}

} // namespace json
