#include "json_builder.h"

using namespace std::literals;

namespace json {

Node Builder::Build() {
    if(state_.size() > 0 && state_.back() != State::FINAL) {
        throw std::logic_error{"Build()"};
    }

    return root_;
}

Node Builder::GetNode(Node::Value val) {
    switch (val.index()) {
    case 1:
        return Node{std::get<Array>(val)}; break;
    case 2:
        return Node{std::get<Dict>(val)}; break;
    case 3:
        return Node{std::get<bool>(val)}; break;
    case 4:
        return Node{std::get<int>(val)}; break;
    case 5:
        return Node{std::get<double>(val)}; break;
    case 6:
        return Node{std::get<std::string>(val)}; break;
    }
    return Node{};
}

Builder& Builder::Value(Node::Value val) {
    Node n{GetNode(val)};

    switch (state_.back()) {
    case State::START: {
        state_.pop_back();
        state_.push_back(State::FINAL);
        root_ = move(n);
        break;
    }
    case State::ARRAY: {
        auto& a = nodes_stack_.back()->AsArray();
        a.push_back(move(n));
        break;
    }
    case State::KEY: {
        state_.pop_back();
        nodes_stack_.back()->AsDict()[key_] = move(n);
        break;
    }
    default:
        throw std::logic_error{"Value()"s};
    }

    return *this;
}

ArrayContext Builder::StartArray(){
    Node na{Array{}};

    switch (state_.back()) {
    case State::START: {
        state_.pop_back();
        root_ = move(na);
        nodes_stack_.push_back(&root_);
        break;
    }
    case State::ARRAY: {
        auto& a = nodes_stack_.back()->AsArray();
        a.push_back(move(na));
        nodes_stack_.push_back(&a.back());
        break;
    }
    case State::KEY: {
        state_.pop_back();        
        nodes_stack_.back()->AsDict()[key_] = move(na);
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key_));
        break;
    }
    default:
        throw std::logic_error{"StartArray()"s};
    }

    state_.push_back(State::ARRAY);

    return ArrayContext{*this};
}

Builder& Builder::EndArray() {

    switch (state_.back()) {
    case State::ARRAY: {
        state_.pop_back();
        nodes_stack_.pop_back();
        break;
    }
    default:
        throw std::logic_error{"EndArray()"s};
    }

    return *this;

}

KeyContext Builder::Key(Node::Value val) {

    switch (state_.back()) {
    case State::DICT: {
        key_ = std::get<std::string>(val);
        state_.push_back(State::KEY);
        break;
    }
    default:
        throw std::logic_error{"Key()"s};
    }

    return KeyContext{*this};

}

DictContext Builder::StartDict(){

    Node nd{Dict{}};

    switch (state_.back()) {
    case State::START: {
        state_.pop_back();
        root_ = move(nd);
        nodes_stack_.push_back(&root_);
        break;
    }
    case State::ARRAY: {
        auto& a = nodes_stack_.back()->AsArray();
        a.push_back(move(nd));
        nodes_stack_.push_back(&a.back());
        break;
    }
    case State::KEY: {
        state_.pop_back();
        nodes_stack_.back()->AsDict()[key_] = move(nd);
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(key_));
        break;
    }
    default:
        throw std::logic_error{"StartDict()"s};
    }

    state_.push_back(State::DICT);

    return DictContext(*this);

}

Builder& Builder::EndDict() {

    switch (state_.back()) {
    case State::DICT: {
        state_.pop_back();
        nodes_stack_.pop_back();
        break;
    }
    default:
        throw std::logic_error{"EndDict()"s};
    }

    return *this;

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
