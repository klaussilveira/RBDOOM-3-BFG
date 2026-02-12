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

#include "RmlUI_FileInterface.h"

/*
========================
idRmlFileInterface::idRmlFileInterface
========================
*/
idRmlFileInterface::idRmlFileInterface()
{
}

/*
========================
idRmlFileInterface::~idRmlFileInterface
========================
*/
idRmlFileInterface::~idRmlFileInterface()
{
}

/*
========================
idRmlFileInterface::Open

Opens a file for reading using Doom 3's file system.
========================
*/
Rml::FileHandle idRmlFileInterface::Open( const Rml::String& path )
{
	common->Printf( "RmlUI FileInterface: Opening '%s'\n", path.c_str() );

	idFile* file = fileSystem->OpenFileRead( path.c_str() );
	if( file == nullptr )
	{
		common->Warning( "RmlUI: Failed to open file '%s'", path.c_str() );
		return 0;
	}

	common->Printf( "RmlUI FileInterface: Successfully opened '%s' (%d bytes)\n", path.c_str(), file->Length() );
	return reinterpret_cast<Rml::FileHandle>( file );
}

/*
========================
idRmlFileInterface::Close

Closes a previously opened file.
========================
*/
void idRmlFileInterface::Close( Rml::FileHandle file )
{
	if( file != 0 )
	{
		idFile* f = reinterpret_cast<idFile*>( file );
		fileSystem->CloseFile( f );
	}
}

/*
========================
idRmlFileInterface::Read

Reads data from a file into a buffer.
========================
*/
size_t idRmlFileInterface::Read( void* buffer, size_t size, Rml::FileHandle file )
{
	if( file == 0 )
	{
		return 0;
	}

	idFile* f = reinterpret_cast<idFile*>( file );
	int bytesRead = f->Read( buffer, static_cast<int>( size ) );

	return bytesRead >= 0 ? static_cast<size_t>( bytesRead ) : 0;
}

/*
========================
idRmlFileInterface::Seek

Seeks to a position in a file.
========================
*/
bool idRmlFileInterface::Seek( Rml::FileHandle file, long offset, int origin )
{
	if( file == 0 )
	{
		return false;
	}

	idFile* f = reinterpret_cast<idFile*>( file );

	// Map standard C seek origins to Doom 3's fsOrigin_t
	fsOrigin_t fsOrigin;
	switch( origin )
	{
		case SEEK_SET:
			fsOrigin = FS_SEEK_SET;
			break;
		case SEEK_CUR:
			fsOrigin = FS_SEEK_CUR;
			break;
		case SEEK_END:
			fsOrigin = FS_SEEK_END;
			break;
		default:
			return false;
	}

	return f->Seek( offset, fsOrigin ) == 0;
}

/*
========================
idRmlFileInterface::Tell

Returns the current position in a file.
========================
*/
size_t idRmlFileInterface::Tell( Rml::FileHandle file )
{
	if( file == 0 )
	{
		return 0;
	}

	idFile* f = reinterpret_cast<idFile*>( file );
	int position = f->Tell();

	return position >= 0 ? static_cast<size_t>( position ) : 0;
}

/*
========================
idRmlFileInterface::Length

Returns the length of a file.
========================
*/
size_t idRmlFileInterface::Length( Rml::FileHandle file )
{
	if( file == 0 )
	{
		return 0;
	}

	idFile* f = reinterpret_cast<idFile*>( file );
	int length = f->Length();

	return length >= 0 ? static_cast<size_t>( length ) : 0;
}
