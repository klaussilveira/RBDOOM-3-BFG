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

#ifndef NEO_RMLUI_RMLUI_INPUTADAPTER_H_
#define NEO_RMLUI_RMLUI_INPUTADAPTER_H_

#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Context.h>

/*
================================================================================
idRmlInputAdapter

Translates Doom 3's sysEvent_t input events to RmlUI input events.
Handles keyboard, mouse, and character input.
================================================================================
*/
class idRmlInputAdapter
{
public:
	idRmlInputAdapter();

	// Inject a sys event into the RmlUI context
	// Returns true if the event was consumed
	bool InjectSysEvent( Rml::Context* context, const sysEvent_t* event );

	// Inject mouse wheel delta
	bool InjectMouseWheel( Rml::Context* context, int delta );

	// Update mouse position (called each frame if needed)
	void UpdateMousePosition( int x, int y );

	// Set display dimensions for coordinate clamping
	void SetDisplaySize( int width, int height );

	// Get current mouse position
	int GetMouseX() const
	{
		return mouseX;
	}
	int GetMouseY() const
	{
		return mouseY;
	}

private:
	// Translate Doom 3 key code to RmlUI key identifier
	Rml::Input::KeyIdentifier TranslateKey( keyNum_t key ) const;

	// Get current key modifiers state
	int GetKeyModifiers() const;

	// Current mouse position
	int mouseX;
	int mouseY;

	// Display dimensions for clamping
	int displayWidth;
	int displayHeight;
};

#endif /* NEO_RMLUI_RMLUI_INPUTADAPTER_H_ */
