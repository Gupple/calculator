#ifndef PARSER_H
#define PARSER_H

#include <iterator>
#include <optional>
#include <string_view>
#include "syntax_tree.hpp"

std::optional<syntax_tree::expr> parse(std::string_view input);
std::optional<int> eval(syntax_tree::expr const& exp);

#endif

