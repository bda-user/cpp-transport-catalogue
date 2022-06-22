#pragma once

#include "json.h"
#include <stdexcept>
#include <variant>

namespace json {

enum State {START, ARRAY, DICT, KEY, FINAL};

class DictContext;
class KeyContext;
class KeyValueContext;
class ArrayContext;
class ArrayValueContext;

class Builder {

public:
    Node GetNode(Node::Value);
    Builder& Value(Node::Value);
    Node Build();
    ArrayContext StartArray();
    Builder& EndArray();
    KeyContext Key(Node::Value);
    DictContext StartDict();
    Builder& EndDict();

private:
    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::vector<State> state_{State::START};
    std::string key_;
};

class Context {

public:
    Context(Builder& b) : builder_(b) {}
    Builder& Value(Node::Value v);
    Node Build(){return builder_.Build();};
    ArrayContext StartArray();
    Builder& EndArray();
    KeyContext Key(Node::Value);
    DictContext StartDict();
    Builder& EndDict();

protected:
    Builder& builder_;
};

class DictContext : public Context {

public:
    DictContext(Builder& b) : Context(b) {}
    Builder& Value(Node::Value v) = delete;
    Node Build() = delete;
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictContext StartDict() = delete;
};

class KeyContext : public Context {

public:
    KeyContext(Builder& b) : Context(b) {}
    Node Build() = delete;
    Builder& EndArray() = delete;
    KeyContext Key(Node::Value) = delete;
    Builder& EndDict() = delete;
    KeyValueContext Value(Node::Value v);
};

class ArrayContext : public Context {

public:
    ArrayContext(Builder& b) : Context(b) {}
    Node Build() = delete;
    KeyContext Key(Node::Value) = delete;
    Builder& EndDict() = delete;
    ArrayValueContext Value(Node::Value v);
};

class KeyValueContext : public Context {

public:
    KeyValueContext(Builder& b) : Context(b) {}
    Node Build() = delete;
    Builder& Value(Node::Value v) = delete;
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictContext StartDict() = delete;
};

class ArrayValueContext : public Context {

public:
    ArrayValueContext(Builder& b) : Context(b) {}
    Node Build() = delete;
    Builder& EndDict() = delete;
    KeyContext Key(Node::Value) = delete;
    ArrayValueContext Value(Node::Value v);
};

} // namespace json
