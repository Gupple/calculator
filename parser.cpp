#include <iostream>
#include <iterator>
#include <string_view>
#include "parser.hpp"
#include "syntax_tree.hpp"

std::pair<std::optional<syntax_tree::expr>, std::string_view> parse_expr(std::string_view input,
				       std::optional<syntax_tree::expr> curr,
				       std::optional<syntax_tree::binary_op> op);

std::pair<std::optional<syntax_tree::expr>, std::string_view> parse_subexpr(std::string_view input,
				       std::optional<syntax_tree::expr> curr,
				       std::optional<syntax_tree::binary_op> op);

bool empty(std::string_view input) {
	return input.empty();
}

std::optional<char> get_token(std::string_view input) {
	if (empty(input)) {
		return std::nullopt;
	}
	return input.front();
}

bool is_unary(std::optional<char> c) {
	return c == '-';
}

bool is_binary(std::optional<char> c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

bool is_space(std::optional<char> c) {
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

bool is_digit(std::optional<char> c) {
	if (!c) {
		return false;
	}
	return '0' <= *c && *c <= '9';
}

bool is_parens(std::optional<char> c) {
	return c == '(' || c == ')';
}

bool is_valid(std::optional<char> c) {
	return is_binary(c) || is_unary(c) ||
	is_digit(c) || is_parens(c);
}

std::optional<syntax_tree::unary_op> unary(std::optional<char> c) {
	if (!c) {
		return std::nullopt;
	}

	switch (*c) {
		case '-':
			return syntax_tree::unary_op::neg;
		default:
			return std::nullopt;
	}
}

std::optional<syntax_tree::binary_op> binary(std::optional<char> c) {
	if (!c) {
		return std::nullopt;
	}

	switch (*c) {
		case '+':
			return syntax_tree::binary_op::add;
		case '-':
			return syntax_tree::binary_op::sub;
		case '*':
			return syntax_tree::binary_op::mul;
		case '/':
			return syntax_tree::binary_op::div;
		case '%':
			return syntax_tree::binary_op::mod;
		default:
			return std::nullopt;
	}
}

std::string_view consume(std::string_view input) {
	return input.substr(1);
}

std::string_view consume_whitespace(std::string_view input) {
	while (!empty(input) && is_space(get_token(input))) {
		input = consume(input);
	}
	return input;
}

std::pair<bool, std::string_view> consume_token(std::string_view input, char c) {
	auto token = get_token(input);
	if (token != c) {
		return {false, input};
	}
	return {true, consume(input)};
}

std::pair<int, std::string_view> parse_int(std::string_view input) {
	int result = 0;
	while (!empty(input)) {
		auto token = get_token(input);
		if (!is_digit(token)) {
			break;
		}

		result = result * 10 + (*token - '0');
		input = consume(input);
	}
	return {result, input};
}

std::optional<syntax_tree::expr> combine_unary(std::optional<syntax_tree::unary_op> op,
					       std::optional<syntax_tree::expr> exp) {
	if (op && exp) {
		return syntax_tree::unary_syn{.op = *op, 
			.exp = std::make_unique<syntax_tree::expr>(std::move(*exp))
		};
	}

	return std::nullopt;
}

std::optional<syntax_tree::expr> combine_binary(std::optional<syntax_tree::binary_op> op,
						std::optional<syntax_tree::expr> left,
						std::optional<syntax_tree::expr> right) {
	if (left && !op && !right) {
		return left;
	}

	if (!left && !op && right) {
		return right;
	}

	if (left && op && right) {
		return syntax_tree::binary_syn{
			.op = *op,
			.left = std::make_unique<syntax_tree::expr>(std::move(*left)),
			.right = std::make_unique<syntax_tree::expr>(std::move(*right))
		};
	}

	return std::nullopt;
}

std::pair<std::optional<syntax_tree::expr>, std::string_view> parse_expr(std::string_view input,
				       std::optional<syntax_tree::expr> curr,
				       std::optional<syntax_tree::binary_op> op) {
	auto [exp, view] = parse_subexpr(consume_whitespace(input), std::nullopt, std::nullopt);
	curr = combine_binary(op, std::move(curr), std::move(exp));
	input = consume_whitespace(view);
	
	auto token = get_token(input);
	if (!token) {
		return {std::move(curr), input};
	}

	if (!is_valid(token)) {
		return {std::nullopt, input};
	}

	op = binary(token);
	if (op) {
		input = consume(input);
		return parse_expr(input, std::move(curr), op);
	}

	return {std::move(curr), input};
}

std::pair<std::optional<syntax_tree::expr>, std::string_view> parse_subexpr(std::string_view input,
					       std::optional<syntax_tree::expr> curr,
					       std::optional<syntax_tree::binary_op> op) {
	input = consume_whitespace(input);
	auto token = get_token(input);
	
	if (is_digit(token)) {
		auto [exp, view] = parse_int(input);
		curr = combine_binary(op, std::move(curr), std::move(exp));
		input = consume_whitespace(view);
		return {std::move(curr), input};
	}

	else if (token == '(') {
		input = consume(input);
		auto [exp, view1] = parse_expr(input, std::nullopt, std::nullopt);
		input = consume_whitespace(view1);

		auto [success, view2] = consume_token(input, ')');
		if (!success) {
			return {std::nullopt, view2};
		}

		input = consume_whitespace(view2);
		curr = combine_binary(op, std::move(curr), std::move(exp));
		return {std::move(curr), input};
	}

	else if (is_unary(token)) {
		auto un = unary(token);
		input = consume(input);
		auto [exp, view] = parse_subexpr(input, std::nullopt, std::nullopt);
		curr = combine_binary(op, std::move(curr), combine_unary(un, std::move(exp)));
		input = consume_whitespace(view);
		return {std::move(curr), input};
	}

	else if (!token || is_valid(token)) {
		return {std::move(curr), input};
	}

	return {std::nullopt, input};
}

std::optional<syntax_tree::expr> parse(std::string_view input) {
	auto [result, view_unused] = parse_expr(input, std::nullopt, std::nullopt);
	return result;
}

std::optional<int> eval(syntax_tree::expr const& exp) {
	if (std::holds_alternative<syntax_tree::binary_syn>(exp)) {
		auto const& e = std::get<syntax_tree::binary_syn>(exp);
		auto left = eval(*e.left);
		auto right = eval(*e.right);

		if (!left || !right) {
			return std::optional<int>();
		}

		switch (e.op) {
			case syntax_tree::binary_op::add:
				return std::make_optional<int>(*left + *right);
			case syntax_tree::binary_op::sub:
				return std::make_optional<int>(*left - *right);
			case syntax_tree::binary_op::mul:
				return std::make_optional<int>(*left * *right);
			case syntax_tree::binary_op::div:
				if (*right == 0) {
					return std::optional<int>();
				} else {
					return std::make_optional<int>(*left / *right);
				}
			case syntax_tree::binary_op::mod:
				if (*right == 0) {
					return std::optional<int>();
				} else {
					return std::make_optional<int>(*left % *right);
				}
			default:
				return std::optional<int>();
		}
	} else if (std::holds_alternative<syntax_tree::unary_syn>(exp)) {
		auto const& e = std::get<syntax_tree::unary_syn>(exp);
		auto val = eval(*e.exp);
		if (!val) {
			return std::optional<int>();
		}
		
		switch (e.op) {
		case syntax_tree::unary_op::neg:
			return std::make_optional<int>(-(*val));
		default:
			return std::optional<int>();
		}
	}

	return std::optional<int>(std::get<int>(exp));
}

