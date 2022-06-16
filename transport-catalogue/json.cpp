#include <sstream>
#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(Node::Value{move(s)});
}

Node LoadNull(istream& input) {
    std::string s;
    char c;
    for(int i = 0; i < 4; ++i) {
        input >> c;
        s.push_back(c);
        c = 0;
    }
    if(s != "null"sv) {
        throw ParsingError("String parsing error");
    }
    return Node();
}

Node LoadBool(istream& input) {
    std::string s;
    char c;
    input >> c;
    s.push_back(c);
    bool b = c == 't' ? true : false;

    for(int i = 0; i < 3; ++i) {
        input >> c;
        s.push_back(c);
    }
    if(!b) {
        input >> c;
        s.push_back(c);
    }

    if(s != "true"sv && s != "false"sv) {
        throw ParsingError("String parsing error");
    }

    return Node(b);
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(Node::Value{std::stoi(parsed_num)});
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(Node::Value{std::stod(parsed_num)});
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadArray(istream& input) {
    Array result;
    char c = 0;
    while(input >> c && c != ']') {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if(c != ']')
        throw ParsingError("Array parsing error: no ]");

    return Node(Node::Value{move(result)});
}


Node LoadDict(istream& input) {
    Dict result;
    char c = 0;
    while(input >> c && c != '}') {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if(c != '}')
        throw ParsingError("Dict parsing error: no }");

    return Node(Node::Value{move(result)});
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
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

Node::Node(Value value)
    : value_(move(value)) {
}

Node::Node(const Node& node)
    : value_(node.value_) {
}

Node::Node(const Document& doc)
    : Node(doc.GetRoot()) {
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
bool Node::IsDouble() const {
    return std::holds_alternative<double>(value_) ||
            std::holds_alternative<int>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

const Array& Node::AsArray() const { 
    if(IsArray()) {
        return std::get<Array>(value_);
    }
    throw std::logic_error("Node::AsArray() wrong access"s);
}

const Dict& Node::AsMap() const {
    if(IsMap()) {
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Node::AsMap() wrong access"s);
}

int Node::AsInt() const {
    if(IsInt()) {
        return std::get<int>(value_);
    }
    throw std::logic_error("Node::AsInt() wrong access"s);
}

double Node::AsDouble() const {
    if(IsPureDouble()) {
        return std::get<double>(value_);
    } else
    if(IsInt()) {
        return std::get<int>(value_);
    }
    throw std::logic_error("Node::AsDouble() wrong access"s);
}

const string& Node::AsString() const {
    if(IsString()) {
        return std::get<string>(value_);
    }
    throw std::logic_error("Node::AsString() wrong access"s);
}

bool Node::AsBool() const {
    if(IsBool()) {
        return std::get<bool>(value_);
    }
    throw std::logic_error("Node::AsBool() wrong access"s);
}

const Node::Value& Node::GetValue() const {
    return value_;
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs.GetValue() == rhs.GetValue());
}

//----- Document -----

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

//----- Print -----

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(bool b, const PrintContext& ctx) {
    ctx.out << (b ? "true"sv : "false"sv);
}

std::string EscapeStr(std::string is) {
    std::istringstream input{is};
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if(it == end) break;
        const char ch = *(it);
        // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
        switch (ch) {
            case '\n':
                s.push_back('\\');
                s.push_back('n');
                break;
            case '\t':
                s.push_back('\t');
                break;
            case '\r':
                s.push_back('\\');
                s.push_back('r');
                break;
            case '\"':
                s.push_back('\\');
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                s.push_back('\\');
                break;
            default:
                s.push_back(ch);
        }
        ++it;
    }
    return s;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::string s, const PrintContext& ctx) {
    ctx.out << "\""sv << EscapeStr(s) << "\""sv;
}

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(Array arr, const PrintContext& ctx) {
    ctx.out << "\n"sv;
    ctx.PrintIndent();
    ctx.out << "["sv;
    bool first = true;
    for(auto& node : arr) {
        if(first) first = false;
        else ctx.out << ", "sv;
        PrintNode(node, ctx.Indented());
    }
    ctx.out << "]"sv;
}

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(Dict dic, const PrintContext& ctx) {
    ctx.out << "\n"sv;
    ctx.PrintIndent();
    ctx.out << "{"sv;
    bool first = true;
    for(auto& node : dic) {
        if(first) first = false;
        else ctx.out << ", "sv;
        ctx.out << "\""sv << node.first << "\": "sv;
        PrintNode(node.second, ctx.Indented());
    }
    ctx.out << "}"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, const PrintContext& ctx) {
    auto node = doc.GetRoot();
    PrintNode(node, ctx);
}

void Print(const Document& doc, std::stringstream& ss) {
    Print(doc, PrintContext{ss});
}

void Print(const Document& doc, std::ostream& os) {
    std::stringstream strm;
    Print(doc, PrintContext{strm});
    os << strm.str();
}

}  // namespace json
