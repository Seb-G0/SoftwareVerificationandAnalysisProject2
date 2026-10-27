#include "parser.h"
#include "peglib.h"

#include <stdexcept>

namespace {

Rational number(const peg::Ast& node) {
    if (node.name == "Constant") {
        Rational value = number(*node.nodes[0]);
        if (node.choice == 1) value = -value;
        return value;
    }

    Rational numerator = 0, denominator = 1;
    size_t i = node.token[0] == '-' ? 1 : 0;
    for (; i < node.token.size() && node.token[i] != '/'; ++i)
        numerator = numerator * 10 + (node.token[i] - '0');
    if (i < node.token.size()) {
        denominator = 0;
        for (++i; i < node.token.size(); ++i)
            denominator = denominator * 10 + (node.token[i] - '0');
    }
    if (denominator == 0) throw std::invalid_argument("zero denominator");
    Rational result = numerator / denominator;
    if (node.token[0] == '-') result = -result;
    return result;
}

void collect(const peg::Ast& node, std::vector<Constraint>& result, Rational factor = 1) {
    if (node.name == "Atom") {
        result.push_back({{}, node.nodes[0]->token_to_string(), Rational(0)});
        collect(*node.nodes[1], result, 1);
        collect(*node.nodes[2], result, -1);
    } else if (node.name == "Number") {
        result.back().rhs -= factor * number(node);
    } else if (node.name == "Variable") {
        result.back().coefficients[node.token_to_string()] += factor;
    } else if (node.name == "Multiply") {
        collect(*node.nodes[1], result, factor * number(*node.nodes[0]));
    } else {
        for (size_t i = 0; i < node.nodes.size(); ++i) {
            Rational weight = factor;
            if (node.name == "Subtract" && (node.nodes.size() == 1 || i > 0))
                weight = -weight;
            collect(*node.nodes[i], result, weight);
        }
    }
}

}

std::vector<Constraint> parse_constraints(const std::string& text) {
    peg::parser parser;
    std::string error;
    parser.set_logger([&](size_t line, size_t column, const std::string& message) {
        error = std::to_string(line) + ':' + std::to_string(column) + ": " + message;
    });
    if (!parser.load_grammar(R"(
        Script <- Command+
        Command <- '(' 'assert' Formula ')'
        Formula <- Conjunction / Atom
        Conjunction <- '(' 'and' Atom+ ')'

        Atom  <- '(' Comparison Term Term ')'
        Comparison  <- < '>=' / '<=' / '>' / '<' / '=' >
        Term        <- Number / Variable / Add / Subtract / Multiply
        Add         <- '(' '+' Term+ ')'
        Subtract    <- '(' '-' Term+ ')'
        Multiply    <- '(' '*' Constant Term ')'
        Constant    <- Number / '(' '-' Number ')'
        Variable    <- < [a-zA-Z][a-zA-Z0-9]* ![^() \t\r\n\f\v] >
        Number      <- < '-'? [0-9]+ ('/' [0-9]+)? ![^() \t\r\n\f\v] >
        %whitespace <- [ \t\r\n\f\v]*
        %word       <- [a-zA-Z][a-zA-Z0-9]*

    )")) {
        throw std::runtime_error("invalid grammar: " + error);
    }

    parser.enable_ast();
    std::shared_ptr<peg::Ast> tree;
    if (!parser.parse(text, tree)) throw std::invalid_argument(error);
    std::vector<Constraint> result;
    collect(*tree, result);
    return result;
}
