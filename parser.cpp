#include <iostream>
#include <iterator>
#include <string_view>
#include <cctype>
#include "parser.hpp"
#include "syntax_tree.hpp"

std::optional<char> get_token(std::string_view& input);
bool empty(std::string_view& input);
bool is_unary(char c);
std::optional<unary_op> unary(std::string_view& input);
std::optional<binary_op> binary(std::string_view& input);
void consume_whitespace(std::string_view& input);
bool consume_token(std::string_view& input, char c);
void consume(std::string_view& input);
int parse_int(std::string_view& input);
std::optional<expr> combine_unary(std::optional<unary_op> op, std::optional<expr>&& exp); 
std::optional<expr> combine_binary(std::optional<binary_op> op, std::optional<expr>&& left, std::optional<expr>&& right); 

std::optional<char> get_token(std::string_view& input) {
	if (empty(input)) {
		return std::optional<char>();
	}
	return std::make_optional<char>(input.front());
}

bool empty(std::string_view& input) {
	consume_whitespace(input);
	return input.empty();
}

bool is_unary(char c) {
	return c == '-';
}

std::optional<unary_op> unary(std::string_view& input) {
	auto c = get_token(input);
	if (!c) {
		return std::optional<unary_op>();
	}

	switch (*c) {
		case '-':
			consume(input);
			return std::optional<unary_op>(unary_op::neg);
		default:
			return std::optional<unary_op>();
	}
}

std::optional<binary_op> binary(std::string_view& input) {
	auto c = get_token(input);
	if (!c) {
		return std::optional<binary_op>();
	}

	switch (*c) {
		case '+':
			consume(input);
			return std::optional<binary_op>(binary_op::add);
		case '-':
			consume(input);
			return std::optional<binary_op>(binary_op::sub);
		case '*':
			consume(input);
			return std::optional<binary_op>(binary_op::mul);
		case '/':
			consume(input);
			return std::optional<binary_op>(binary_op::div);
		case '%':
			consume(input);
			return std::optional<binary_op>(binary_op::mod);
		default:
			return std::optional<binary_op>();
	}
}

void consume_whitespace(std::string_view& input) {
	int i = 0;
	while (i < input.length() && std::isspace(input[i])) {
		++i;
	}
	input = input.substr(i);
}

bool consume_token(std::string_view& input, char c) {
	if (empty(input)) {
		return false;
	}
	bool ret = input.front() == c;
	consume(input);
	return ret;
}

void consume(std::string_view& input) {
	if (empty(input)) {
		return;
	}
	input = input.substr(1);
}

int parse_int(std::string_view& input) {
	consume_whitespace(input);

	int result = 0;
	int i = 0;
	while (i < input.length() && std::isdigit(input[i])) {
		result = result * 10 + (input[i] - '0');
		++i;
	}
	input = input.substr(i);
	return result;
}

std::optional<expr> combine_unary(std::optional<unary_op> op, std::optional<expr>&& exp) {
	if (op && exp) {
		return std::make_optional<expr>(
			unary_syn(
				*op,
				*std::move(exp)
			)
		);
	}

	return std::optional<expr>();
}

std::optional<expr> combine_binary(std::optional<binary_op> op, std::optional<expr>&& left, std::optional<expr>&& right) {
	if (left && !op && !right) {
		return std::move(left);
	}

	if (!left && !op && right) {
		return std::move(right);
	}

	if (left && op && right) {
		return std::make_optional<expr>(
			binary_syn(
				*op,
				*std::move(left),
				*std::move(right)	
			)
		);
	}

	return std::optional<expr>();
}

std::optional<expr> parse(std::string_view& input, std::optional<expr>&& curr, std::optional<binary_op> op) {
	if (empty(input)) {
		return combine_binary(op, std::move(curr), std::optional<expr>());
	}

	char token = *get_token(input);
	if (token == '(') {
		consume(input);
		auto exp = parse(input, std::optional<expr>(), std::optional<binary_op>());
		if (!consume_token(input, ')')) {
			return std::optional<expr>();
		}

		auto combined = combine_binary(op, std::move(curr), std::move(exp));
		op = binary(input);
		if (!op) {
			return combined;
		}
		return parse(input, std::move(combined), op);
	}

	else if (is_unary(token)) {
		auto un = unary(input);
		auto exp = parse(input, std::optional<expr>(), std::optional<binary_op>());

		auto combined = combine_binary(op, std::move(curr), combine_unary(un, std::move(exp))); 
		op = binary(input);
		if (!op) {
			return combined;
		}
		return parse(input, std::move(combined), op);
	}

	else if (std::isdigit(token)) {
		auto exp = std::make_optional<expr>(
			std::in_place_index<2>, 
			parse_int(input)
		);

		auto combined = combine_binary(op, std::move(curr), std::move(exp)); 
		op = binary(input);
		if (!op) {
			return combined;
		}
		return parse(input, std::move(combined), op);
	}

	return std::optional<expr>();
}

std::optional<int> eval(expr const& exp) {
	if (std::holds_alternative<binary_syn>(exp)) {
		auto const& e = std::get<binary_syn>(exp);
		auto left = eval(*e.left);
		auto right = eval(*e.right);

		if (!left || !right) {
			return std::optional<int>();
		}

		switch (e.op) {
			case binary_op::add:
				return std::make_optional<int>(*left + *right);
			case binary_op::sub:
				return std::make_optional<int>(*left - *right);
			case binary_op::mul:
				return std::make_optional<int>((*left) * (*right));
			case binary_op::div:
				if (*right == 0) {
					return std::optional<int>();
				} else {
					return std::make_optional<int>(*left / *right);
				}
			case binary_op::mod:
				if (*right == 0) {
					return std::optional<int>();
				} else {
					return std::make_optional<int>(*left % *right);
				}
			default:
				return std::optional<int>();
		}
	} else if (std::holds_alternative<unary_syn>(exp)) {
		auto const& e = std::get<unary_syn>(exp);
		auto val = eval(*e.exp);
		if (!val) {
			return std::optional<int>();
		}
		
		switch (e.op) {
		case unary_op::neg:
			return std::make_optional<int>(-(*val));
		default:
			return std::optional<int>();
		}
	}

	return std::optional<int>(std::get<int>(exp));
}

