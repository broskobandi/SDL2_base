#include "SDL2_base.hpp"
#include <ctest.h>
#include <iostream>
#include <stdexcept>

using namespace SDL2_Base;

int main(void) {
	try {
		Base base(
			SDL_INIT_EVERYTHING,
			"Test",
			800, 600,
			SDL_WINDOW_SHOWN,
			SDL_RENDERER_PRESENTVSYNC
		);
		CTEST(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << "\n";
	}
	ctest_print_results();
	return 0;
}
