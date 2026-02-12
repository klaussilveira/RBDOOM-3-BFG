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

#include "RmlUI_RenderInterface.h"
#include "renderer/RenderCommon.h"
#include "renderer/Image.h"
#include "sys/DeviceManager.h"

extern DeviceManager* deviceManager;

/*
========================
idRmlRenderInterface::idRmlRenderInterface
========================
*/
idRmlRenderInterface::idRmlRenderInterface()
	: nextGeometryHandle( 1 )
	, generatedTextureCount( 0 )
	, displayWidth( 640 )
	, displayHeight( 480 )
	, scissorEnabled( false )
{
	scissorRect.Clear();
}

/*
========================
idRmlRenderInterface::~idRmlRenderInterface
========================
*/
idRmlRenderInterface::~idRmlRenderInterface()
{
	// Clean up any remaining geometry
	for( auto& pair : geometryCache )
	{
		delete pair.second;
	}
	geometryCache.clear();

	// Clean up any pending textures that weren't uploaded
	for( int i = 0; i < pendingTextures.Num(); i++ )
	{
		delete pendingTextures[i];
	}
	pendingTextures.Clear();

	// Note: We don't delete images/materials as Doom 3 manages their lifetime
	textureCache.clear();
}

/*
========================
idRmlRenderInterface::SetDisplaySize
========================
*/
void idRmlRenderInterface::SetDisplaySize( int width, int height )
{
	displayWidth = width;
	displayHeight = height;
}

/*
========================
idRmlRenderInterface::GetDefaultMaterial
========================
*/
const idMaterial* idRmlRenderInterface::GetDefaultMaterial() const
{
	return declManager->FindMaterial( "_white" );
}

/*
========================
idRmlRenderInterface::ConvertColor

Convert RmlUI premultiplied RGBA color to Doom 3 packed color.
========================
*/
uint32 idRmlRenderInterface::ConvertColor( Rml::ColourbPremultiplied color )
{
	// RmlUI uses premultiplied alpha in RGBA order
	// Doom 3 expects packed color in RGBA order as well
	return ( ( uint32 )color.red ) |
		   ( ( uint32 )color.green << 8 ) |
		   ( ( uint32 )color.blue << 16 ) |
		   ( ( uint32 )color.alpha << 24 );
}

/*
========================
idRmlRenderInterface::CompileGeometry

Compile geometry by storing vertices and indices for later rendering.
========================
*/
Rml::CompiledGeometryHandle idRmlRenderInterface::CompileGeometry(
	Rml::Span<const Rml::Vertex> vertices,
	Rml::Span<const int> indices )
{
	RmlCompiledGeometry* geometry = new RmlCompiledGeometry();

	// Convert and store vertices
	geometry->vertices.SetNum( vertices.size() );
	for( size_t i = 0; i < vertices.size(); i++ )
	{
		const Rml::Vertex& src = vertices[i];
		idDrawVert& dst = geometry->vertices[i];

		dst.xyz.x = src.position.x;
		dst.xyz.y = src.position.y;
		dst.xyz.z = 0.0f;

		dst.SetTexCoord( src.tex_coord.x, src.tex_coord.y );
		dst.SetColor( ConvertColor( src.colour ) );
	}

	// Store indices (convert from int to triIndex_t)
	geometry->indices.SetNum( indices.size() );
	for( size_t i = 0; i < indices.size(); i++ )
	{
		geometry->indices[i] = static_cast<triIndex_t>( indices[i] );
	}

	Rml::CompiledGeometryHandle handle = nextGeometryHandle++;
	geometryCache[handle] = geometry;

	return handle;
}

/*
========================
idRmlRenderInterface::RenderGeometry

Render compiled geometry using Doom 3's GUI model.
Following the same pattern as ImGui integration.
========================
*/
void idRmlRenderInterface::RenderGeometry(
	Rml::CompiledGeometryHandle geometryHandle,
	Rml::Vector2f translation,
	Rml::TextureHandle textureHandle )
{
	// Upload any pending textures now that we're in the render phase
	UploadPendingTextures();

	auto it = geometryCache.find( geometryHandle );
	if( it == geometryCache.end() )
	{
		common->Warning( "RmlUI: Geometry handle %d not found in cache", (int)geometryHandle );
		return;
	}

	RmlCompiledGeometry* geometry = it->second;
	if( geometry->vertices.Num() == 0 || geometry->indices.Num() == 0 )
	{
		return;
	}

	// Get the material for this texture
	// TextureHandle is actually a material pointer (same pattern as ImGui)
	const idMaterial* material = nullptr;
	if( textureHandle != 0 )
	{
		material = reinterpret_cast<const idMaterial*>( textureHandle );
	}
	else
	{
		material = GetDefaultMaterial();
	}

	// Calculate scale from display to virtual resolution
	const float sysWidth = static_cast<float>( renderSystem->GetWidth() );
	const float sysHeight = static_cast<float>( renderSystem->GetHeight() );
	const float virtualWidth = static_cast<float>( renderSystem->GetVirtualWidth() );
	const float virtualHeight = static_cast<float>( renderSystem->GetVirtualHeight() );

	idVec2 scaleToVirtual( virtualWidth / sysWidth, virtualHeight / sysHeight );

	// Prepare scissor rect if enabled
	// Convert from RmlUI coordinates (top-left origin) to Doom 3 coordinates (bottom-left origin)
	idScreenRect clipRect;
	if( scissorEnabled )
	{
		clipRect.x1 = static_cast<short>( scissorRect.x1 );
		clipRect.y1 = static_cast<short>( displayHeight - scissorRect.y2 );
		clipRect.x2 = static_cast<short>( scissorRect.x2 );
		clipRect.y2 = static_cast<short>( displayHeight - scissorRect.y1 );
		clipRect.zmin = 0.0f;
		clipRect.zmax = 1.0f;
	}
	else
	{
		clipRect.Clear();
	}

	// Allocate triangles in the GUI model (same as ImGui does)
	idDrawVert* verts = tr.guiModel->AllocTris(
							geometry->vertices.Num(),
							geometry->indices.Ptr(),
							geometry->indices.Num(),
							material,
							tr.currentGLState,
							STEREO_DEPTH_TYPE_NONE,
							clipRect );

	if( verts == nullptr )
	{
		return;
	}

	// Copy vertices with translation applied
	for( int i = 0; i < geometry->vertices.Num(); i++ )
	{
		const idDrawVert& src = geometry->vertices[i];

		ALIGNTYPE16 idDrawVert tempVert;

		// Apply translation and scale to virtual coordinates
		idVec2 pos( src.xyz.x + translation.x, src.xyz.y + translation.y );
		tempVert.xyz.ToVec2() = pos.Scale( scaleToVirtual );
		tempVert.xyz.z = 0.0f;

		tempVert.SetTexCoord( src.GetTexCoordS(), src.GetTexCoordT() );
		tempVert.SetColor( src.GetColor() );

		WriteDrawVerts16( &verts[i], &tempVert, 1 );
	}
}

/*
========================
idRmlRenderInterface::ReleaseGeometry
========================
*/
void idRmlRenderInterface::ReleaseGeometry( Rml::CompiledGeometryHandle geometry )
{
	auto it = geometryCache.find( geometry );
	if( it != geometryCache.end() )
	{
		delete it->second;
		geometryCache.erase( it );
	}
}

/*
========================
idRmlRenderInterface::LoadTexture

Load a texture from file using Doom 3's material system.
========================
*/
Rml::TextureHandle idRmlRenderInterface::LoadTexture(
	Rml::Vector2i& texture_dimensions,
	const Rml::String& source )
{
	// Try to find the material
	const idMaterial* material = declManager->FindMaterial( source.c_str() );
	if( material == nullptr )
	{
		common->Warning( "RmlUI: Failed to load texture '%s'", source.c_str() );
		return 0;
	}

	// Get texture dimensions from the material's image
	if( material->GetNumStages() > 0 && material->GetStage( 0 )->texture.image != nullptr )
	{
		texture_dimensions.x = material->GetImageWidth();
		texture_dimensions.y = material->GetImageHeight();
	}
	else
	{
		// Fallback to default dimensions if we can't determine them
		texture_dimensions.x = 256;
		texture_dimensions.y = 256;
	}

	// Store in cache for tracking
	textureCache[reinterpret_cast<Rml::TextureHandle>( material )] = material;

	// Return the material pointer as the texture handle (same as ImGui)
	return reinterpret_cast<Rml::TextureHandle>( material );
}

/*
========================
idRmlRenderInterface::GenerateTexture

Generate a texture from raw pixel data (used for font atlases).
Pre-allocates the GPU texture and defers pixel upload until command list is available.
========================
*/
Rml::TextureHandle idRmlRenderInterface::GenerateTexture(
	Rml::Span<const Rml::byte> source,
	Rml::Vector2i source_dimensions )
{
	// Create a unique name for this generated texture
	idStr textureName;
	textureName.Format( "_rmlui_gen_%d", generatedTextureCount++ );

	// Allocate the image and add it to the image manager's hash
	idImage* image = globalImages->AllocImage( textureName.c_str() );
	if( image == nullptr )
	{
		common->Warning( "RmlUI: Failed to allocate image for generated texture" );
		return 0;
	}

	// Pre-allocate the GPU texture with the correct dimensions
	// This creates the nvrhi texture without requiring a command list
	idImageOpts imgOpts;
	imgOpts.textureType = DTT_2D;
	imgOpts.format = FMT_RGBA8;
	imgOpts.colorFormat = CFM_DEFAULT;
	imgOpts.width = source_dimensions.x;
	imgOpts.height = source_dimensions.y;
	imgOpts.numLevels = 1;

	image->AllocImage( imgOpts, TF_LINEAR, TR_CLAMP );

	// Mark as referenced outside level load to prevent purging
	image->referencedOutsideLevelLoad = true;

	// Find or create an implicit material that references this image
	const idMaterial* material = declManager->FindMaterial( textureName.c_str(), true );
	if( material == nullptr )
	{
		common->Warning( "RmlUI: Failed to create material for '%s'", textureName.c_str() );
		return 0;
	}

	// Store pixel data for deferred upload
	// The actual upload will happen in UploadPendingTextures() when we create our own command list
	RmlPendingTexture* pending = new RmlPendingTexture();
	pending->name = textureName;
	pending->width = source_dimensions.x;
	pending->height = source_dimensions.y;
	pending->image = image;
	pending->material = material;

	// Copy the pixel data
	size_t dataSize = source_dimensions.x * source_dimensions.y * 4; // RGBA
	pending->pixels.SetNum( dataSize );
	memcpy( pending->pixels.Ptr(), source.data(), dataSize );

	pendingTextures.Append( pending );

	// Store in cache for tracking
	textureCache[reinterpret_cast<Rml::TextureHandle>( material )] = material;

	// Return the material pointer as the texture handle (same as ImGui)
	return reinterpret_cast<Rml::TextureHandle>( material );
}

/*
========================
idRmlRenderInterface::ReleaseTexture
========================
*/
void idRmlRenderInterface::ReleaseTexture( Rml::TextureHandle texture )
{
	// Note: We don't actually delete the material/image as Doom 3 manages their lifetime
	textureCache.erase( texture );
}

/*
========================
idRmlRenderInterface::UploadPendingTextures

Upload any textures that were deferred because the command list wasn't available.
Creates its own temporary command list for the upload, similar to LoadDeferredImages.
========================
*/
void idRmlRenderInterface::UploadPendingTextures()
{
	if( pendingTextures.Num() == 0 )
	{
		return;
	}

	// Check if deviceManager is available
	if( deviceManager == nullptr || deviceManager->GetDevice() == nullptr )
	{
		return;
	}

	// Create our own temporary command list for texture uploads
	// This follows the pattern used in LoadDeferredImages and CommonPasses
	nvrhi::CommandListHandle commandList = deviceManager->GetDevice()->createCommandList();
	commandList->open();

	for( int i = 0; i < pendingTextures.Num(); i++ )
	{
		RmlPendingTexture* pending = pendingTextures[i];

		// Get the nvrhi texture handle from the image
		nvrhi::TextureHandle texture = pending->image->GetTextureHandle();
		if( !texture )
		{
			common->Warning( "RmlUI: Texture handle is null for '%s'", pending->name.c_str() );
			delete pending;
			continue;
		}

		// Write the pixel data to the texture
		// Row pitch for RGBA8 is width * 4 bytes per pixel
		int rowPitch = pending->width * 4;

		commandList->beginTrackingTextureState( texture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->writeTexture( texture, 0, 0, pending->pixels.Ptr(), rowPitch );
		commandList->setPermanentTextureState( texture, nvrhi::ResourceStates::ShaderResource );
		commandList->commitBarriers();

		// Mark the image as loaded so it's recognized as valid by the rendering system
		pending->image->isLoaded = true;

		// Free the pending texture data
		delete pending;
	}

	pendingTextures.Clear();

	// Close and execute the command list
	commandList->close();
	deviceManager->GetDevice()->executeCommandList( commandList );
}

/*
========================
idRmlRenderInterface::EnableScissorRegion
========================
*/
void idRmlRenderInterface::EnableScissorRegion( bool enable )
{
	scissorEnabled = enable;
}

/*
========================
idRmlRenderInterface::SetScissorRegion
========================
*/
void idRmlRenderInterface::SetScissorRegion( Rml::Rectanglei region )
{
	scissorRect.x1 = static_cast<short>( region.Left() );
	scissorRect.y1 = static_cast<short>( region.Top() );
	scissorRect.x2 = static_cast<short>( region.Right() );
	scissorRect.y2 = static_cast<short>( region.Bottom() );
	scissorRect.zmin = 0.0f;
	scissorRect.zmax = 1.0f;
}
