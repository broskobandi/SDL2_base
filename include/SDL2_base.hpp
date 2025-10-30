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

/** @file include/SDL2_base.hpp
 * @brief Public header file for the SDL2_base library.
 * @detail This header-only library provides type and memory safe 
 * wrappers around fundamental objects and functionalities of SDL2. */

#ifndef SDL2_BASE_HPP
#define SDL2_BASE_HPP

#include <SDL2/SDL.h>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <iostream>
#include <vector>

#ifndef NDEBUG
#define DBGMSG(message)\
	std::cout << "[SDL2_BASE_DEBUG]: " << message << "\n";
#else
#define DBGMSG(message)
#endif

namespace SDL2_Base {

	// Custom types

	using Window =std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
	using Renderer = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
	using Surface = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
	using Texture = std::shared_ptr<SDL_Texture>;

	// Main class

	/** Class store and manage SDL2_Base resources. */
	class Base {

		private:

		/** Class to manage the SDL2 session. */
		class SDL {

			private:
			
			Uint32 flags;
			
			public:

			/** Constructor of the SDL class.
			 * \param flags SDL init flags.
			 * \throws std::runtime_error on failure. */
			SDL(Uint32 flags) : flags(flags) {
				if (SDL_Init(flags))
					throw std::runtime_error("Failed to init SDL.");
				DBGMSG("SDL2 initialized.");
			}

			~SDL() {
				if (SDL_WasInit(flags) == flags)
					SDL_Quit();
				DBGMSG("SDL2 terminated.");
			}

		};

		// Variables

		SDL sdl;
		Window win;
		Renderer ren;
		[[maybe_unused]] SDL_Event event;
		[[maybe_unused]] bool is_running {true};
		std::map<std::string, Texture> textures_map;

		public:

		/** Constructor of the Base class.
		 * @param init_flags SDL2 init flag(s) separated by '|'.
		 * @param title The window title. 
		 * @param w The width of the window.
		 * @param h the height of the window.
		 * @param win_flags SDL2 window flag(s) separated by '|'.
		 * @param ren_flags SDL2 renderer flag(s) separated by '|'.
		 * @throws std::runtime_error on failure. */
		Base(
			Uint32 init_flags,
			std::string_view title,
			int w, int h,
			Uint32 win_flags,
			Uint32 ren_flags
		) :
			sdl(init_flags),
			win(
				[&]() {
					auto wi =
						SDL_CreateWindow(title.data(), 0, 0, w, h, win_flags);
					if (!wi)
						throw std::runtime_error("Failed to create window.");
					DBGMSG("Window created.");
					return wi;
				}(),
				[](SDL_Window* w) {
					if (w) SDL_DestroyWindow(w);
					DBGMSG("Window destroyed.");
				}
			),
			ren(
				[&](){
					auto r = SDL_CreateRenderer(win.get(), -1, ren_flags);
					if (!r)
						throw std::runtime_error("Failed to create renderer.");
					DBGMSG("Renderer created.");
					return r;
				}(),
				[](SDL_Renderer* r) {
					if (r) SDL_DestroyRenderer(r);
					DBGMSG("Renderer destroyed.");
				}
			)
		{}

		/** Set renderer draw color.
		 * @param col RGBA color.
		 * @throws std::runtime_error on failure. */
		void set_draw_color(SDL_Color col) {
			if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
				throw std::runtime_error("Failed to set draw color.");
		}

		/** Clear renderer.
		 * @throws std::runtime_error on failure. */
		void clear() {
			if (SDL_RenderClear(ren.get()))
				throw std::runtime_error("Failed to clear renderer.");
		}

		/** Presents the renderer. */
		void present() {
			SDL_RenderPresent(ren.get());
		}

		/** Checks if the texture was loaded.
		 * @param bmp Path to the bmp.
		 * @return a boolean indicating the result. */
		bool is_texture_loaded(std::string_view bmp) {
			auto tex = textures_map.find(std::string(bmp));
			return tex != textures_map.end();
		}

		/** Creates a Texture from a bmp file and stores the path and the 
		 * resulting texture in a std::map. 
		 * @param path_to_bmp Path to the bmp file. 
		 * @throws std::runtime_error on failure. */
		void load_texture(std::string_view path_to_bmp) {
			if (is_texture_loaded(path_to_bmp)) {
				DBGMSG("Texture has already been loaded for bmp:");
				DBGMSG(path_to_bmp);
			}
			Surface sur(
				[&](){
					auto s = SDL_LoadBMP(path_to_bmp.data());
					if (!s) throw std::runtime_error("Failed to load bmp.");
					DBGMSG("Texture created from bmp:");
					DBGMSG(path_to_bmp);
					return s;
				}(),
				[](SDL_Surface* s){
					if (s) SDL_FreeSurface(s);
					DBGMSG("Surface freed.");
				}
			);
			Texture tex(
				[&](){
					auto t =
						SDL_CreateTextureFromSurface(ren.get(), sur.get());
					if (!t)
						throw std::runtime_error("Failed to create texture.");
					return t;
				}(),
				[](SDL_Texture* t){
					if (t) SDL_DestroyTexture(t);
					DBGMSG("Texture destroyed.");
				}
			);
			textures_map.emplace(path_to_bmp, tex);
		}

		/** Returns a shared pointer to a Texture. 
		 * It loads the texture lazily if it hasn't been loaded before.
		 * @param bmp Path to the bmp. 
		 * @return The Texture.
		 * @throws std::runtime_error on failure. */
		Texture get_texture(std::string_view bmp) {
			auto tex = textures_map.find(std::string(bmp));
			if (tex == textures_map.end()) {
				load_texture(bmp);
				tex = textures_map.find(std::string(bmp));
			}
			DBGMSG("Texture found for bmp:");
			DBGMSG(bmp);
			return tex->second;
		}

		/** Returns a map of Strings and Textures associated.
		 * Loads textures that have not been loaded before lazily.
		 * @param bmps A list of bmps to return a map to.
		 * @throws std::runtime_error on failure. */
		auto get_textures_map(std::vector<std::string_view> bmps) {
			std::map<std::string, Texture> map;
			for (auto bmp : bmps) {
				auto tex = get_texture(bmp);
				map.emplace(bmp, tex);
			}
			DBGMSG("String/Texture map created.");
			return map;
		}

		/** Draws a rectangle that's filled with the specified color.
		 * @param dstrect The destination rectangle. 
		 * @parama col The desired color of th rectangle.
		 * @throws std::runtime_error on failure. */
		void draw(SDL_Rect dstrect, SDL_Color col) {
			if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
				throw std::runtime_error("Failed to set draw color.");
			if (SDL_RenderFillRect(ren.get(), &dstrect))
				throw std::runtime_error("Failed to fill rect.");
		}

		/** Draws a rectangle that's filled with the specified color (float).
		 * @param dstrect The destination rectangle. 
		 * @parama col The desired color of th rectangle.
		 * @throws std::runtime_error on failure. */
		void draw(SDL_FRect dstfrect, SDL_Color col) {
			if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
				throw std::runtime_error("Failed to set draw color.");
			if (SDL_RenderFillRectF(ren.get(), &dstfrect))
				throw std::runtime_error("Failed to fill rect.");
		}

		/** Draws a texture.
		 * @param tex The texture to draw.
		 * @param srcrect A pointer to the reqion on the texture to be drawn
		 * (pass nullptr to render the whole texture).
		 * @param dstrect A pointer to the rectangle to draw the texture on
		 * (pass nullptr to render on the whole render target).
		 * @param angle The angle to rotate the texture with (pass 0.0 to not rotate).
		 * @param flip Value indicating whether to flip the texture on tho horizontal or 
		 * vertical axis (pass SDL_FLIP_NONE for not flipping). */
		void draw(
			Texture tex, SDL_Rect* srcrect,
			SDL_Rect* dstrect, float angle,
			SDL_RendererFlip flip
		) {
			if (SDL_RenderCopyEx(ren.get(), tex.get(), srcrect, dstrect, angle, nullptr, flip))
				throw std::runtime_error("Failed to render texxture.");
		}

		/** Draws a texture. (float)
		 * @param tex The texture to draw.
		 * @param srcrect A pointer to the reqion on the texture to be drawn
		 * (pass nullptr to render the whole texture).
		 * @param dstrect A pointer to the rectangle to draw the texture on
		 * (pass nullptr to render on the whole render target).
		 * @param angle The angle to rotate the texture with (pass 0.0 to not rotate).
		 * @param flip Value indicating whether to flip the texture on tho horizontal or 
		 * vertical axis (pass SDL_FLIP_NONE for not flipping). */
		void draw(
			Texture tex, SDL_Rect* srcrect,
			SDL_FRect* dstrect, float angle,
			SDL_RendererFlip flip
		) {
			if (SDL_RenderCopyExF(ren.get(), tex.get(), srcrect, dstrect, angle, nullptr, flip))
				throw std::runtime_error("Failed to render texxture.");
		}
	};
}

#endif
