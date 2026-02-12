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

#include "RmlUI_SystemInterface.h"

/*
========================
idRmlSystemInterface::idRmlSystemInterface
========================
*/
idRmlSystemInterface::idRmlSystemInterface()
{
}

/*
========================
idRmlSystemInterface::~idRmlSystemInterface
========================
*/
idRmlSystemInterface::~idRmlSystemInterface()
{
}

/*
========================
idRmlSystemInterface::GetElapsedTime

Returns elapsed time in seconds since application start.
========================
*/
double idRmlSystemInterface::GetElapsedTime()
{
	return Sys_Milliseconds() * 0.001;
}

/*
========================
idRmlSystemInterface::LogMessage

Log messages using Doom 3's logging system.
========================
*/
bool idRmlSystemInterface::LogMessage( Rml::Log::Type type, const Rml::String& message )
{
	switch( type )
	{
		case Rml::Log::LT_ERROR:
			common->Warning( "RmlUI Error: %s", message.c_str() );
			break;

		case Rml::Log::LT_ASSERT:
			common->Warning( "RmlUI Assert: %s", message.c_str() );
			break;

		case Rml::Log::LT_WARNING:
			common->Warning( "RmlUI: %s", message.c_str() );
			break;

		case Rml::Log::LT_INFO:
			common->Printf( "RmlUI: %s\n", message.c_str() );
			break;

		case Rml::Log::LT_DEBUG:
		case Rml::Log::LT_ALWAYS:
		default:
			common->DPrintf( "RmlUI: %s\n", message.c_str() );
			break;
	}

	return true;
}

/*
========================
idRmlSystemInterface::SetClipboardText

Set clipboard text using Doom 3's clipboard functions.
========================
*/
void idRmlSystemInterface::SetClipboardText( const Rml::String& text )
{
	Sys_SetClipboardData( text.c_str() );
}

/*
========================
idRmlSystemInterface::GetClipboardText

Get clipboard text using Doom 3's clipboard functions.
========================
*/
void idRmlSystemInterface::GetClipboardText( Rml::String& text )
{
	char* clipboardData = Sys_GetClipboardData();
	if( clipboardData != nullptr )
	{
		text = clipboardData;
		Mem_Free( clipboardData );
	}
	else
	{
		text.clear();
	}
}

/*
========================
idRmlSystemInterface::SetMouseCursor

Set mouse cursor - currently a no-op as we use Doom 3's cursor handling.
========================
*/
void idRmlSystemInterface::SetMouseCursor( const Rml::String& cursor_name )
{
	// TODO: Implement cursor switching if needed
	// For now, we rely on Doom 3's existing cursor handling
}
