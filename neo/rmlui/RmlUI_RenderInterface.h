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

#ifndef NEO_RMLUI_RMLUI_RENDERINTERFACE_H_
#define NEO_RMLUI_RMLUI_RENDERINTERFACE_H_

#include <RmlUi/Core/RenderInterface.h>
#include <unordered_map>

/*
================================================================================
idRmlRenderInterface

Implements the RmlUi RenderInterface using Doom 3's rendering system.
Renders geometry through idGuiModel::AllocTris(), following the same
pattern as the ImGui integration.

Texture handles are actually material pointers (const idMaterial*),
which allows direct use with the guiModel rendering pipeline.
================================================================================
*/

// Compiled geometry data stored for later rendering
struct RmlCompiledGeometry
{
	idList<idDrawVert>	vertices;
	idList<triIndex_t>	indices;
};

// Pending texture data for deferred upload
struct RmlPendingTexture
{
	idStr				name;
	idList<byte>		pixels;
	int					width;
	int					height;
	idImage*			image;
	const idMaterial*	material;
};

class idRmlRenderInterface : public Rml::RenderInterface
{
public:
	idRmlRenderInterface();
	virtual ~idRmlRenderInterface();

	// Set the display dimensions for coordinate scaling
	void SetDisplaySize( int width, int height );

	//
	// Required RenderInterface methods
	//

	// Compile geometry for later rendering
	Rml::CompiledGeometryHandle CompileGeometry( Rml::Span<const Rml::Vertex> vertices,
			Rml::Span<const int> indices ) override;

	// Render compiled geometry
	void RenderGeometry( Rml::CompiledGeometryHandle geometry,
						 Rml::Vector2f translation,
						 Rml::TextureHandle texture ) override;

	// Release compiled geometry
	void ReleaseGeometry( Rml::CompiledGeometryHandle geometry ) override;

	// Load a texture from file
	Rml::TextureHandle LoadTexture( Rml::Vector2i& texture_dimensions,
									const Rml::String& source ) override;

	// Generate a texture from pixel data (used for fonts)
	Rml::TextureHandle GenerateTexture( Rml::Span<const Rml::byte> source,
										Rml::Vector2i source_dimensions ) override;

	// Release a texture
	void ReleaseTexture( Rml::TextureHandle texture ) override;

	// Enable/disable scissor region
	void EnableScissorRegion( bool enable ) override;

	// Set scissor region
	void SetScissorRegion( Rml::Rectanglei region ) override;

private:
	// Get the default material for RmlUI rendering
	const idMaterial* GetDefaultMaterial() const;

	// Convert RmlUI color to Doom 3 color
	static uint32 ConvertColor( Rml::ColourbPremultiplied color );

	// Geometry cache
	std::unordered_map<Rml::CompiledGeometryHandle, RmlCompiledGeometry*> geometryCache;
	Rml::CompiledGeometryHandle nextGeometryHandle;

	// Texture cache (maps handles to materials for tracking)
	// Note: TextureHandle is actually a material pointer (const idMaterial*)
	std::unordered_map<Rml::TextureHandle, const idMaterial*> textureCache;

	// Generated texture counter for unique naming
	int generatedTextureCount;

	// Display dimensions
	int displayWidth;
	int displayHeight;

	// Scissor state
	bool scissorEnabled;
	idScreenRect scissorRect;

	// Pending textures that need to be uploaded when command list is available
	idList<RmlPendingTexture*> pendingTextures;

	// Upload any pending textures (called during rendering when command list is valid)
	void UploadPendingTextures();
};

#endif /* NEO_RMLUI_RMLUI_RENDERINTERFACE_H_ */
