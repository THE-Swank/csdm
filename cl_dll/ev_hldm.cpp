/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>
#include <windows.h>

#include "r_studioint.h"
#include "com_model.h"

extern "C" char PM_FindTextureType( char *name );

#define SND_CHANGE_PITCH	(1<<7)

extern engine_studio_api_t IEngineStudio;

static int tracerCount[ 32 ];

void V_PunchAxis( int axis, float punch );
void VectorAngles( const float *forward, float *angles );

extern cvar_t *cl_lw;

extern "C"
{

	// HLDM
	void EV_Knife( event_args_t *args );
	void EV_FireUSP( struct event_args_s *args );
	void EV_Fireglock18( struct event_args_s *args );
	void EV_FireM4A1( struct event_args_s *args );
	void EV_FireAK47( struct event_args_s *args );
	void EV_FireAWP( struct event_args_s *args );
	void EV_FireGALIL( struct event_args_s *args );
	void EV_FireFAMAS( struct event_args_s *args );
	void EV_FireDEAGLE( struct event_args_s *args );
	void EV_FireAUG( struct event_args_s *args );
	void EV_FireSG552( struct event_args_s *args );
	void EV_FireMP5( struct event_args_s *args );
	void EV_FireM3( struct event_args_s *args );
	void EV_TrainPitchAdjust( struct event_args_s *args );
}

#define VECTOR_CONE_1DEGREES Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES Vector( 0.06105, 0.06105, 0.06105 )	
#define VECTOR_CONE_8DEGREES Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES Vector( 0.17365, 0.17365, 0.17365 )

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
void EV_HLDM_WaterSplash( float x, float y, float z, float ScaleSplash1, float ScaleSplash2 )
{
	int  iWaterSplash = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/watersplash1.spr");
	TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z + 100 * ScaleSplash1 ),
		Vector( 0, 0, 0 ),
		ScaleSplash1, iWaterSplash, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL | FTENT_PERSIST );

	if(pTemp)
	{
		pTemp->fadeSpeed = 90.0;
		pTemp->entity.curstate.framerate = 100.0;
		pTemp->entity.curstate.renderamt = 155;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
	}

	iWaterSplash = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/watersplash2.spr");
	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z ),
		Vector( 0, 0, 0 ),
		ScaleSplash2, iWaterSplash, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL | FTENT_PERSIST );

	if(pTemp)
	{
		pTemp->fadeSpeed = 60.0;
		pTemp->entity.curstate.framerate = 50.0;
		pTemp->entity.curstate.renderamt = 100;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
		pTemp->entity.angles = Vector( 90, 0, 0 );
	}

	iWaterSplash = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/watersplash2.spr");
	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z ),
		Vector( 0, 0, 0 ),
		ScaleSplash2, iWaterSplash, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL | FTENT_PERSIST );

	if(pTemp)
	{
		pTemp->fadeSpeed = 60.0;
		pTemp->entity.curstate.framerate = 50.0;
		pTemp->entity.curstate.renderamt = 100;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
		pTemp->entity.angles = Vector( -90, 0, 0 );
	}
}

void EV_HLDM_SmokeGrenade( float x, float y, float z )
{
	for( int i = 1; i <= 50; i++ )
	{
		int  iSmokeSprite = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/smoke.spr");
		TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x + gEngfuncs.pfnRandomLong( -100, 100 ), y + gEngfuncs.pfnRandomLong( -100, 100 ), z + gEngfuncs.pfnRandomLong( 0, 200 ) ),
			Vector( gEngfuncs.pfnRandomLong( -10, 10 ), gEngfuncs.pfnRandomLong( -10, 10 ), gEngfuncs.pfnRandomLong( -10, 10 ) ),
			5, iSmokeSprite, kRenderTransAlpha, kRenderFxNone, 1.0, 0.5, FTENT_FADEOUT | FTENT_PERSIST | FTENT_COLLIDEWORLD );

		if(pTemp)
		{
			pTemp->fadeSpeed = 0.02;
			pTemp->entity.curstate.framerate = 1;
			pTemp->entity.curstate.renderamt = 255;
			int Color = gEngfuncs.pfnRandomLong( 0, 140 );
			pTemp->entity.curstate.rendercolor.r = Color;
			pTemp->entity.curstate.rendercolor.g = Color;
			pTemp->entity.curstate.rendercolor.b = Color;
		}
	}
}

void EV_HLDM_NewExplode( float x, float y, float z, float ScaleExplode1 )
{

	int  iNewExplode = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/dexplo.spr");
	TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z + 5 ),
		Vector( 0, 0, 0 ),
		ScaleExplode1, iNewExplode, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL );

	if(pTemp)
	{
		pTemp->fadeSpeed = 90.0;
		pTemp->entity.curstate.framerate = 37.0;
		pTemp->entity.curstate.renderamt = 155;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
	}

	iNewExplode = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/fexplo.spr");
	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z + 10),
		Vector( 0, 0, 0 ),
		ScaleExplode1, iNewExplode, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL );

	if(pTemp)
	{
		pTemp->fadeSpeed = 90.0;
		pTemp->entity.curstate.framerate = 35.0;
		pTemp->entity.curstate.renderamt = 150;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
		pTemp->entity.angles = Vector( 90, 0, 0 );
	}
	
	for( int i = 1; i <= 10; i++ )
	{
		int  iSmokeSprite = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/smoke.spr");
		TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z ),
			Vector( gEngfuncs.pfnRandomLong( -100, 100 ), gEngfuncs.pfnRandomLong( -100, 100 ), gEngfuncs.pfnRandomLong( -100, 100 ) ),
			5, iSmokeSprite, kRenderTransAlpha, kRenderFxNone, 1.0, 0.5, FTENT_FADEOUT | FTENT_PERSIST );

		if(pTemp)
		{
			pTemp->fadeSpeed = 0.6;
			pTemp->entity.curstate.framerate = 1;
			pTemp->entity.curstate.renderamt = 255;
			int Color = gEngfuncs.pfnRandomLong( 0, 140 );
			pTemp->entity.curstate.rendercolor.r = Color;
			pTemp->entity.curstate.rendercolor.g = Color;
			pTemp->entity.curstate.rendercolor.b = Color;
		}
	}

}

float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[ 64 ];
	char szbuffer[ 64 ];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if ( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if ( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );

		if ( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}

			// '}}'
			strcpy( szbuffer, pTextureName );
			szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;

			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );	
		}
	}

	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0,cnt-1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0,0xf) );

	return fvolbar;
}

char *EV_HLDM_DamageDecal( physent_t *pe )
{
	static char decalname[ 32 ];
	int idx;

	if ( pe->classnumber == 1 )
	{
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else if ( pe->rendermode != kRenderNormal )
	{
		sprintf( decalname, "{bproof1" );
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong( 0, 4 );
		sprintf( decalname, "{shot%i", idx + 1 );
	}
	return decalname;
}

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName )
{
	int iRand;
	physent_t *pe;

	gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );

	iRand = gEngfuncs.pfnRandomLong(0,0x7FFF);
	if ( iRand < (0x7fff/2) )// not every bullet makes a sound.
	{
		switch( iRand % 5)
		{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 4:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	// Only decal brush models such as the world etc.
	if (  decalName && decalName[0] && pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{
		if ( CVAR_GET_FLOAT( "r_decals" ) )
		{
			gEngfuncs.pEfxAPI->R_DecalShoot( 
				gEngfuncs.pEfxAPI->Draw_DecalIndex( gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalName ) ), 
				gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace ), 0, pTrace->endpos, 0 );

		}
	}
}

void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType )
{
	physent_t *pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if ( pe && pe->solid == SOLID_BSP )
		EV_HLDM_GunshotDecalTrace( pTrace, EV_HLDM_DamageDecal( pe ) );
}

int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount )
{
	int tracer = 0;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;

	if ( iTracerFreq != 0 && ( (*tracerCount)++ % iTracerFreq) == 0 )
	{
		vec3_t vecTracerSrc;

		if ( player )
		{
			vec3_t offset( 0, 0, -4 );

			// adjust tracer position for player
			for ( i = 0; i < 3; i++ )
			{
				vecTracerSrc[ i ] = vecSrc[ i ] + offset[ i ] + right[ i ] * 2 + forward[ i ] * 16;
			}
		}
		else
		{
			VectorCopy( vecSrc, vecTracerSrc );
		}

		if ( iTracerFreq != 1 )		// guns that always trace also always decal
			tracer = 1;


		EV_CreateTracer( vecTracerSrc, end );
	}

	return tracer;
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY )
{
	int i;
	pmtrace_t tr;
	int iShot;
	int tracer;

	for ( iShot = 1; iShot <= cShots; iShot++ )	
	{
		vec3_t vecDir, vecEnd;

		for ( i = 0; i < 3; i++ )
		{
			vecDir[i] = vecDirShooting[i] + flSpreadX * right[ i ] + flSpreadY * up [ i ];
			vecEnd[i] = vecSrc[ i ] + flDistance * vecDir[ i ];
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

		tracer = EV_HLDM_CheckTracer( idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount );

		// do damage, paint decals
		if ( tr.fraction != 1.0 )
		{
			//AddDecal( tr.endpos ,  tr.plane.normal, Vector( 0, 0, 0 ), "", 0 );
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

//======================
// Knife START
//======================

enum knife_e
{
	KNIFE_IDLE1 = 0,
	KNIFE_SLASH1,
	KNIFE_SLASH2,
	KNIFE_DRAW,
	KNIFE_STAB,
	KNIFE_STAB_MISS,
	KNIFE_MIDSLASH1,
	KNIFE_MIDSLASH2
};

void EV_Knife( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );

	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM); 
}
//======================
// Knife END
//======================
//======================
// USP START
//======================

enum usp_e 
{
	USP_IDLE = 0,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOTLAST,
	USP_RELOAD,
	USP_DRAW,
	USP_ADD_SILENCER,
	USP_IDLE_UNSIL,
	USP_SHOOT1_UNSIL,
	USP_SHOOT2_UNSIL,
	USP_SHOOT3_UNSIL,
	USP_SHOOTLAST_UNSIL,
	USP_RELOAD_UNSIL,
	USP_DRAW_UNSIL,
	USP_DETACH_SILENCER
};

void EV_FireUSP( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		gHUD.RealSize += 70;
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(USP_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);

		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( 0, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/pshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);

	/*gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
	"weapons/usp1.wav",
	1, ATTN_NORM, 0,
	94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// USP END
//======================

//======================
// glock18 START
//======================

enum glock18_e 
{
	GLOCK18_IDLE1 = 0,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT1,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,
	GLOCK18_DRAW,
	GLOCK18_HOLSTER,
	GLOCK18_ADD_SILENCER,
	GLOCK18_DRAW2,
	GLOCK18_RELOAD2
};

void EV_Fireglock18( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		gHUD.RealSize += 70;
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(GLOCK18_SHOOT1 + gEngfuncs.pfnRandomLong(0,2), 2);

		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/pshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
	"weapons/glock18-1.wav",
	1, ATTN_NORM, 0,
	94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
	break;
	case 1:
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
	"weapons/glock18-2.wav",
	1, ATTN_NORM, 0,
	94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
	break;
	}
	*/
	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// glock18 END
//======================

//======================
// M4A1 START
//======================

enum m4a1_e
{
    M4A1_IDLE1 = 0,
    M4A1_SHOOT1,
    M4A1_SHOOT2,
    M4A1_SHOOT3,
    M4A1_RELOAD,
    M4A1_DRAW,
    M4A1_ADD_SILENCER,
    M4A1_IDLE_UNSIL,
    M4A1_SHOOT1_UNSIL,
    M4A1_SHOOT2_UNSIL,
    M4A1_SHOOT3_UNSIL,
    M4A1_RELOAD_UNSIL,
    M4A1_DRAW_UNSIL,
    M4A1_DETACH_SILENCER
};

void EV_FireM4A1( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 100;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// M4A1 END
//======================

//======================
// AK47 START
//======================

enum ak47_e
{
    AK47_IDLE1 = 0,
    AK47_RELOAD,
    AK47_DRAW,
    AK47_SHOOT1,
    AK47_SHOOT2,
    AK47_SHOOT3
};

void EV_FireAK47( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// AK47 END
//======================

//======================
// AWP START
//======================

enum awp_e
{
    AWP_IDLE = 0,
    AWP_SHOOT1,
    AWP_SHOOT2,
    AWP_SHOOT3,
    AWP_RELOAD,
    AWP_DRAW
};

void EV_FireAWP( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation( AWP_SHOOT1, 2 );
		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/awp1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// AWP END
//======================

//======================
// GALIL START
//======================

enum galil_e
{
    GALIL_IDLE = 0,
    GALIL_RELOAD,
    GALIL_DRAW,
    GALIL_SHOOT1,
    GALIL_SHOOT2,
    GALIL_SHOOT3
};


void EV_FireGALIL( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// GALIL END
//======================

//======================
// FAMAS START
//======================

enum famas_e 
{
    FAMAS_IDLE = 0,
    FAMAS_RELOAD,
    FAMAS_DRAW,
    FAMAS_SHOOT1,
    FAMAS_SHOOT2,
    FAMAS_SHOOT3
};

void EV_FireFAMAS( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// FAMAS END
//======================

//======================
// DEAGLE START
//======================

enum deagle_e 
{
    DEAGLE_IDLE1 = 0,
    DEAGLE_SHOOT1,
    DEAGLE_SHOOT2,
    DEAGLE_SHOOT_EMPTY,
    DEAGLE_RELOAD,
    DEAGLE_DRAW
};

void EV_FireDEAGLE( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// DEAGLE END
//======================

//======================
// AUG START
//======================

enum aug_e
{
    AUG_IDLE = 0,
    AUG_RELOAD,
    AUG_DRAW,
    AUG_SHOOT1,
    AUG_SHOOT2,
    AUG_SHOOT3
};

void EV_FireAUG( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// AUG END
//======================

//======================
// SG552 START
//======================

enum sg552_e
{
    SG552_IDLE = 0,
    SG552_RELOAD,
    SG552_DRAW,
    SG552_SHOOT1,
    SG552_SHOOT2,
    SG552_SHOOT3
};

void EV_FireSG552( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// SG552 END
//======================

//======================
// MP5 START
//======================

enum mp5n_e
{
    MP5N_IDLE = 0,
    MP5N_RELOAD,
    MP5N_DRAW,
    MP5N_SHOOT1,
    MP5N_SHOOT2,
    MP5N_SHOOT3
};

void EV_FireMP5( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// MP5 END
//======================

//======================
// M3 START
//======================

enum m3_e
{
    M3_IDLE1 = 0,
    M3_SHOOT1,
    M3_SHOOT2,
    M3_INSERT,
    M3_AFTER_RELOAD,
    M3_START_RELOAD,
    M3_DRAW
};

void EV_FireM3( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		EV_MuzzleFlash();
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(m4a1_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2);
		gHUD.RealSize += 150;
		//V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/rshell.mdl");
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, -right, up, 12, -10, -7 );
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL);
	/*
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
			"weapons/m4a1-1.wav",
			1, ATTN_NORM, 0,
			94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );*/


	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	EV_HLDM_FireBullets( idx,
		forward,
		right,
		up,
		1,
		vecSrc,
		vecAiming,
		8192,
		0,
		0,
		0,
		args->fparam1,
		args->fparam2 );
}
//======================
// M3 END
//======================

void EV_TrainPitchAdjust( event_args_t *args )
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;

	char sz[ 256 ];

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	us_params = (unsigned short)args->iparam1;
	stop	  = args->bparam1;

	m_flVolume	= (float)(us_params & 0x003f)/40.0;
	noise		= (int)(((us_params) >> 12 ) & 0x0007);
	pitch		= (int)( 10.0 * (float)( ( us_params >> 6 ) & 0x003f ) );

	switch ( noise )
	{
	case 1: strcpy( sz, "plats/ttrain1.wav"); break;
	case 2: strcpy( sz, "plats/ttrain2.wav"); break;
	case 3: strcpy( sz, "plats/ttrain3.wav"); break; 
	case 4: strcpy( sz, "plats/ttrain4.wav"); break;
	case 5: strcpy( sz, "plats/ttrain6.wav"); break;
	case 6: strcpy( sz, "plats/ttrain7.wav"); break;
	default:
		// no sound
		strcpy( sz, "" );
		return;
	}

	if ( stop )
	{
		gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, sz );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch );
	}
}

int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 )
{
	return 0;
}