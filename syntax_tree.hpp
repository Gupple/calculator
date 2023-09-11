#ifndef SYN_TREE_H
#define SYN_TREE_H

#include <variant>
#include <memory>

enum class binary_op{add, sub, mul, div, mod};
enum class unary_op{neg};

class binary_syn;
class unary_syn;

using expr = std::variant<binary_syn, unary_syn, int>;

class binary_syn {
public:
	binary_op op;
	std::unique_ptr<expr> left;
	std::unique_ptr<expr> right;

	binary_syn() = delete;
	binary_syn(binary_syn&&) = default;
	binary_syn(binary_op, expr&&, expr&&);
};

class unary_syn {
public:
	unary_op op;
	std::unique_ptr<expr> exp;

	unary_syn() = delete;
	unary_syn(unary_syn&&) = default;
	unary_syn(unary_op, expr&&);
};

#endif

