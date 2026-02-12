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

#ifndef NEO_RMLUI_RMLUI_FILEINTERFACE_H_
#define NEO_RMLUI_RMLUI_FILEINTERFACE_H_

#include <RmlUi/Core/FileInterface.h>

/*
================================================================================
idRmlFileInterface

Implements the RmlUi FileInterface using Doom 3's idFileSystem.
All file paths are relative to the game's base directory.
================================================================================
*/
class idRmlFileInterface : public Rml::FileInterface
{
public:
	idRmlFileInterface();
	virtual ~idRmlFileInterface();

	// Open a file for reading
	Rml::FileHandle Open( const Rml::String& path ) override;

	// Close an opened file
	void Close( Rml::FileHandle file ) override;

	// Read data from a file
	size_t Read( void* buffer, size_t size, Rml::FileHandle file ) override;

	// Seek to a position in a file
	bool Seek( Rml::FileHandle file, long offset, int origin ) override;

	// Get the current position in a file
	size_t Tell( Rml::FileHandle file ) override;

	// Get the length of a file
	size_t Length( Rml::FileHandle file ) override;
};

#endif /* NEO_RMLUI_RMLUI_FILEINTERFACE_H_ */
