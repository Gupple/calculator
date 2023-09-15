#ifndef SYN_TREE_H
#define SYN_TREE_H

#include <variant>
#include <memory>

namespace syntax_tree {
	enum class binary_op{add, sub, mul, div, mod};
	enum class unary_op{neg};

	class binary_syn;
	class unary_syn;

	using expr = std::variant<binary_syn, unary_syn, int>;

	struct binary_syn {
		binary_op op;
		std::unique_ptr<expr> left;
		std::unique_ptr<expr> right;
	};

	struct unary_syn {
		unary_op op;
		std::unique_ptr<expr> exp;
	};
}

#endif

