// HEXEN : Zeroth

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

void idPlayer::Event_SetViewAngles( idVec3& vec )
{
	viewAngles[0] = vec.x;
	viewAngles[1] = vec.y;
	viewAngles[2] = vec.z;
}

void idPlayer::Event_GetEyePos()
{
	idThread::ReturnVector( GetEyePosition() );
}

void idPlayer::Event_SetFreeMove( const float yesorno )
{
	//spawnArgs.SetInt("freemovement", yesorno);
	if( yesorno )
	{
		FreeMove = true;
	}
	else
	{
		FreeMove = false;
	}
}

void idPlayer::Event_ChaosDevice()
{
	SetOrigin( SpawnPos );
	SetViewAngles( SpawnViewAngles );
}

void idPlayer::Event_GiveHealth( const float amount )
{
	health += amount;
}

void idPlayer::Event_GiveSpeed( const float amount )
{
	speedMod += amount;
}

void idPlayer::Event_GiveArmor( const float amount )
{
	inventory.armor += amount;
}

void idPlayer::Event_SetHealth( const float amount )
{
	health = amount;
}

void idPlayer::Event_SetArmor( const float amount )
{
	inventory.armor = amount;
}

void idPlayer::Event_SetInvincible( const float number )
{
	if( number != 0 )
	{
		invincible = true;
	}
	else
	{
		invincible = false;
	}
}

void idPlayer::Event_GetInvincible()
{
	if( godmode || invincible )
	{
		idThread::ReturnFloat( 1 );
	}
	else
	{
		idThread::ReturnFloat( 0 );
	}
}

void idPlayer::Event_GetClass()
{
	idThread::ReturnFloat( inventory.Class );
}

void idPlayer::Event_GetHealth()
{
	idThread::ReturnFloat( health );
}

void idPlayer::Event_GetFullHealth()
{
	idThread::ReturnFloat( inventory.maxHealth );
}

void idPlayer::Event_GetFullArmor()
{
	idThread::ReturnFloat( inventory.maxarmor );
}

void idPlayer::Event_GetFullAmmo( const char* ammo_classname )
{
	idThread::ReturnFloat( spawnArgs.GetInt( va( "max_%s", ammo_classname ), "0" ) );
}

void idPlayer::Event_GetAmmo( const char* ammo_classname )
{
	idThread::ReturnFloat( inventory.GetAmmo( idWeapon::GetAmmoNumForName( ammo_classname ) ) );
}

void idPlayer::Event_SetAmmo( const char* ammo_classname, float amount )
{
	if( ammo_classname )
	{
		inventory.SetAmmo( idWeapon::GetAmmoNumForName( ammo_classname ), amount );
	}
}

void idPlayer::Event_GetArmor()
{
	idThread::ReturnFloat( inventory.armor );
}

void idPlayer::Event_StickToSurface( const idVec3& surfaceNormal )
{
	//idPhysics_Actor *x=GetPhysics();
	physicsObj.SetSurfaceNormal( surfaceNormal );
	physicsObj.SetStuckToSurface( true );
}

void idPlayer::Event_UnstickToSurface()
{
	physicsObj.SetStuckToSurface( false );
}

bool idPlayer::StuckToSurface()
{
	return physicsObj.StuckToSurface();
}

void idPlayer::Event_StuckToSurface()
{
	idThread::ReturnFloat( ( float )StuckToSurface() );
}

void idPlayer::Event_SetPowerTome( float val )
{
	PowerTome = !( !val );
}

void idPlayer::Event_GetPowerTome()
{
	idThread::ReturnFloat( ( float )PowerTome );
}

void idPlayer::Event_VecFacingP()
{
	// return proper facing direction (yaw only)
	idVec3 dir = viewAxis[ 0 ] * physicsObj.GetGravityAxis();
	dir.Normalize();
	idThread::ReturnVector( dir );

}

void idPlayer::Event_HudMessage( const char* message )
{
	ShowHudMessage( message );
}

void idPlayer::Event_VecForwardP()
{
	idThread::ReturnVector( VecForwardP() );
}

idVec3 idPlayer::VecForwardP() const
{
	// return proper forward vector for whatever way the player is looking
	idVec3 dir = viewAngles.ToForward() * physicsObj.GetGravityAxis();
	dir.Normalize();
	return dir;
}

// HEXEN : Zeroth
void idPlayer::Event_ArtifactUseFlash()
{
	idUserInterface* hudUI = ( idUserInterface* ) hud;
	if( hudUI )
	{
		hudUI->HandleNamedEvent( "eoc_artifactUse" );
	}
}
