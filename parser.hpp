#ifndef PARSER_H
#define PARSER_H

#include <iterator>
#include <optional>
#include <string_view>
#include "syntax_tree.hpp"

std::optional<expr> parse(std::string_view& input, std::optional<expr>&& curr, std::optional<binary_op> op);
std::optional<int> eval(expr const& exp);

#endif

