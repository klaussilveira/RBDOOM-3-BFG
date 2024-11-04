/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2016 Leyland Needham

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

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __VR_SYSTEM_H
#define __VR_SYSTEM_H

/*
=============================================================

VR

=============================================================
*/

const sysEvent_t& VR_UIEventNext();

void VR_ResetPose();
void VR_LogDevices();

void VR_UpdateResolution();
void VR_UpdateScaling();
void VR_UpdateControllers();

int VR_PollGameInputEvents();
int VR_ReturnGameInputEvent( const int n, int& action, int& value );

void VR_PreSwap( GLuint left, GLuint right );
void VR_PostSwap();

bool VR_GetHead( idVec3& origin, idMat3& axis );
bool VR_GetLeftController( idVec3& origin, idMat3& axis );
bool VR_GetRightController( idVec3& origin, idMat3& axis );
void VR_MoveDelta( idVec3& delta, float& height );
void VR_HapticPulse( int leftDuration, int rightDuration );
bool VR_GetLeftControllerAxis( idVec2& axis );
bool VR_GetRightControllerAxis( idVec2& axis );
bool VR_LeftControllerWasPressed();
bool VR_LeftControllerIsPressed();
bool VR_RightControllerWasPressed();
bool VR_RightControllerIsPressed();

const idVec3& VR_GetSeatedOrigin();
const idMat3& VR_GetSeatedAxis();
const idMat3& VR_GetSeatedAxisInverse();


#endif // __VR_SYSTEM_H
