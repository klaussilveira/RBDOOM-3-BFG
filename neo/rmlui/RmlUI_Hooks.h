/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2024 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms
and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition
Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may
contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef NEO_RMLUI_RMLUI_HOOKS_H_
#define NEO_RMLUI_RMLUI_HOOKS_H_

#include "../sys/sys_public.h"

// Forward declarations
namespace Rml
{
class Context;
class ElementDocument;
}

namespace RmlUIHook
{

// Initialization and shutdown
bool	Init( int windowWidth, int windowHeight );
bool	IsInitialized();
void	Destroy();

// Tell RmlUI that the window size has changed
void	NotifyDisplaySizeChanged( int width, int height );

// Inject a sys event (keyboard, mouse)
// Returns true if the event was consumed
bool	InjectSysEvent( const sysEvent_t* event );

// Inject mouse wheel delta
bool	InjectMouseWheel( int delta );

// Call this once per frame before Update()
void	NewFrame();

// Update RmlUI (layout, animations, etc.)
void	Update();

// Render all RmlUI documents
void	Render();

// Screen management
enum Screen
{
	SCREEN_NONE = -1,
	SCREEN_GAME_SELECT = 0,
	SCREEN_MAIN_MENU = 1,
	SCREEN_CAMPAIGN = 2,
	SCREEN_NEW_GAME = 3,
	SCREEN_DIFFICULTY = 4,
	SCREEN_SETTINGS = 5,
	SCREEN_LOAD_GAME = 6,
	SCREEN_CREDITS = 7,
	SCREEN_PAUSE = 8,
	SCREEN_CONTROLS = 9,
	SCREEN_GAME_OPTIONS = 10,
	SCREEN_SYSTEM_OPTIONS = 11,
	SCREEN_SAVE_GAME = 12
};

void	SetScreen( Screen screen );
Screen	GetCurrentScreen();

// Feature flag control
bool	IsEnabled();
void	SetEnabled( bool enabled );

// Returns true if RmlUI is showing a menu and should capture mouse/input
bool	IsMenuActive();

// HUD management
void	LoadHUD();
void	UnloadHUD();
bool	IsHudLoaded();
void	UpdateHUD( int health, int armor, float stamina, float maxStamina,
				   int ammoInClip, int ammoAvailable, int clipSize );

// Called by the menu manager to keep the hooks-layer screen state in sync
// when navigating internally (ESC back, ResumeGame, etc.)
void	NotifyScreenChanged( Screen screen );

// Access the RmlUI context (for advanced usage)
Rml::Context* GetContext();

} // namespace RmlUIHook

#endif /* NEO_RMLUI_RMLUI_HOOKS_H_ */
