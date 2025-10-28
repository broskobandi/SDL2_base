#include "SDL2_base.hpp"
#include <ctest.h>
#include <iostream>
#include <stdexcept>

struct Test {
	void run_tests() {
		Base base(
			SDL_INIT_EVERYTHING,
			"test",
			800,
			600,
			SDL_WINDOW_SHOWN,
			SDL_RENDERER_PRESENTVSYNC
		);
		CTEST(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING);
	}
};

int main(void) {
	try {
	Test test;
	test.run_tests();
	} catch (std::runtime_error& e) {
		std::cerr << e.what() << "\n";
	}
	ctest_print_results();
	return 0;
}
