#include <utility>
#include <memory>
#include "syntax_tree.hpp"

binary_syn::binary_syn(binary_op op, expr&& l, expr&& r) : op(op) {
	left = std::make_unique<expr>(std::move(l));
	right = std::make_unique<expr>(std::move(r));
}

unary_syn::unary_syn(unary_op op, expr&& e) : op(op) {
	exp = std::make_unique<expr>(std::move(e));
}

