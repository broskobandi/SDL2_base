/*
MIT License

Copyright (c) 2025 broskobandi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** \file include/SDL2_base.hpp
 * \brief Public header file for the SDL2_base library.
 * \detail This header-only library provides type and memory safe 
 * wrappers around fundamental objects and functionalities of SDL2. */

#ifndef SDL2_BASE_HPP
#define SDL2_BASE_HPP

#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifndef NDEBUG
#define DBGMSG(message)\
	std::cout << SDL_GetTicks() << "\t" << (message) << "\n";
#else
#define DBGMSG(message)
#endif

/** Class to store and manage SDL2 resources. */
class Base {

private:

#ifdef TEST
	friend struct Test;
#endif

	/** Struct to handle initialization and termination of SDL2.
	 * Throws runtime_error on failure. */
	struct Sdl {
		Uint32 flags;
		Sdl(Uint32 flags) : flags(flags) {
			if (SDL_Init(flags))
				throw std::runtime_error("Failed to init SDL2.");
		DBGMSG("SDL context initialized.");
		}
		~Sdl() {
			DBGMSG("Terminating SDL context.");
			SDL_Quit();
		}
	};

	// Custom types

	using Window = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
	using Renderer = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
	using Surface = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
	using Texture = std::shared_ptr<SDL_Texture>;

	// Variables

	Sdl sdl;
	Window win;
	Renderer ren;
	std::vector<Texture> textures;

	// Private functions

	/** Draws a texture on the specified target.
	 * Throws runtime_error on failure.
	 * \param tex A shared pointer to the texture object.
	 * \param srcrect The portion of the texture to be rendered. Pass nullopt 
	 * to render the entire texture to the target.
	 * \param dstrect The target rect to copy the texture over. Pass nullopt 
	 * to render to the entire renderer target.
	 * \param dstrect The target (float based) rect to copy the texture over.
	 * Pass nullopt 
	 * to render to the entire renderer target.
	 * \param angle Value representing the number of degrees the image should 
	 * be rotated by. Pass 0.0f for no rotation.
	 * \param flip Value expressing whether to flip the image. */
	void draw_texture_internal(
		const std::shared_ptr<SDL_Texture>& tex,
		std::optional<SDL_Rect> srcrect,
		std::optional<SDL_Rect> dstrect,
		std::optional<SDL_FRect> dstrectf,
		float angle,
		SDL_RendererFlip flip
	) {
		SDL_Rect* src = srcrect.has_value() ? &srcrect.value() : nullptr;
		SDL_Rect* dst = dstrect.has_value() ? &dstrect.value() : nullptr;
		SDL_FRect* dstf = dstrectf.has_value() ? &dstrectf.value() : nullptr;
		if (dstf) {
			if (
				SDL_RenderCopyExF(
					ren.get(), tex.get(), src, dstf, angle, nullptr, flip
				)
			)
					throw std::runtime_error("Failed to draw texture.");
		} else {
			if (
				SDL_RenderCopyEx(
					ren.get(), tex.get(), src, dst, angle, nullptr, flip
				)
			)
					throw std::runtime_error("Failed to draw texture.");
		}
	}


public:

	/** Struct to store information about an object's dimensions as integers.
	 * It defines operator<, so it can be used in a map or unordered_map. 
	 * It also defines operator==. */
	struct Dimensions {
		int w, h;
		bool operator<(const Dimensions& other) const {
			if (w == other.w)
				return h < other.h;
			return w < other.w;
		}
		bool operator==(const Dimensions& other) const {
			return w == other.w && h == other.h;
		}
	};

	/** Struct to store information about an object's dimensions as floats.
	 * It defines operator<, so it can be used in a map or unordered_map. 
	 * It also defines operator==. */
	struct FDimensions {
		float w, h;
		bool operator<(const FDimensions& other) const {
			if (w == other.w)
				return h < other.h;
			return w < other.w;
		}
		bool operator==(const FDimensions& other) const {
			return w == other.w && h == other.h;
		}
	};

	/** Struct to store information about an object's coordinates as ints.
	 * It defines operator<, so it can be used in a map or unordered_map. 
	 * It also defines operator==. */
	struct Coordinates {
		int x, y;
		bool operator<(const Coordinates& other) const {
			if (x == other.x)
				return y < other.y;
			return x < other.x;
		}
		bool operator==(const Coordinates& other) const {
			return x == other.x && y == other.y;
		}
	};

	/** Struct to store information about an object's coordinates as floats.
	 * It defines operator<, so it can be used in a map or unordered_map. 
	 * It also defines operator==. */
	struct FCoordinates {
		float x, y;
		bool operator<(const FCoordinates& other) const {
			if (x == other.x)
				return y < other.y;
			return x < other.x;
		}
		bool operator==(const FCoordinates& other) const {
			return x == other.x && y == other.y;
		}
	};

	/** Constructor of the Base class.
	 * Throws runtime_error on failure. */
	Base(
		Uint32 init_flags,
		std::string_view title,
		int w,
		int h,
		Uint32 win_flags,
		Uint32 ren_flags
	) :
		sdl(init_flags),
		win(
			[&](){
				auto wi =
					SDL_CreateWindow(title.data(), 0, 0, w, h, win_flags);
				if (!wi)
					throw std::runtime_error("Failed to create window.");
				DBGMSG("Window created succecssfully.");
				return wi;
			}(),
			[](SDL_Window* w){
				if (w) SDL_DestroyWindow(w);
				DBGMSG("Window destroyed.");
			}
		),
		ren(
			[&](){
				auto r = SDL_CreateRenderer(win.get(), -1, ren_flags);
				if (!r) throw std::runtime_error("Failed to create renderer.");
				DBGMSG("Renderer created succecssfully.");
				return r;
			}(),
			[](SDL_Renderer* r){
				if (r) SDL_DestroyRenderer(r);
				DBGMSG("Renderer destroyed.");
			}
		)
	{}

	/** Sets the renderers color.
	 * Throws runtime_error on failure.
	 * \param col The desired color. */
	void set_draw_color(SDL_Color col) {
		if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
			throw std::runtime_error("Failed to set draw color.");
	}

	/** Clears the canvas with the renderer's currently set draw color.
	 * Throws runtime_error on failure. */
	void clear() {
		if (SDL_RenderClear(ren.get()))
			throw std::runtime_error("Failed to clear renderer.");
	}

	/** Presents everything that was drawn since the last call to clear().
	 * Throws runtime_error on failure. */
	void present() {
		SDL_RenderPresent(ren.get());
	}

	/** Creates a new texture from a bmp file.
	 * Throws runtime_error on failure.
	 * \param path_to_bmp The path to the bmp file to be used.
	 * \return A shared pointer to the newly created texture object. */
	[[nodiscard]] Texture create_texture(std::string_view path_to_bmp) {
		auto sur = Surface(
			[&](){
				auto s = SDL_LoadBMP(path_to_bmp.data());
				if (!s) throw std::runtime_error("Failed to load bmp.");
				DBGMSG("Surface succecssfully created from bmp.");
				DBGMSG("\t");
				DBGMSG(path_to_bmp.data());
				return s;
			}(),
			[](SDL_Surface* s){
				if (s) SDL_FreeSurface(s);
				DBGMSG("Surface freed.");
			}
		);
		auto tex = Texture(
			[&](){
				auto t = SDL_CreateTextureFromSurface(ren.get(), sur.get());
				if (!t) throw std::runtime_error("Failed to create texture.");
				DBGMSG("Texture created succecssfully.");
				return t;
			}(),
			[](SDL_Texture* t){
				if (t) SDL_DestroyTexture(t);
				DBGMSG("Texture destroyed.");
			}
		);
		textures.push_back(tex);
		return tex;
	}

	// draw_texture overloads

	/** Draws a texture on the specified target.
	 * Throws runtime_error on failure.
	 * \param tex A shared pointer to the texture object.
	 * \param srcrect The portion of the texture to be rendered. Pass nullopt 
	 * to render the entire texture to the target.
	 * \param dstrect The target rect to copy the texture over. Pass nullopt 
	 * to render to the entire renderer target.
	 * \param angle Value representing the number of degrees the image should 
	 * be rotated by. Pass 0.0f for no rotation.
	 * \param flip Value expressing whether to flip the image. */
	void draw_texture(
		const Texture& tex,
		std::optional<SDL_Rect> srcrect,
		std::optional<SDL_Rect> dstrect,
		float angle,
		SDL_RendererFlip flip
	) {
		draw_texture_internal(tex, srcrect, dstrect, std::nullopt, angle, flip);
	}

	/** Draws a texture on the specified target (for float based rect).
	 * Throws runtime_error on failure.
	 * \param tex A shared pointer to the texture object.
	 * \param srcrect The portion of the texture to be rendered. Pass nullopt 
	 * to render the entire texture to the target.
	 * \param dstrect The target rect to copy the texture over. Pass nullopt 
	 * to render to the entire renderer target.
	 * \param angle Value representing the number of degrees the image should 
	 * be rotated by. Pass 0.0f for no rotation.
	 * \param flip Value expressing whether to flip the image. */
	void draw_texture(
		const Texture& tex,
		SDL_Rect& srcrect,
		SDL_FRect& dstrect,
		float angle,
		SDL_RendererFlip flip
	) {
		draw_texture_internal(tex, srcrect, std::nullopt, dstrect, angle, flip);
	}

	/** Sets the window size.
	 * \param w Width.
	 * \param h Height. */
	void set_window_size(int w, int h) {
		SDL_SetWindowSize(win.get(), w, h);
		DBGMSG("Window width set to");
		DBGMSG("\t");
		DBGMSG(w);
		DBGMSG("\t");
		DBGMSG(h);
	}

	/** Returns the window dimensions.
	 * \return The current size of the window. */
	[[nodiscard]] Dimensions get_window_size() {
		Dimensions dim;
		SDL_GetWindowSize(win.get(), &dim.w, &dim.h);
		return dim;
	}
};

#endif
