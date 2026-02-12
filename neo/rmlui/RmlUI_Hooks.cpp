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

#include "precompiled.h"
#pragma hdrstop

#include "RmlUI_Hooks.h"
#include "RmlUI_SystemInterface.h"
#include "RmlUI_FileInterface.h"
#include "RmlUI_RenderInterface.h"
#include "RmlUI_InputAdapter.h"
#include "RmlUI_MenuManager.h"

#include <RmlUi/Core.h>

// CVar for enabling/disabling RmlUI menus
idCVar rmlui_enabled( "rmlui_enabled", "0", CVAR_BOOL | CVAR_ARCHIVE, "Use RmlUI menus instead of Flash" );

namespace RmlUIHook
{

namespace
{
	// Global state
	bool g_IsInit = false;
	bool g_Enabled = false;

	// Display dimensions
	int g_DisplayWidth = 640;
	int g_DisplayHeight = 480;

	// Interface implementations
	idRmlSystemInterface* g_SystemInterface = nullptr;
	idRmlFileInterface* g_FileInterface = nullptr;
	idRmlRenderInterface* g_RenderInterface = nullptr;
	idRmlInputAdapter* g_InputAdapter = nullptr;

	// RmlUI context
	Rml::Context* g_Context = nullptr;

	// Menu manager
	idRmlMenuManager* g_MenuManager = nullptr;

	// Current screen
	Screen g_CurrentScreen = SCREEN_NONE;

	/*
	========================
	LoadFonts

	Load TTF fonts for RmlUI rendering.
	========================
	*/
	bool LoadFonts()
	{
		common->Printf( "RmlUI: LoadFonts() called\n" );

		// Try to load fonts from the rmlui/fonts directory
		// We'll use a fallback approach - try common font names

		bool fontLoaded = false;

		// Try to load a sans-serif font - make it a fallback font so it's used for missing glyphs
		common->Printf( "RmlUI: Attempting to load 'rmlui/fonts/LatoLatin-Regular.ttf'...\n" );
		if( Rml::LoadFontFace( "rmlui/fonts/LatoLatin-Regular.ttf", true ) )
		{
			fontLoaded = true;
			common->Printf( "RmlUI: Loaded font 'LatoLatin-Regular.ttf' as fallback\n" );
		}
		else
		{
			common->Warning( "RmlUI: Failed to load 'rmlui/fonts/LatoLatin-Regular.ttf'" );
		}

		// Try to load bold variant
		if( Rml::LoadFontFace( "rmlui/fonts/LatoLatin-Bold.ttf" ) )
		{
			common->Printf( "RmlUI: Loaded font 'LatoLatin-Bold.ttf'\n" );
		}

		// Try to load italic variant
		if( Rml::LoadFontFace( "rmlui/fonts/LatoLatin-Italic.ttf" ) )
		{
			common->Printf( "RmlUI: Loaded font 'LatoLatin-Italic.ttf'\n" );
		}

		if( !fontLoaded )
		{
			common->Warning( "RmlUI: No fonts loaded. Text rendering may not work correctly." );
			common->Warning( "RmlUI: Please place TTF fonts in base/rmlui/fonts/" );
		}

		return fontLoaded;
	}

} // anonymous namespace

/*
========================
Init

Initialize RmlUI and all interfaces.
========================
*/
bool Init( int windowWidth, int windowHeight )
{
	if( IsInitialized() )
	{
		Destroy();
	}

	common->Printf( "RmlUI: Initializing...\n" );

	g_DisplayWidth = windowWidth;
	g_DisplayHeight = windowHeight;

	// Create interface implementations
	g_SystemInterface = new idRmlSystemInterface();
	g_FileInterface = new idRmlFileInterface();
	g_RenderInterface = new idRmlRenderInterface();
	g_InputAdapter = new idRmlInputAdapter();

	g_RenderInterface->SetDisplaySize( windowWidth, windowHeight );
	g_InputAdapter->SetDisplaySize( windowWidth, windowHeight );

	// Set RmlUI interfaces
	Rml::SetSystemInterface( g_SystemInterface );
	Rml::SetFileInterface( g_FileInterface );
	Rml::SetRenderInterface( g_RenderInterface );

	// Initialize RmlUI
	if( !Rml::Initialise() )
	{
		common->Warning( "RmlUI: Failed to initialize RmlUi core" );
		Destroy();
		return false;
	}

	// Load fonts
	LoadFonts();

	// Create RmlUI context
	g_Context = Rml::CreateContext( "main", Rml::Vector2i( windowWidth, windowHeight ) );
	if( g_Context == nullptr )
	{
		common->Warning( "RmlUI: Failed to create RmlUi context" );
		Destroy();
		return false;
	}

	// Create menu manager
	g_MenuManager = new idRmlMenuManager();
	g_MenuManager->Initialize( g_Context );

	g_IsInit = true;
	g_Enabled = rmlui_enabled.GetBool();

	common->Printf( "RmlUI: Initialization complete (%dx%d)\n", windowWidth, windowHeight );

	return true;
}

/*
========================
IsInitialized
========================
*/
bool IsInitialized()
{
	return g_IsInit;
}

/*
========================
Destroy

Shutdown RmlUI and cleanup.
========================
*/
void Destroy()
{
	if( !g_IsInit )
	{
		return;
	}

	common->Printf( "RmlUI: Shutting down...\n" );

	// Mark as not initialized first to prevent any callbacks during shutdown
	g_IsInit = false;
	g_Enabled = false;
	g_CurrentScreen = SCREEN_NONE;

	// Cleanup menu manager first (closes documents)
	if( g_MenuManager != nullptr )
	{
		g_MenuManager->Shutdown();
		delete g_MenuManager;
		g_MenuManager = nullptr;
	}

	// Remove context before calling Rml::Shutdown
	if( g_Context != nullptr )
	{
		Rml::RemoveContext( "main" );
		g_Context = nullptr;
	}

	// Shutdown RmlUI core - this must happen before deleting interfaces
	// because RmlUI may call interface methods during shutdown
	Rml::Shutdown();

	// Now safe to delete interfaces (after RmlUI is fully shut down)
	if( g_RenderInterface != nullptr )
	{
		delete g_RenderInterface;
		g_RenderInterface = nullptr;
	}

	if( g_FileInterface != nullptr )
	{
		delete g_FileInterface;
		g_FileInterface = nullptr;
	}

	if( g_SystemInterface != nullptr )
	{
		delete g_SystemInterface;
		g_SystemInterface = nullptr;
	}

	if( g_InputAdapter != nullptr )
	{
		delete g_InputAdapter;
		g_InputAdapter = nullptr;
	}

	common->Printf( "RmlUI: Shutdown complete\n" );
}

/*
========================
NotifyDisplaySizeChanged
========================
*/
void NotifyDisplaySizeChanged( int width, int height )
{
	if( !g_IsInit )
	{
		return;
	}

	if( g_DisplayWidth != width || g_DisplayHeight != height )
	{
		g_DisplayWidth = width;
		g_DisplayHeight = height;

		if( g_RenderInterface != nullptr )
		{
			g_RenderInterface->SetDisplaySize( width, height );
		}

		if( g_InputAdapter != nullptr )
		{
			g_InputAdapter->SetDisplaySize( width, height );
		}

		if( g_Context != nullptr )
		{
			g_Context->SetDimensions( Rml::Vector2i( width, height ) );
		}

		common->Printf( "RmlUI: Display size changed to %dx%d\n", width, height );
	}
}

/*
========================
InjectSysEvent

Inject a system event into RmlUI.
Returns true if the event was consumed.
========================
*/
bool InjectSysEvent( const sysEvent_t* event )
{
	if( !g_IsInit || !g_Enabled || g_Context == nullptr || g_InputAdapter == nullptr )
	{
		return false;
	}

	return g_InputAdapter->InjectSysEvent( g_Context, event );
}

/*
========================
InjectMouseWheel
========================
*/
bool InjectMouseWheel( int delta )
{
	if( !g_IsInit || !g_Enabled || g_Context == nullptr || g_InputAdapter == nullptr )
	{
		return false;
	}

	return g_InputAdapter->InjectMouseWheel( g_Context, delta );
}

/*
========================
NewFrame

Called at the start of each frame before Update().
========================
*/
void NewFrame()
{
	if( !g_IsInit )
	{
		return;
	}

	// Check if RmlUI was just enabled via cvar
	bool wasEnabled = g_Enabled;
	g_Enabled = rmlui_enabled.GetBool();

	// If just enabled and no screen is set, show the game select screen
	// (but not if a HUD is loaded — that means a map is running)
	if( g_Enabled && !wasEnabled && g_CurrentScreen == SCREEN_NONE && !IsHudLoaded() )
	{
		common->Printf( "RmlUI: Enabled via cvar, showing game select screen\n" );
		SetScreen( SCREEN_GAME_SELECT );
	}
}

/*
========================
Update

Update RmlUI (layout, animations, etc.)
========================
*/
void Update()
{
	if( !g_IsInit || !g_Enabled || g_Context == nullptr )
	{
		return;
	}

	// Update menu manager (handles screen transitions)
	if( g_MenuManager != nullptr )
	{
		g_MenuManager->Update();
	}

	// Update RmlUI context (layout, animations)
	g_Context->Update();
}

/*
========================
Render

Render all RmlUI documents.
========================
*/
void Render()
{
	if( !g_IsInit || !g_Enabled || g_Context == nullptr )
	{
		return;
	}

	g_Context->Render();
}

/*
========================
SetScreen
========================
*/
void SetScreen( Screen screen )
{
	if( !g_IsInit || g_MenuManager == nullptr )
	{
		return;
	}

	g_CurrentScreen = screen;
	g_MenuManager->SetScreen( screen );
}

/*
========================
GetCurrentScreen
========================
*/
Screen GetCurrentScreen()
{
	return g_CurrentScreen;
}

/*
========================
IsEnabled
========================
*/
bool IsEnabled()
{
	return g_IsInit && g_Enabled;
}

/*
========================
IsMenuActive

Returns true if RmlUI is showing a menu and should capture mouse/input.
========================
*/
bool IsMenuActive()
{
	return g_IsInit && g_Enabled && g_CurrentScreen != SCREEN_NONE;
}

/*
========================
SetEnabled
========================
*/
void SetEnabled( bool enabled )
{
	g_Enabled = enabled;
	rmlui_enabled.SetBool( enabled );
}

/*
========================
LoadHUD
========================
*/
void LoadHUD()
{
	if( !g_IsInit || !g_Enabled || g_MenuManager == nullptr )
	{
		return;
	}

	g_MenuManager->LoadHUD();
}

/*
========================
UnloadHUD
========================
*/
void UnloadHUD()
{
	if( !g_IsInit || g_MenuManager == nullptr )
	{
		return;
	}

	g_MenuManager->UnloadHUD();
}

/*
========================
IsHudLoaded
========================
*/
bool IsHudLoaded()
{
	if( !g_IsInit || !g_Enabled || g_MenuManager == nullptr )
	{
		return false;
	}

	return g_MenuManager->IsHudLoaded();
}

/*
========================
UpdateHUD
========================
*/
void UpdateHUD( int health, int armor, float stamina, float maxStamina,
				int ammoInClip, int ammoAvailable, int clipSize )
{
	if( !g_IsInit || !g_Enabled || g_MenuManager == nullptr )
	{
		return;
	}

	g_MenuManager->UpdateHUD( health, armor, stamina, maxStamina, ammoInClip, ammoAvailable, clipSize );
}

/*
========================
NotifyScreenChanged

Called by the menu manager to keep g_CurrentScreen in sync
when the manager navigates internally (ESC back, ResumeGame, etc.)
========================
*/
void NotifyScreenChanged( Screen screen )
{
	g_CurrentScreen = screen;
}

/*
========================
GetContext
========================
*/
Rml::Context* GetContext()
{
	return g_Context;
}

} // namespace RmlUIHook
