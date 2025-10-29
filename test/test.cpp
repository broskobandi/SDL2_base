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
		auto path = "../assets/face.bmp";
		base.load_texture(path);
		CTEST(base.is_texture_loaded(path));
		auto tex = base.get_texture(path);
		base.load_texture(path);
		std::vector<std::string_view> paths;
		paths.push_back(path);
		auto map = base.get_textures_map(paths);
		CTEST(map.find(path)->second == tex);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << "\n";
	}
	ctest_print_results();
	return 0;
}
