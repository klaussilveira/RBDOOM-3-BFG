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

#ifndef NEO_RMLUI_RMLUI_MENUMANAGER_H_
#define NEO_RMLUI_RMLUI_MENUMANAGER_H_

#include "RmlUI_Hooks.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/EventListener.h>

/*
================================================================================
idRmlMenuManager

Manages RmlUI menu documents and screen transitions.
Handles loading/unloading of menu screens and processes UI events.
================================================================================
*/
class idRmlMenuManager : public Rml::EventListener
{
public:
	idRmlMenuManager();
	~idRmlMenuManager();

	// Initialize with an RmlUI context
	void Initialize( Rml::Context* context );

	// Shutdown and cleanup
	void Shutdown();

	// Set the current screen
	void SetScreen( RmlUIHook::Screen screen );

	// Get the current screen
	RmlUIHook::Screen GetCurrentScreen() const
	{
		return currentScreen;
	}

	// Update (called each frame)
	void Update();

	// HUD management
	void LoadHUD();
	void UnloadHUD();
	bool IsHudLoaded() const;
	void UpdateHUD( int health, int armor, float stamina, float maxStamina,
					int ammoInClip, int ammoAvailable, int clipSize );

	// Event listener callback
	void ProcessEvent( Rml::Event& event ) override;

private:
	// Load a specific screen
	void LoadScreen( RmlUIHook::Screen screen );

	// Unload the current screen
	void UnloadCurrentScreen();

	// Handle button actions
	void HandleAction( const Rml::String& action );

	// Handle slider adjustment with LEFT/RIGHT keys
	// direction: -1 for left (decrease), 1 for right (increase)
	void HandleSliderAdjust( const Rml::String& action, int direction );

	// Select a game (Doom 1, 2, or 3)
	void SelectGame( int gameIndex );

	// Start the game with current settings
	void StartGame();

	// Start game with specific type and difficulty
	void StartGameWithSettings( int gameType, int difficulty );

	// Exit the game
	void ExitGame();

	// Continue from last checkpoint
	void ContinueGame();

	// Resume game from pause menu
	void ResumeGame();

	// Restart the current map
	void RestartMap();

	// Exit to main menu (disconnect)
	void ExitToMainMenu();

	// Quick save
	void QuickSave();

	// UI update helpers
	void SetElementText( const char* elementId, const char* text );
	void SetSliderValue( const char* fillId, const char* valueId, float percent );
	void UpdateScreenValues();
	void UpdateGameOptionsValues();
	void UpdateSystemOptionsValues();
	void UpdateControlsValues();

	// CVar helpers
	void ToggleCVar( idCVar& cvar, const char* elementId );
	void ToggleCVar( idCVar& cvar, const char* elementId, const char* onText, const char* offText );
	void AdjustSliderCVar( idCVar& cvar, float minVal, float maxVal, float step, const char* fillId, const char* valueId );

	// RmlUI context
	Rml::Context* context;

	// Current menu document
	Rml::ElementDocument* currentDocument;

	// HUD document (persists independently of menu screens)
	Rml::ElementDocument* hudDocument;

	// Current screen
	RmlUIHook::Screen currentScreen;

	// Pending screen (for transitions)
	RmlUIHook::Screen pendingScreen;

	// Screen to return to from settings (MAIN_MENU or PAUSE)
	RmlUIHook::Screen settingsReturnScreen;

	// Selected game index in game selector (0=Doom1, 1=Doom3, 2=Doom2)
	int selectedGameIndex;

	// New game type (0=Doom3, 1=RoE, 2=Lost Missions)
	int newGameType;

	// Selected difficulty (0=Easy, 1=Medium, 2=Hard, 3=Nightmare)
	int selectedDifficulty;
};

#endif /* NEO_RMLUI_RMLUI_MENUMANAGER_H_ */
