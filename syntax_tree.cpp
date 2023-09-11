#include <utility>
#include <memory>
#include "syntax_tree.hpp"

namespace syntax_tree {
	binary_syn::binary_syn(binary_op op, expr&& l, expr&& r) noexcept : op(op) {
		left = std::make_unique<expr>(std::move(l));
		right = std::make_unique<expr>(std::move(r));
	}

	unary_syn::unary_syn(unary_op op, expr&& e) noexcept : op(op) {
		exp = std::make_unique<expr>(std::move(e));
	}
}

