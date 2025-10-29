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
#include <memory>
#include <stdexcept>
#include <string_view>
#include <iostream>

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
	using Texture = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;
	using Rect = SDL_Rect;
	using FRect = SDL_FRect;
	using Event = SDL_Event;

	/** Class store and manage SDL2_Base resources. */
	class Base {

		private:

		/** Class to manage the SDL2 session. */
		class SDL {

			private:
			
			Uint32 flags;
			
			public:

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
		[[maybe_unused]] Event event;
		[[maybe_unused]] bool is_running {true};

		public:

		/** Constructor of the Base class.
		 * @param init_flags SDL2 init flag(s) separated by '|'.
		 * @param title The window title. 
		 * @param w The width of the window.
		 * @param h the height of the window.
		 * @param win_flags SDL2 window flag(s) separated by '|'.
		 * @param ren_flags SDL2 renderer flag(s) separated by '|'. */
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

		/** Presents renderer. */
		void present() {
			SDL_RenderPresent(ren.get());
		}
	};
}

#endif
