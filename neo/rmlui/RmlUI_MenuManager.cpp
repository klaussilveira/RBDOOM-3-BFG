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

#include "RmlUI_MenuManager.h"
#include "RmlUI_Hooks.h"
#include <RmlUi/Core/Element.h>

// Game options CVars
extern idCVar g_fov;
extern idCVar g_checkpoints;
extern idCVar ui_autoSwitch;
extern idCVar ui_autoReload;
extern idCVar aa_targetAimAssistEnable;
extern idCVar in_alwaysRun;
extern idCVar g_muzzleFlash;

// Controls CVars
extern idCVar in_mouseInvertLook;
extern idCVar in_mouseSpeed;
extern idCVar in_useJoystick;

// System options CVars
extern idCVar r_swapInterval;
extern idCVar r_antiAliasing;
extern idCVar r_useSSAO;
extern idCVar r_useSSR;
extern idCVar r_useFilmicPostFX;
extern idCVar r_useCRTPostFX;
extern idCVar r_forceAmbient;
extern idCVar r_exposure;
extern idCVar r_lightScale;
extern idCVar s_volume_dB;
extern idCVar com_engineHz;
extern idCVar r_fullscreen;

// Classic flashlight CVar
extern idCVar ng_classicFlashlight;

/*
========================
idRmlMenuManager::idRmlMenuManager
========================
*/
idRmlMenuManager::idRmlMenuManager()
	: context( nullptr )
	, currentDocument( nullptr )
	, hudDocument( nullptr )
	, currentScreen( RmlUIHook::SCREEN_NONE )
	, pendingScreen( RmlUIHook::SCREEN_NONE )
	, settingsReturnScreen( RmlUIHook::SCREEN_MAIN_MENU )
	, selectedGameIndex( 1 )  // Default to Doom 3 (index 1)
	, newGameType( 0 )        // Default to Doom 3 (type 0)
	, selectedDifficulty( 1 ) // Default to Medium
{
}

/*
========================
idRmlMenuManager::~idRmlMenuManager
========================
*/
idRmlMenuManager::~idRmlMenuManager()
{
	Shutdown();
}

/*
========================
idRmlMenuManager::Initialize
========================
*/
void idRmlMenuManager::Initialize( Rml::Context* ctx )
{
	context = ctx;
	currentDocument = nullptr;
	hudDocument = nullptr;
	currentScreen = RmlUIHook::SCREEN_NONE;
	pendingScreen = RmlUIHook::SCREEN_NONE;
}

/*
========================
idRmlMenuManager::Shutdown
========================
*/
void idRmlMenuManager::Shutdown()
{
	UnloadHUD();
	UnloadCurrentScreen();
	context = nullptr;
}

/*
========================
idRmlMenuManager::SetScreen
========================
*/
void idRmlMenuManager::SetScreen( RmlUIHook::Screen screen )
{
	if( screen == currentScreen )
	{
		return;
	}

	// Keep the hooks-layer g_CurrentScreen in sync so IsMenuActive()
	// returns the correct value when we navigate internally
	// (ESC back, ResumeGame, ExitToMainMenu, etc.)
	RmlUIHook::NotifyScreenChanged( screen );

	// Handle closing immediately for SCREEN_NONE
	if( screen == RmlUIHook::SCREEN_NONE )
	{
		UnloadCurrentScreen();
		return;
	}

	pendingScreen = screen;
}

/*
========================
idRmlMenuManager::Update
========================
*/
void idRmlMenuManager::Update()
{
	// Handle pending screen transition
	if( pendingScreen != RmlUIHook::SCREEN_NONE && pendingScreen != currentScreen )
	{
		LoadScreen( pendingScreen );
		pendingScreen = RmlUIHook::SCREEN_NONE;
	}
}

/*
========================
idRmlMenuManager::LoadScreen
========================
*/
void idRmlMenuManager::LoadScreen( RmlUIHook::Screen screen )
{
	if( context == nullptr )
	{
		return;
	}

	UnloadCurrentScreen();

	const char* documentPath = nullptr;

	switch( screen )
	{
		case RmlUIHook::SCREEN_GAME_SELECT:
			documentPath = "rmlui/game_select.rml";
			break;

		case RmlUIHook::SCREEN_MAIN_MENU:
			documentPath = "rmlui/main_menu.rml";
			break;

		case RmlUIHook::SCREEN_CAMPAIGN:
			documentPath = "rmlui/campaign.rml";
			break;

		case RmlUIHook::SCREEN_NEW_GAME:
			documentPath = "rmlui/new_game.rml";
			break;

		case RmlUIHook::SCREEN_DIFFICULTY:
			documentPath = "rmlui/difficulty.rml";
			break;

		case RmlUIHook::SCREEN_SETTINGS:
			documentPath = "rmlui/settings.rml";
			break;

		case RmlUIHook::SCREEN_LOAD_GAME:
			documentPath = "rmlui/load_game.rml";
			break;

		case RmlUIHook::SCREEN_CREDITS:
			documentPath = "rmlui/credits.rml";
			break;

		case RmlUIHook::SCREEN_PAUSE:
			documentPath = "rmlui/pause.rml";
			break;

		case RmlUIHook::SCREEN_CONTROLS:
			documentPath = "rmlui/controls.rml";
			break;

		case RmlUIHook::SCREEN_GAME_OPTIONS:
			documentPath = "rmlui/game_options.rml";
			break;

		case RmlUIHook::SCREEN_SYSTEM_OPTIONS:
			documentPath = "rmlui/system_options.rml";
			break;

		case RmlUIHook::SCREEN_SAVE_GAME:
			documentPath = "rmlui/save_game.rml";
			break;

		default:
			return;
	}

	currentDocument = context->LoadDocument( documentPath );
	if( currentDocument == nullptr )
	{
		common->Warning( "RmlUI: Failed to load document '%s'", documentPath );
		return;
	}

	// Add event listeners to the body element.
	// Click events from buttons and data-action divs will bubble up here.
	if( Rml::Element* body = currentDocument->GetElementById( "body" ) )
	{
		body->AddEventListener( Rml::EventId::Click, this );
		body->AddEventListener( Rml::EventId::Keydown, this );
	}

	currentDocument->Show();
	currentScreen = screen;

	// Update UI elements with current CVar values
	UpdateScreenValues();

	common->Printf( "RmlUI: Loaded screen '%s'\n", documentPath );
}

/*
========================
idRmlMenuManager::UnloadCurrentScreen
========================
*/
void idRmlMenuManager::UnloadCurrentScreen()
{
	if( currentDocument != nullptr )
	{
		// Remove event listeners before closing to prevent callbacks during cleanup
		if( Rml::Element* body = currentDocument->GetElementById( "body" ) )
		{
			body->RemoveEventListener( Rml::EventId::Click, this );
			body->RemoveEventListener( Rml::EventId::Keydown, this );
		}

		currentDocument->Close();
		currentDocument = nullptr;
	}

	currentScreen = RmlUIHook::SCREEN_NONE;
}

/*
========================
idRmlMenuManager::ProcessEvent

Handle RmlUI events from buttons and other interactive elements.
========================
*/
void idRmlMenuManager::ProcessEvent( Rml::Event& event )
{
	// Use GetTargetElement() to get the original clicked element, not the bubbling target
	Rml::Element* element = event.GetTargetElement();
	if( element == nullptr )
	{
		return;
	}

	// Handle click events on buttons
	if( event.GetId() == Rml::EventId::Click )
	{
		// Walk up the DOM tree to find an element with an action or ID
		Rml::Element* actionElement = element;
		while( actionElement != nullptr )
		{
			// Check for action attribute
			Rml::String action = actionElement->GetAttribute<Rml::String>( "data-action", "" );
			if( !action.empty() )
			{
				HandleAction( action );
				return;
			}

			// Check for ID on button elements
			Rml::String id = actionElement->GetId();
			if( !id.empty() && ( actionElement->GetTagName() == "button" || id.rfind( "btn_", 0 ) == 0 ) )
			{
				HandleAction( id );
				return;
			}

			actionElement = actionElement->GetParentNode();
		}
	}
	// Handle keyboard events for navigation
	else if( event.GetId() == Rml::EventId::Keydown )
	{
		Rml::Input::KeyIdentifier key = static_cast<Rml::Input::KeyIdentifier>(
											event.GetParameter<int>( "key_identifier", 0 ) );

		switch( key )
		{
			case Rml::Input::KI_LEFT:
				if( currentScreen == RmlUIHook::SCREEN_GAME_SELECT )
				{
					selectedGameIndex = ( selectedGameIndex + 2 ) % 3;  // Wrap around
					// Update visual selection
				}
				else if( currentScreen == RmlUIHook::SCREEN_GAME_OPTIONS ||
						 currentScreen == RmlUIHook::SCREEN_SYSTEM_OPTIONS ||
						 currentScreen == RmlUIHook::SCREEN_CONTROLS )
				{
					// Get focused element and adjust slider values
					Rml::Element* focused = currentDocument->GetFocusLeafNode();
					if( focused != nullptr )
					{
						Rml::String action = focused->GetAttribute<Rml::String>( "data-action", "" );
						if( !action.empty() )
						{
							HandleSliderAdjust( action, -1 );
						}
					}
				}
				break;

			case Rml::Input::KI_RIGHT:
				if( currentScreen == RmlUIHook::SCREEN_GAME_SELECT )
				{
					selectedGameIndex = ( selectedGameIndex + 1 ) % 3;  // Wrap around
					// Update visual selection
				}
				else if( currentScreen == RmlUIHook::SCREEN_GAME_OPTIONS ||
						 currentScreen == RmlUIHook::SCREEN_SYSTEM_OPTIONS ||
						 currentScreen == RmlUIHook::SCREEN_CONTROLS )
				{
					// Get focused element and adjust slider values
					Rml::Element* focused = currentDocument->GetFocusLeafNode();
					if( focused != nullptr )
					{
						Rml::String action = focused->GetAttribute<Rml::String>( "data-action", "" );
						if( !action.empty() )
						{
							HandleSliderAdjust( action, 1 );
						}
					}
				}
				break;

			case Rml::Input::KI_RETURN:
				if( currentScreen == RmlUIHook::SCREEN_GAME_SELECT )
				{
					SelectGame( selectedGameIndex );
				}
				break;

			case Rml::Input::KI_ESCAPE:
				// Handle back navigation based on current screen
				switch( currentScreen )
				{
					case RmlUIHook::SCREEN_MAIN_MENU:
						SetScreen( RmlUIHook::SCREEN_GAME_SELECT );
						break;
					case RmlUIHook::SCREEN_CAMPAIGN:
						SetScreen( RmlUIHook::SCREEN_MAIN_MENU );
						break;
					case RmlUIHook::SCREEN_NEW_GAME:
						SetScreen( RmlUIHook::SCREEN_CAMPAIGN );
						break;
					case RmlUIHook::SCREEN_DIFFICULTY:
						SetScreen( RmlUIHook::SCREEN_NEW_GAME );
						break;
					case RmlUIHook::SCREEN_SETTINGS:
						SetScreen( settingsReturnScreen );
						break;
					case RmlUIHook::SCREEN_LOAD_GAME:
						SetScreen( RmlUIHook::SCREEN_CAMPAIGN );
						break;
					case RmlUIHook::SCREEN_CREDITS:
						SetScreen( RmlUIHook::SCREEN_MAIN_MENU );
						break;
					case RmlUIHook::SCREEN_PAUSE:
						ResumeGame();
						break;
					case RmlUIHook::SCREEN_CONTROLS:
						SetScreen( RmlUIHook::SCREEN_SETTINGS );
						break;
					case RmlUIHook::SCREEN_GAME_OPTIONS:
						SetScreen( RmlUIHook::SCREEN_SETTINGS );
						break;
					case RmlUIHook::SCREEN_SYSTEM_OPTIONS:
						SetScreen( RmlUIHook::SCREEN_SETTINGS );
						break;
					case RmlUIHook::SCREEN_SAVE_GAME:
						SetScreen( RmlUIHook::SCREEN_PAUSE );
						break;
					default:
						break;
				}
				break;

			default:
				break;
		}
	}
}

/*
========================
idRmlMenuManager::HandleAction

Process an action string from a button click.
========================
*/
void idRmlMenuManager::HandleAction( const Rml::String& action )
{
	common->Printf( "RmlUI: Action '%s'\n", action.c_str() );

	// Game selection actions
	if( action == "select_doom1" || action == "btn_doom1" )
	{
		SelectGame( 0 );
	}
	else if( action == "select_doom2" || action == "btn_doom2" )
	{
		SelectGame( 2 );
	}
	else if( action == "select_doom3" || action == "btn_doom3" )
	{
		SelectGame( 1 );
	}
	// Main menu actions
	else if( action == "start_game" || action == "btn_start" )
	{
		StartGame();
	}
	else if( action == "open_campaign" || action == "btn_campaign" )
	{
		SetScreen( RmlUIHook::SCREEN_CAMPAIGN );
	}
	else if( action == "open_settings" || action == "btn_settings" )
	{
		settingsReturnScreen = RmlUIHook::SCREEN_MAIN_MENU;
		SetScreen( RmlUIHook::SCREEN_SETTINGS );
	}
	else if( action == "open_credits" || action == "btn_credits" )
	{
		SetScreen( RmlUIHook::SCREEN_CREDITS );
	}
	else if( action == "open_multiplayer" || action == "btn_multiplayer" )
	{
		// Multiplayer is disabled
		common->Printf( "RmlUI: Multiplayer not available\n" );
	}
	else if( action == "quit_game" || action == "btn_quit" || action == "exit_game" || action == "btn_exit" )
	{
		ExitGame();
	}
	// Campaign menu actions
	else if( action == "continue_game" )
	{
		ContinueGame();
	}
	else if( action == "new_game" )
	{
		SetScreen( RmlUIHook::SCREEN_NEW_GAME );
	}
	else if( action == "load_game" )
	{
		SetScreen( RmlUIHook::SCREEN_LOAD_GAME );
	}
	// New game menu actions (select game type)
	else if( action == "select_doom3" || action == "game_doom3" )
	{
		newGameType = 0;
		SetScreen( RmlUIHook::SCREEN_DIFFICULTY );
	}
	else if( action == "select_roe" || action == "game_roe" )
	{
		newGameType = 1;
		SetScreen( RmlUIHook::SCREEN_DIFFICULTY );
	}
	else if( action == "select_le" || action == "game_le" )
	{
		newGameType = 2;
		SetScreen( RmlUIHook::SCREEN_DIFFICULTY );
	}
	// Difficulty menu actions
	else if( action == "difficulty_easy" )
	{
		StartGameWithSettings( newGameType, 0 );
	}
	else if( action == "difficulty_medium" )
	{
		StartGameWithSettings( newGameType, 1 );
	}
	else if( action == "difficulty_hard" )
	{
		StartGameWithSettings( newGameType, 2 );
	}
	else if( action == "difficulty_nightmare" )
	{
		StartGameWithSettings( newGameType, 3 );
	}
	// Navigation actions
	else if( action == "goto_main_menu" || action == "back_to_main" )
	{
		SetScreen( RmlUIHook::SCREEN_MAIN_MENU );
	}
	else if( action == "goto_game_select" || action == "back_to_select" )
	{
		SetScreen( RmlUIHook::SCREEN_GAME_SELECT );
	}
	else if( action == "goto_campaign" || action == "back_to_campaign" )
	{
		SetScreen( RmlUIHook::SCREEN_CAMPAIGN );
	}
	else if( action == "goto_new_game" || action == "back_to_new_game" )
	{
		SetScreen( RmlUIHook::SCREEN_NEW_GAME );
	}
	// Settings menu actions
	else if( action == "open_controls" )
	{
		SetScreen( RmlUIHook::SCREEN_CONTROLS );
	}
	else if( action == "open_game_options" )
	{
		SetScreen( RmlUIHook::SCREEN_GAME_OPTIONS );
	}
	else if( action == "open_system_options" )
	{
		SetScreen( RmlUIHook::SCREEN_SYSTEM_OPTIONS );
	}
	// Pause menu actions
	else if( action == "resume_game" )
	{
		ResumeGame();
	}
	else if( action == "save_game" )
	{
		SetScreen( RmlUIHook::SCREEN_SAVE_GAME );
	}
	else if( action == "pause_settings" )
	{
		settingsReturnScreen = RmlUIHook::SCREEN_PAUSE;
		SetScreen( RmlUIHook::SCREEN_SETTINGS );
	}
	else if( action == "restart_map" )
	{
		RestartMap();
	}
	else if( action == "exit_to_menu" )
	{
		ExitToMainMenu();
	}
	// Controls menu actions
	else if( action == "open_keyboard_bindings" )
	{
		// TODO: Implement keyboard bindings submenu
		common->Printf( "RmlUI: Keyboard bindings not yet implemented\n" );
	}
	else if( action == "open_gamepad_config" )
	{
		// TODO: Implement gamepad config submenu
		common->Printf( "RmlUI: Gamepad config not yet implemented\n" );
	}
	else if( action == "toggle_gamepad" )
	{
		ToggleCVar( in_useJoystick, "gamepad-value" );
	}
	else if( action == "toggle_invert_mouse" )
	{
		ToggleCVar( in_mouseInvertLook, "invert-value" );
	}
	else if( action == "adjust_mouse_sens" )
	{
		// Increment by 10%, wrap around (0.25 to 4.0 range)
		float sens = in_mouseSpeed.GetFloat();
		float sensPercent = ( ( sens - 0.25f ) / ( 4.0f - 0.25f ) ) * 100.0f;
		sensPercent += 10.0f;
		if( sensPercent > 100.0f )
		{
			sensPercent = 0.0f;
		}
		sens = 0.25f + ( ( 4.0f - 0.25f ) * ( sensPercent / 100.0f ) );
		in_mouseSpeed.SetFloat( sens );
		UpdateControlsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// Game options actions
	else if( action == "adjust_fov" )
	{
		// Cycle FOV: 80, 85, 90, 95, 100
		float fov = g_fov.GetFloat();
		fov += 5.0f;
		if( fov > 100.0f )
		{
			fov = 80.0f;
		}
		g_fov.SetFloat( fov );
		UpdateGameOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "toggle_checkpoints" )
	{
		ToggleCVar( g_checkpoints, "checkpoints-value" );
	}
	else if( action == "toggle_auto_switch" )
	{
		ToggleCVar( ui_autoSwitch, "auto-switch-value" );
	}
	else if( action == "toggle_auto_reload" )
	{
		ToggleCVar( ui_autoReload, "auto-reload-value" );
	}
	else if( action == "toggle_aim_assist" )
	{
		ToggleCVar( aa_targetAimAssistEnable, "aim-assist-value" );
	}
	else if( action == "toggle_always_run" )
	{
		ToggleCVar( in_alwaysRun, "always-run-value" );
	}
	else if( action == "toggle_flashlight" )
	{
		ToggleCVar( ng_classicFlashlight, "flashlight-value" );
	}
	else if( action == "toggle_muzzle_flash" )
	{
		ToggleCVar( g_muzzleFlash, "muzzle-value" );
	}
	// System options actions
	else if( action == "cycle_fullscreen" )
	{
		// Cycle: Windowed (0) -> Fullscreen (1) -> Borderless (-1) -> Windowed
		int fs = r_fullscreen.GetInteger();
		if( fs == 0 )
		{
			fs = 1;
		}
		else if( fs > 0 )
		{
			fs = -1;
		}
		else
		{
			fs = 0;
		}
		r_fullscreen.SetInteger( fs );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_framerate" )
	{
		// Cycle: 60 -> 120 -> 60
		// com_engineHz is CVAR_FLOAT, so use GetFloat/SetFloat
		float hz = com_engineHz.GetFloat();
		hz = ( hz < 90.0f ) ? 120.0f : 60.0f;
		com_engineHz.SetFloat( hz );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_vsync" )
	{
		// Cycle: Off (0) -> Smart (1) -> On (2) -> Off
		int vsync = r_swapInterval.GetInteger();
		vsync = ( vsync + 1 ) % 3;
		r_swapInterval.SetInteger( vsync );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_antialiasing" )
	{
		// Cycle: None (0) -> SMAA (1) -> TAA (2) -> None
		int aa = r_antiAliasing.GetInteger();
		aa = ( aa + 1 ) % 3;
		r_antiAliasing.SetInteger( aa );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_render_mode" )
	{
		// TODO: Implement render mode cycling
		common->Printf( "RmlUI: Cycle render mode\n" );
	}
	else if( action == "adjust_ambient" )
	{
		// Increment by 10%, wrap around
		float ambient = r_forceAmbient.GetFloat();
		ambient += 0.1f;
		if( ambient > 1.0f )
		{
			ambient = 0.0f;
		}
		r_forceAmbient.SetFloat( ambient );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "toggle_ssao" )
	{
		ToggleCVar( r_useSSAO, "ssao-value" );
	}
	else if( action == "toggle_reflections" )
	{
		ToggleCVar( r_useSSR, "reflections-value", "Dynamic (SSR)", "Static" );
	}
	else if( action == "toggle_filmic" )
	{
		ToggleCVar( r_useFilmicPostFX, "filmic-value" );
	}
	else if( action == "cycle_crt" )
	{
		// Cycle: Off (0) -> Mattias (1) -> Newpixie (2) -> Advanced (3) -> Off
		int crt = r_useCRTPostFX.GetInteger();
		crt = ( crt + 1 ) % 4;
		r_useCRTPostFX.SetInteger( crt );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "adjust_brightness" )
	{
		// Increment by 10%, wrap around
		float brightness = r_exposure.GetFloat();
		brightness += 0.1f;
		if( brightness > 1.0f )
		{
			brightness = 0.0f;
		}
		r_exposure.SetFloat( brightness );
		// Also adjust light scale
		r_lightScale.SetFloat( 2.0f + ( brightness * 2.0f ) );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "adjust_volume" )
	{
		// Increment by 10%, wrap around
		float volumePercent = 100.0f * ( 1.0f - ( s_volume_dB.GetFloat() / DB_SILENCE ) );
		volumePercent += 10.0f;
		if( volumePercent > 100.0f )
		{
			volumePercent = 0.0f;
		}
		// Convert back to dB
		float db = DB_SILENCE - ( ( volumePercent / 100.0f ) * ( -DB_SILENCE ) );
		s_volume_dB.SetFloat( db );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// Save game actions
	else if( action == "save_new" )
	{
		// TODO: Create new save
		common->Printf( "RmlUI: Create new save\n" );
	}
	else if( action == "quick_save" )
	{
		QuickSave();
	}

	// Force layout update after all text changes
	// This ensures changes made during event processing are reflected immediately
	if( currentDocument != nullptr )
	{
		currentDocument->UpdateDocument();
	}
}

/*
========================
idRmlMenuManager::HandleSliderAdjust

Handle LEFT/RIGHT key adjustments for slider values.
direction: -1 for decrease, 1 for increase
========================
*/
void idRmlMenuManager::HandleSliderAdjust( const Rml::String& action, int direction )
{
	float step = ( direction > 0 ) ? 1.0f : -1.0f;

	// Game options sliders
	if( action == "adjust_fov" )
	{
		float fov = g_fov.GetFloat();
		fov += step * 5.0f;
		if( fov > 100.0f )
		{
			fov = 80.0f;
		}
		else if( fov < 80.0f )
		{
			fov = 100.0f;
		}
		g_fov.SetFloat( fov );
		UpdateGameOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// Controls sliders
	else if( action == "adjust_mouse_sens" )
	{
		float sens = in_mouseSpeed.GetFloat();
		float sensPercent = ( ( sens - 0.25f ) / ( 4.0f - 0.25f ) ) * 100.0f;
		sensPercent += step * 10.0f;
		if( sensPercent > 100.0f )
		{
			sensPercent = 0.0f;
		}
		else if( sensPercent < 0.0f )
		{
			sensPercent = 100.0f;
		}
		sens = 0.25f + ( ( 4.0f - 0.25f ) * ( sensPercent / 100.0f ) );
		in_mouseSpeed.SetFloat( sens );
		UpdateControlsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// System options sliders
	else if( action == "adjust_ambient" )
	{
		float ambient = r_forceAmbient.GetFloat();
		ambient += step * 0.1f;
		if( ambient > 1.0f )
		{
			ambient = 0.0f;
		}
		else if( ambient < 0.0f )
		{
			ambient = 1.0f;
		}
		r_forceAmbient.SetFloat( ambient );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "adjust_brightness" )
	{
		float brightness = r_exposure.GetFloat();
		brightness += step * 0.1f;
		if( brightness > 1.0f )
		{
			brightness = 0.0f;
		}
		else if( brightness < 0.0f )
		{
			brightness = 1.0f;
		}
		r_exposure.SetFloat( brightness );
		r_lightScale.SetFloat( 2.0f + ( brightness * 2.0f ) );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "adjust_volume" )
	{
		float volumePercent = 100.0f * ( 1.0f - ( s_volume_dB.GetFloat() / DB_SILENCE ) );
		volumePercent += step * 10.0f;
		if( volumePercent > 100.0f )
		{
			volumePercent = 0.0f;
		}
		else if( volumePercent < 0.0f )
		{
			volumePercent = 100.0f;
		}
		float db = DB_SILENCE - ( ( volumePercent / 100.0f ) * ( -DB_SILENCE ) );
		s_volume_dB.SetFloat( db );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// Cycle options (LEFT/RIGHT cycles through options)
	else if( action == "cycle_fullscreen" )
	{
		int fs = r_fullscreen.GetInteger();
		if( direction > 0 )
		{
			// Right: 0 -> 1 -> -1 -> 0
			if( fs == 0 ) fs = 1;
			else if( fs > 0 ) fs = -1;
			else fs = 0;
		}
		else
		{
			// Left: 0 -> -1 -> 1 -> 0
			if( fs == 0 ) fs = -1;
			else if( fs < 0 ) fs = 1;
			else fs = 0;
		}
		r_fullscreen.SetInteger( fs );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_framerate" )
	{
		// com_engineHz is CVAR_FLOAT
		float hz = com_engineHz.GetFloat();
		hz = ( hz < 90.0f ) ? 120.0f : 60.0f;
		com_engineHz.SetFloat( hz );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_vsync" )
	{
		int vsync = r_swapInterval.GetInteger();
		if( direction > 0 )
		{
			vsync = ( vsync + 1 ) % 3;
		}
		else
		{
			vsync = ( vsync + 2 ) % 3;
		}
		r_swapInterval.SetInteger( vsync );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_antialiasing" )
	{
		int aa = r_antiAliasing.GetInteger();
		if( direction > 0 )
		{
			aa = ( aa + 1 ) % 3;
		}
		else
		{
			aa = ( aa + 2 ) % 3;
		}
		r_antiAliasing.SetInteger( aa );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	else if( action == "cycle_crt" )
	{
		int crt = r_useCRTPostFX.GetInteger();
		if( direction > 0 )
		{
			crt = ( crt + 1 ) % 4;
		}
		else
		{
			crt = ( crt + 3 ) % 4;
		}
		r_useCRTPostFX.SetInteger( crt );
		UpdateSystemOptionsValues();
		cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
	}
	// Toggle options (LEFT/RIGHT also toggles)
	else if( action == "toggle_checkpoints" )
	{
		ToggleCVar( g_checkpoints, "checkpoints-value" );
	}
	else if( action == "toggle_auto_switch" )
	{
		ToggleCVar( ui_autoSwitch, "auto-switch-value" );
	}
	else if( action == "toggle_auto_reload" )
	{
		ToggleCVar( ui_autoReload, "auto-reload-value" );
	}
	else if( action == "toggle_aim_assist" )
	{
		ToggleCVar( aa_targetAimAssistEnable, "aim-assist-value" );
	}
	else if( action == "toggle_always_run" )
	{
		ToggleCVar( in_alwaysRun, "always-run-value" );
	}
	else if( action == "toggle_flashlight" )
	{
		ToggleCVar( ng_classicFlashlight, "flashlight-value" );
	}
	else if( action == "toggle_muzzle_flash" )
	{
		ToggleCVar( g_muzzleFlash, "muzzle-value" );
	}
	else if( action == "toggle_ssao" )
	{
		ToggleCVar( r_useSSAO, "ssao-value" );
	}
	else if( action == "toggle_reflections" )
	{
		ToggleCVar( r_useSSR, "reflections-value", "Dynamic (SSR)", "Static" );
	}
	else if( action == "toggle_filmic" )
	{
		ToggleCVar( r_useFilmicPostFX, "filmic-value" );
	}
	else if( action == "toggle_gamepad" )
	{
		ToggleCVar( in_useJoystick, "gamepad-value" );
		UpdateControlsValues();
	}
	else if( action == "toggle_invert_mouse" )
	{
		ToggleCVar( in_mouseInvertLook, "invert-value" );
		UpdateControlsValues();
	}

	// Force layout update after all text changes
	// This ensures changes made during event processing are reflected immediately
	if( currentDocument != nullptr )
	{
		currentDocument->UpdateDocument();
	}
}

/*
========================
idRmlMenuManager::SelectGame

Select a game from the game selector.
Index: 0 = Doom 1, 1 = Doom 3, 2 = Doom 2
========================
*/
void idRmlMenuManager::SelectGame( int gameIndex )
{
	selectedGameIndex = gameIndex;

	switch( gameIndex )
	{
		case 0:  // Doom 1
			common->Printf( "RmlUI: Switching to DOOM Classic\n" );
			// Close RmlUI menu before switching to classic game
			SetScreen( RmlUIHook::SCREEN_NONE );
			RmlUIHook::SetEnabled( false );
			common->SwitchToGame( DOOM_CLASSIC );
			break;

		case 1:  // Doom 3 BFG
			common->Printf( "RmlUI: Entering Doom 3 main menu\n" );
			SetScreen( RmlUIHook::SCREEN_MAIN_MENU );
			break;

		case 2:  // Doom 2
			common->Printf( "RmlUI: Switching to DOOM II Classic\n" );
			// Close RmlUI menu before switching to classic game
			SetScreen( RmlUIHook::SCREEN_NONE );
			RmlUIHook::SetEnabled( false );
			common->SwitchToGame( DOOM2_CLASSIC );
			break;
	}
}

/*
========================
idRmlMenuManager::StartGame

Start a new Doom 3 game with default settings.
========================
*/
void idRmlMenuManager::StartGame()
{
	// Start with default: Doom 3, Medium difficulty
	StartGameWithSettings( 0, 1 );
}

/*
========================
idRmlMenuManager::StartGameWithSettings

Start a new game with specific game type and difficulty.
gameType: 0 = Doom 3, 1 = RoE, 2 = Lost Missions
difficulty: 0 = Easy, 1 = Medium, 2 = Hard, 3 = Nightmare
========================
*/
void idRmlMenuManager::StartGameWithSettings( int gameType, int difficulty )
{
	const char* mapName = nullptr;

	switch( gameType )
	{
		case 0:  // Doom 3
			mapName = "game/mars_city1";
			break;
		case 1:  // Resurrection of Evil
			mapName = "game/erebus1";
			break;
		case 2:  // Lost Missions
			mapName = "game/le_enpro1";
			break;
		default:
			mapName = "game/mars_city1";
			break;
	}

	common->Printf( "RmlUI: Starting game type %d, difficulty %d, map %s\n", gameType, difficulty, mapName );

	// Keep menu visible — it will be dismissed when ExecuteMapChange loads the HUD

	// Set difficulty
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "g_skill %d\n", difficulty ) );

	// Disconnect any current game and load the map
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "map %s %d\n", mapName, gameType ) );
}

/*
========================
idRmlMenuManager::ContinueGame

Continue from the last checkpoint.
========================
*/
void idRmlMenuManager::ContinueGame()
{
	common->Printf( "RmlUI: Continuing game\n" );

	// Keep menu visible — it will be dismissed when ExecuteMapChange loads the HUD.
	// If the loadgame fails, the user still has the menu.

	// Load the checkpoint save
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "loadgame checkpointGame\n" );
}

/*
========================
idRmlMenuManager::ExitGame

Exit the game.
========================
*/
void idRmlMenuManager::ExitGame()
{
	common->Printf( "RmlUI: Exiting game\n" );

	// Close the RmlUI menu before exiting
	SetScreen( RmlUIHook::SCREEN_NONE );

	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
}

/*
========================
idRmlMenuManager::ResumeGame

Resume the game from the pause menu.
========================
*/
void idRmlMenuManager::ResumeGame()
{
	common->Printf( "RmlUI: Resuming game\n" );

	// Close the menu overlay — HUD stays visible
	SetScreen( RmlUIHook::SCREEN_NONE );
}

/*
========================
idRmlMenuManager::RestartMap

Restart the current map.
========================
*/
void idRmlMenuManager::RestartMap()
{
	common->Printf( "RmlUI: Restarting map\n" );

	// Unload HUD — it will be reloaded after the map loads
	UnloadHUD();
	SetScreen( RmlUIHook::SCREEN_NONE );

	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "restartMap\n" );
}

/*
========================
idRmlMenuManager::ExitToMainMenu

Exit to the main menu (disconnect current game).
========================
*/
void idRmlMenuManager::ExitToMainMenu()
{
	common->Printf( "RmlUI: Exiting to main menu\n" );

	UnloadHUD();

	// Disconnect and go back to game select screen
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );

	// Show the game select screen
	SetScreen( RmlUIHook::SCREEN_GAME_SELECT );
}

/*
========================
idRmlMenuManager::LoadHUD

Load the HUD document. The HUD persists during gameplay and pause,
rendering underneath any menu documents.
========================
*/
void idRmlMenuManager::LoadHUD()
{
	if( context == nullptr )
	{
		return;
	}

	if( hudDocument != nullptr )
	{
		return; // Already loaded
	}

	hudDocument = context->LoadDocument( "rmlui/hud.rml" );
	if( hudDocument == nullptr )
	{
		common->Warning( "RmlUI: Failed to load HUD document" );
		return;
	}

	// Show the HUD but do NOT add event listeners — HUD must not capture input
	hudDocument->Show();

	common->Printf( "RmlUI: HUD loaded\n" );
}

/*
========================
idRmlMenuManager::UnloadHUD
========================
*/
void idRmlMenuManager::UnloadHUD()
{
	if( hudDocument != nullptr )
	{
		hudDocument->Close();
		hudDocument = nullptr;
		common->Printf( "RmlUI: HUD unloaded\n" );
	}
}

/*
========================
idRmlMenuManager::IsHudLoaded
========================
*/
bool idRmlMenuManager::IsHudLoaded() const
{
	return hudDocument != nullptr;
}

/*
========================
idRmlMenuManager::UpdateHUD

Update HUD elements with current game state data.
========================
*/
void idRmlMenuManager::UpdateHUD( int health, int armor, float stamina, float maxStamina,
								  int ammoInClip, int ammoAvailable, int clipSize )
{
	if( hudDocument == nullptr )
	{
		return;
	}

	// Health
	Rml::Element* healthValue = hudDocument->GetElementById( "health-value" );
	if( healthValue != nullptr )
	{
		Rml::String newText = va( "%d", health );
		if( healthValue->GetInnerRML() != newText )
		{
			healthValue->SetInnerRML( newText );
		}

		// Color based on health threshold
		if( health > 60 )
		{
			healthValue->SetProperty( "color", "#ffffff" );
		}
		else if( health > 30 )
		{
			healthValue->SetProperty( "color", "#ff8800" );
		}
		else
		{
			healthValue->SetProperty( "color", "#ff0000" );
		}
	}

	// Armor
	Rml::Element* armorValue = hudDocument->GetElementById( "armor-value" );
	if( armorValue != nullptr )
	{
		Rml::String newText = va( "%d", armor );
		if( armorValue->GetInnerRML() != newText )
		{
			armorValue->SetInnerRML( newText );
		}
	}

	// Ammo - hide when ammoAvailable < 0 (melee/fists)
	Rml::Element* ammoContainer = hudDocument->GetElementById( "ammo-container" );
	if( ammoContainer != nullptr )
	{
		if( ammoAvailable < 0 )
		{
			ammoContainer->SetProperty( "display", "none" );
		}
		else
		{
			ammoContainer->RemoveProperty( "display" );

			Rml::Element* clipEl = hudDocument->GetElementById( "ammo-clip" );
			if( clipEl != nullptr )
			{
				Rml::String newText;
				if( clipSize > 0 )
				{
					newText = va( "%d", ammoInClip );
				}
				else
				{
					newText = va( "%d", ammoAvailable );
				}
				if( clipEl->GetInnerRML() != newText )
				{
					clipEl->SetInnerRML( newText );
				}
			}

			Rml::Element* reserveEl = hudDocument->GetElementById( "ammo-reserve" );
			if( reserveEl != nullptr )
			{
				if( clipSize > 0 )
				{
					reserveEl->RemoveProperty( "display" );
					Rml::String newText = va( "/ %d", ammoAvailable );
					if( reserveEl->GetInnerRML() != newText )
					{
						reserveEl->SetInnerRML( newText );
					}
				}
				else
				{
					reserveEl->SetProperty( "display", "none" );
				}
			}
		}
	}

	// Stamina bar - hide when maxStamina <= 0
	Rml::Element* staminaContainer = hudDocument->GetElementById( "stamina-container" );
	if( staminaContainer != nullptr )
	{
		if( maxStamina <= 0.0f )
		{
			staminaContainer->SetProperty( "display", "none" );
		}
		else
		{
			staminaContainer->RemoveProperty( "display" );

			Rml::Element* staminaFill = hudDocument->GetElementById( "stamina-fill" );
			if( staminaFill != nullptr )
			{
				float percent = ( stamina / maxStamina ) * 100.0f;
				if( percent < 0.0f ) percent = 0.0f;
				if( percent > 100.0f ) percent = 100.0f;
				staminaFill->SetProperty( "width", Rml::String( va( "%.0f%%", percent ) ) );
			}
		}
	}
}

/*
========================
idRmlMenuManager::QuickSave

Perform a quick save.
========================
*/
void idRmlMenuManager::QuickSave()
{
	common->Printf( "RmlUI: Quick save\n" );

	// Close the menu overlay, HUD stays
	SetScreen( RmlUIHook::SCREEN_NONE );

	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "savegame quick\n" );
}

/*
========================
idRmlMenuManager::SetElementText

Helper to set the inner text of an element by ID.
========================
*/
void idRmlMenuManager::SetElementText( const char* elementId, const char* text )
{
	if( currentDocument == nullptr )
	{
		return;
	}

	Rml::Element* element = currentDocument->GetElementById( elementId );
	if( element != nullptr )
	{
		Rml::String currentText = element->GetInnerRML();

		// Only update if the text is actually different
		if( currentText != text )
		{
			element->SetInnerRML( text );

			// Force immediate document update to process the new text element's layout
			// This is necessary because SetInnerRML creates a new text element that needs
			// to have its layout calculated before rendering
			currentDocument->UpdateDocument();
		}
	}
	else
	{
		common->Warning( "RmlUI: SetElementText('%s', '%s') - element not found", elementId, text );
	}
}

/*
========================
idRmlMenuManager::SetSliderValue

Helper to set a slider fill width and value text.
========================
*/
void idRmlMenuManager::SetSliderValue( const char* fillId, const char* valueId, float percent )
{
	if( currentDocument == nullptr )
	{
		return;
	}

	// Update fill bar width
	Rml::Element* fill = currentDocument->GetElementById( fillId );
	if( fill != nullptr )
	{
		fill->SetProperty( "width", Rml::String( va( "%.0f%%", percent ) ) );
	}

	// Update value text
	Rml::Element* value = currentDocument->GetElementById( valueId );
	if( value != nullptr )
	{
		value->SetInnerRML( va( "%.0f", percent ) );
	}
}

/*
========================
idRmlMenuManager::UpdateScreenValues

Update UI elements to reflect current CVar values.
========================
*/
void idRmlMenuManager::UpdateScreenValues()
{
	if( currentDocument == nullptr )
	{
		return;
	}

	switch( currentScreen )
	{
		case RmlUIHook::SCREEN_GAME_OPTIONS:
			UpdateGameOptionsValues();
			break;

		case RmlUIHook::SCREEN_SYSTEM_OPTIONS:
			UpdateSystemOptionsValues();
			break;

		case RmlUIHook::SCREEN_CONTROLS:
			UpdateControlsValues();
			break;

		default:
			break;
	}
}

/*
========================
idRmlMenuManager::UpdateGameOptionsValues

Update game options screen with current CVar values.
========================
*/
void idRmlMenuManager::UpdateGameOptionsValues()
{
	// FOV (80-100 range)
	float fov = g_fov.GetFloat();
	float fovPercent = ( ( fov - 80.0f ) / 20.0f ) * 100.0f;
	SetSliderValue( "fov-fill", "fov-value", fov );
	if( Rml::Element* fill = currentDocument->GetElementById( "fov-fill" ) )
	{
		fill->SetProperty( "width", Rml::String( va( "%.0f%%", fovPercent ) ) );
	}

	// Boolean toggles - use GetInteger() like original menu code
	SetElementText( "checkpoints-value", g_checkpoints.GetInteger() ? "On" : "Off" );
	SetElementText( "auto-switch-value", ui_autoSwitch.GetInteger() ? "On" : "Off" );
	SetElementText( "auto-reload-value", ui_autoReload.GetInteger() ? "On" : "Off" );
	SetElementText( "aim-assist-value", aa_targetAimAssistEnable.GetInteger() ? "On" : "Off" );
	SetElementText( "always-run-value", in_alwaysRun.GetInteger() ? "On" : "Off" );
	SetElementText( "flashlight-value", ng_classicFlashlight.GetInteger() ? "On" : "Off" );
	SetElementText( "muzzle-value", g_muzzleFlash.GetInteger() ? "On" : "Off" );
}

/*
========================
idRmlMenuManager::UpdateSystemOptionsValues

Update system options screen with current CVar values.
========================
*/
void idRmlMenuManager::UpdateSystemOptionsValues()
{
	// Fullscreen
	int fullscreen = r_fullscreen.GetInteger();
	if( fullscreen == 0 )
	{
		SetElementText( "fullscreen-value", "Windowed" );
	}
	else if( fullscreen == -1 )
	{
		SetElementText( "fullscreen-value", "Borderless" );
	}
	else
	{
		SetElementText( "fullscreen-value", "Fullscreen" );
	}

	// Framerate - com_engineHz is CVAR_FLOAT, cast to int for display
	SetElementText( "framerate-value", va( "%d FPS", ( int )com_engineHz.GetFloat() ) );

	// VSync
	int vsync = r_swapInterval.GetInteger();
	if( vsync == 0 )
	{
		SetElementText( "vsync-value", "Off" );
	}
	else if( vsync == 1 )
	{
		SetElementText( "vsync-value", "Smart" );
	}
	else
	{
		SetElementText( "vsync-value", "On" );
	}

	// Anti-aliasing
	int aa = r_antiAliasing.GetInteger();
	const char* aaNames[] = { "None", "SMAA", "TAA" };
	if( aa >= 0 && aa <= 2 )
	{
		SetElementText( "aa-value", aaNames[aa] );
	}

	// SSAO - use GetInteger() like original menu code
	SetElementText( "ssao-value", r_useSSAO.GetInteger() ? "On" : "Off" );

	// Blood Reflections (SSR)
	SetElementText( "reflections-value", r_useSSR.GetInteger() ? "Dynamic (SSR)" : "Static" );

	// Filmic Post FX - use GetInteger() like original menu code
	SetElementText( "filmic-value", r_useFilmicPostFX.GetInteger() ? "On" : "Off" );

	// CRT Filter
	int crt = r_useCRTPostFX.GetInteger();
	const char* crtNames[] = { "Off", "Mattias", "Newpixie", "Advanced" };
	if( crt >= 0 && crt <= 3 )
	{
		SetElementText( "crt-value", crtNames[crt] );
	}

	// Ambient lighting (0-1 range to 0-100)
	float ambient = r_forceAmbient.GetFloat() * 100.0f;
	SetSliderValue( "ambient-fill", "ambient-value", ambient );

	// Brightness (0-1 range to 0-100)
	float brightness = r_exposure.GetFloat() * 100.0f;
	SetSliderValue( "brightness-fill", "brightness-value", brightness );

	// Volume (convert from dB)
	float volumePercent = 100.0f * ( 1.0f - ( s_volume_dB.GetFloat() / DB_SILENCE ) );
	if( volumePercent < 0.0f ) volumePercent = 0.0f;
	if( volumePercent > 100.0f ) volumePercent = 100.0f;
	SetSliderValue( "volume-fill", "volume-value", volumePercent );
}

/*
========================
idRmlMenuManager::UpdateControlsValues

Update controls screen with current CVar values.
========================
*/
void idRmlMenuManager::UpdateControlsValues()
{
	// Gamepad enabled - use GetInteger() like original menu code
	SetElementText( "gamepad-value", in_useJoystick.GetInteger() ? "On" : "Off" );

	// Invert mouse
	SetElementText( "invert-value", in_mouseInvertLook.GetInteger() ? "On" : "Off" );

	// Mouse sensitivity (0.25-4.0 range to 0-100)
	float sens = in_mouseSpeed.GetFloat();
	float sensPercent = ( ( sens - 0.25f ) / ( 4.0f - 0.25f ) ) * 100.0f;
	SetSliderValue( "mouse-sens-fill", "mouse-sens-value", sensPercent );
}

/*
========================
idRmlMenuManager::ToggleCVar

Toggle a boolean CVar and update UI.
Uses GetInteger/SetInteger to match original Doom 3 menu behavior.
========================
*/
void idRmlMenuManager::ToggleCVar( idCVar& cvar, const char* elementId )
{
	ToggleCVar( cvar, elementId, "On", "Off" );
}

void idRmlMenuManager::ToggleCVar( idCVar& cvar, const char* elementId, const char* onText, const char* offText )
{
	int newValue = cvar.GetInteger() ? 0 : 1;
	cvar.SetInteger( newValue );
	SetElementText( elementId, newValue ? onText : offText );
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idRmlMenuManager::AdjustSliderCVar

Adjust a slider CVar value.
========================
*/
void idRmlMenuManager::AdjustSliderCVar( idCVar& cvar, float minVal, float maxVal, float step, const char* fillId, const char* valueId )
{
	float current = cvar.GetFloat();
	float newValue = current + step;

	// Wrap around
	if( newValue > maxVal )
	{
		newValue = minVal;
	}
	else if( newValue < minVal )
	{
		newValue = maxVal;
	}

	cvar.SetFloat( newValue );

	// Update UI
	float percent = ( ( newValue - minVal ) / ( maxVal - minVal ) ) * 100.0f;
	SetSliderValue( fillId, valueId, newValue );
	if( currentDocument != nullptr )
	{
		Rml::Element* fill = currentDocument->GetElementById( fillId );
		if( fill != nullptr )
		{
			fill->SetProperty( "width", Rml::String( va( "%.0f%%", percent ) ) );
		}
	}

	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}
