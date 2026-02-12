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

#include "RmlUI_InputAdapter.h"

/*
========================
idRmlInputAdapter::idRmlInputAdapter
========================
*/
idRmlInputAdapter::idRmlInputAdapter()
	: mouseX( 960 )  // Start at center of default display
	, mouseY( 540 )
	, displayWidth( 1920 )
	, displayHeight( 1080 )
{
}

/*
========================
idRmlInputAdapter::SetDisplaySize

Set display dimensions for mouse coordinate clamping.
========================
*/
void idRmlInputAdapter::SetDisplaySize( int width, int height )
{
	displayWidth = width;
	displayHeight = height;

	// Re-center mouse if it's outside new bounds
	if( mouseX >= displayWidth )
	{
		mouseX = displayWidth / 2;
	}
	if( mouseY >= displayHeight )
	{
		mouseY = displayHeight / 2;
	}
}

/*
========================
idRmlInputAdapter::TranslateKey

Translate Doom 3 key codes to RmlUI key identifiers.
========================
*/
Rml::Input::KeyIdentifier idRmlInputAdapter::TranslateKey( keyNum_t key ) const
{
	using namespace Rml::Input;

	switch( key )
	{
		// Letters
		case K_A:
			return KI_A;
		case K_B:
			return KI_B;
		case K_C:
			return KI_C;
		case K_D:
			return KI_D;
		case K_E:
			return KI_E;
		case K_F:
			return KI_F;
		case K_G:
			return KI_G;
		case K_H:
			return KI_H;
		case K_I:
			return KI_I;
		case K_J:
			return KI_J;
		case K_K:
			return KI_K;
		case K_L:
			return KI_L;
		case K_M:
			return KI_M;
		case K_N:
			return KI_N;
		case K_O:
			return KI_O;
		case K_P:
			return KI_P;
		case K_Q:
			return KI_Q;
		case K_R:
			return KI_R;
		case K_S:
			return KI_S;
		case K_T:
			return KI_T;
		case K_U:
			return KI_U;
		case K_V:
			return KI_V;
		case K_W:
			return KI_W;
		case K_X:
			return KI_X;
		case K_Y:
			return KI_Y;
		case K_Z:
			return KI_Z;

		// Numbers
		case K_0:
			return KI_0;
		case K_1:
			return KI_1;
		case K_2:
			return KI_2;
		case K_3:
			return KI_3;
		case K_4:
			return KI_4;
		case K_5:
			return KI_5;
		case K_6:
			return KI_6;
		case K_7:
			return KI_7;
		case K_8:
			return KI_8;
		case K_9:
			return KI_9;

		// Numpad
		case K_KP_0:
			return KI_NUMPAD0;
		case K_KP_1:
			return KI_NUMPAD1;
		case K_KP_2:
			return KI_NUMPAD2;
		case K_KP_3:
			return KI_NUMPAD3;
		case K_KP_4:
			return KI_NUMPAD4;
		case K_KP_5:
			return KI_NUMPAD5;
		case K_KP_6:
			return KI_NUMPAD6;
		case K_KP_7:
			return KI_NUMPAD7;
		case K_KP_8:
			return KI_NUMPAD8;
		case K_KP_9:
			return KI_NUMPAD9;
		case K_KP_ENTER:
			return KI_NUMPADENTER;
		case K_KP_STAR:
			return KI_MULTIPLY;
		case K_KP_PLUS:
			return KI_ADD;
		case K_KP_MINUS:
			return KI_SUBTRACT;
		case K_KP_DOT:
			return KI_DECIMAL;
		case K_KP_SLASH:
			return KI_DIVIDE;
		case K_KP_EQUALS:
			return KI_OEM_NEC_EQUAL;

		// Function keys
		case K_F1:
			return KI_F1;
		case K_F2:
			return KI_F2;
		case K_F3:
			return KI_F3;
		case K_F4:
			return KI_F4;
		case K_F5:
			return KI_F5;
		case K_F6:
			return KI_F6;
		case K_F7:
			return KI_F7;
		case K_F8:
			return KI_F8;
		case K_F9:
			return KI_F9;
		case K_F10:
			return KI_F10;
		case K_F11:
			return KI_F11;
		case K_F12:
			return KI_F12;
		case K_F13:
			return KI_F13;
		case K_F14:
			return KI_F14;
		case K_F15:
			return KI_F15;

		// Navigation keys
		case K_UPARROW:
			return KI_UP;
		case K_DOWNARROW:
			return KI_DOWN;
		case K_LEFTARROW:
			return KI_LEFT;
		case K_RIGHTARROW:
			return KI_RIGHT;
		case K_HOME:
			return KI_HOME;
		case K_END:
			return KI_END;
		case K_PGUP:
			return KI_PRIOR;
		case K_PGDN:
			return KI_NEXT;
		case K_INS:
			return KI_INSERT;
		case K_DEL:
			return KI_DELETE;

		// Control keys
		case K_ESCAPE:
			return KI_ESCAPE;
		case K_ENTER:
			return KI_RETURN;
		case K_TAB:
			return KI_TAB;
		case K_BACKSPACE:
			return KI_BACK;
		case K_SPACE:
			return KI_SPACE;
		case K_PAUSE:
			return KI_PAUSE;
		case K_CAPSLOCK:
			return KI_CAPITAL;
		case K_NUMLOCK:
			return KI_NUMLOCK;
		case K_SCROLL:
			return KI_SCROLL;
		case K_PRINTSCREEN:
			return KI_SNAPSHOT;

		// Modifier keys
		case K_LSHIFT:
			return KI_LSHIFT;
		case K_RSHIFT:
			return KI_RSHIFT;
		case K_LCTRL:
			return KI_LCONTROL;
		case K_RCTRL:
			return KI_RCONTROL;
		case K_LALT:
			return KI_LMENU;
		case K_RALT:
			return KI_RMENU;
		case K_LWIN:
			return KI_LWIN;
		case K_RWIN:
			return KI_RWIN;
		case K_APPS:
			return KI_APPS;

		// Punctuation
		case K_SEMICOLON:
			return KI_OEM_1;
		case K_EQUALS:
			return KI_OEM_PLUS;
		case K_COMMA:
			return KI_OEM_COMMA;
		case K_MINUS:
			return KI_OEM_MINUS;
		case K_PERIOD:
			return KI_OEM_PERIOD;
		case K_SLASH:
			return KI_OEM_2;
		case K_GRAVE:
			return KI_OEM_3;
		case K_LBRACKET:
			return KI_OEM_4;
		case K_BACKSLASH:
			return KI_OEM_5;
		case K_RBRACKET:
			return KI_OEM_6;
		case K_APOSTROPHE:
			return KI_OEM_7;
		case K_OEM_102:
			return KI_OEM_102;

		// Power/Sleep
		case K_POWER:
			return KI_POWER;
		case K_SLEEP:
			return KI_SLEEP;
		case K_WAKE:
			return KI_WAKE;

		default:
			return KI_UNKNOWN;
	}
}

/*
========================
idRmlInputAdapter::GetKeyModifiers

Get the current state of modifier keys.
========================
*/
int idRmlInputAdapter::GetKeyModifiers() const
{
	int modifiers = 0;

	if( usercmdGen->KeyState( K_LCTRL ) == 1 || usercmdGen->KeyState( K_RCTRL ) == 1 )
	{
		modifiers |= Rml::Input::KM_CTRL;
	}

	if( usercmdGen->KeyState( K_LSHIFT ) == 1 || usercmdGen->KeyState( K_RSHIFT ) == 1 )
	{
		modifiers |= Rml::Input::KM_SHIFT;
	}

	if( usercmdGen->KeyState( K_LALT ) == 1 || usercmdGen->KeyState( K_RALT ) == 1 )
	{
		modifiers |= Rml::Input::KM_ALT;
	}

	if( usercmdGen->KeyState( K_LWIN ) == 1 || usercmdGen->KeyState( K_RWIN ) == 1 )
	{
		modifiers |= Rml::Input::KM_META;
	}

	return modifiers;
}

/*
========================
idRmlInputAdapter::UpdateMousePosition

Update the stored mouse position.
========================
*/
void idRmlInputAdapter::UpdateMousePosition( int x, int y )
{
	mouseX = x;
	mouseY = y;
}

/*
========================
idRmlInputAdapter::InjectSysEvent

Inject a Doom 3 sys event into the RmlUI context.
Returns true if the event was consumed.
========================
*/
bool idRmlInputAdapter::InjectSysEvent( Rml::Context* context, const sysEvent_t* event )
{
	if( context == nullptr || event == nullptr )
	{
		return false;
	}

	switch( event->evType )
	{
		case SE_KEY:
		{
			keyNum_t keyNum = static_cast<keyNum_t>( event->evValue );
			bool pressed = event->evValue2 > 0;

			// Handle mouse buttons
			if( keyNum >= K_MOUSE1 && keyNum <= K_MOUSE5 )
			{
				int button = keyNum - K_MOUSE1;
				common->DPrintf( "RmlUI: Mouse button %d %s at (%d, %d)\n",
								 button, pressed ? "down" : "up", mouseX, mouseY );
				if( pressed )
				{
					return context->ProcessMouseButtonDown( button, GetKeyModifiers() );
				}
				else
				{
					return context->ProcessMouseButtonUp( button, GetKeyModifiers() );
				}
			}
			// Handle keyboard keys
			else if( keyNum < K_JOY1 )
			{
				Rml::Input::KeyIdentifier rmlKey = TranslateKey( keyNum );
				if( rmlKey != Rml::Input::KI_UNKNOWN )
				{
					if( pressed )
					{
						return context->ProcessKeyDown( rmlKey, GetKeyModifiers() );
					}
					else
					{
						return context->ProcessKeyUp( rmlKey, GetKeyModifiers() );
					}
				}
			}
			break;
		}

		case SE_CHAR:
		{
			// Unicode character input
			if( event->evValue > 0 && event->evValue < 0x10000 )
			{
				return context->ProcessTextInput( static_cast<Rml::Character>( event->evValue ) );
			}
			break;
		}

		case SE_MOUSE_ABSOLUTE:
		{
			// Absolute mouse position
			mouseX = event->evValue;
			mouseY = event->evValue2;
			return context->ProcessMouseMove( mouseX, mouseY, GetKeyModifiers() );
		}

		case SE_MOUSE:
		{
			// Relative mouse movement
			mouseX += event->evValue;
			mouseY += event->evValue2;

			// Clamp to display bounds
			if( mouseX < 0 )
			{
				mouseX = 0;
			}
			if( mouseX >= displayWidth )
			{
				mouseX = displayWidth - 1;
			}
			if( mouseY < 0 )
			{
				mouseY = 0;
			}
			if( mouseY >= displayHeight )
			{
				mouseY = displayHeight - 1;
			}

			return context->ProcessMouseMove( mouseX, mouseY, GetKeyModifiers() );
		}

		case SE_MOUSE_LEAVE:
		{
			// Mouse left the window
			mouseX = -1;
			mouseY = -1;
			return context->ProcessMouseLeave();
		}

		default:
			break;
	}

	return false;
}

/*
========================
idRmlInputAdapter::InjectMouseWheel

Inject mouse wheel delta into the RmlUI context.
========================
*/
bool idRmlInputAdapter::InjectMouseWheel( Rml::Context* context, int delta )
{
	if( context == nullptr || delta == 0 )
	{
		return false;
	}

	// RmlUI expects wheel delta as a Vector2f (x for horizontal, y for vertical)
	return context->ProcessMouseWheel( Rml::Vector2f( 0.0f, static_cast<float>( delta ) ), GetKeyModifiers() );
}
