#include <iostream>
#include <string>
#include <string_view>
#include "parser.hpp"

int main() {
	while (true) {
		std::string line;
		std::cout << ">>> ";
		std::getline(std::cin, line);

		if (line == "quit" || line == "exit") {
			return 0;
		}

		std::string_view input(line);
		auto exp = parse(input, std::optional<expr>(), std::optional<binary_op>());
		if (!exp) {
			std::cout << "Parse error. Try again." << std::endl;
			continue;
		}
		auto value = eval(*exp);
		if (!value) {
			std::cout << "Division or mod by 0. Try again." << std::endl;
			continue;
		}
		std::cout << *value << std::endl;
	}

	return 0;
}

