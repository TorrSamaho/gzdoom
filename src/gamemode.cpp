//-----------------------------------------------------------------------------
//
// Skulltag Source
// Copyright (C) 2007 Brad Carney
// Copyright (C) 2007-2012 Skulltag Development Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the Skulltag Development Team nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
// 4. Redistributions in any form must be accompanied by information on how to
//    obtain complete source code for the software and any accompanying
//    software that uses the software. The source code must either be included
//    in the distribution or be available for no more than the cost of
//    distribution plus a nominal fee, and must be freely redistributable
//    under reasonable conditions. For an executable file, complete source
//    code means the source code for all modules it contains. It does not
//    include source code for modules or files that typically accompany the
//    major components of the operating system on which the executable file
//    runs.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Date created:  7/12/07
//
//
// Filename: gamemode.cpp
//
// Description: 
//
//-----------------------------------------------------------------------------

#include "cooperative.h"
#include "deathmatch.h"
#include "domination.h"
#include "doomstat.h"
#include "d_event.h"
#include "gamemode.h"
#include "team.h"
#include "network.h"
#include "sv_commands.h"
#include "g_game.h"
#include "joinqueue.h"
#include "cl_demo.h"
#include "survival.h"
#include "duel.h"
#include "invasion.h"
#include "lastmanstanding.h"
#include "possession.h"
#include "p_lnspec.h"
#include "p_acs.h"
// [BB] The next includes are only needed for GAMEMODE_DisplayStandardMessage
#include "sbar.h"
#include "v_video.h"

//*****************************************************************************
//	CONSOLE VARIABLES

CVAR( Bool, instagib, false, CVAR_SERVERINFO | CVAR_LATCH | CVAR_CAMPAIGNLOCK );
CVAR( Bool, buckshot, false, CVAR_SERVERINFO | CVAR_LATCH | CVAR_CAMPAIGNLOCK );

CVAR( Bool, sv_suddendeath, true, CVAR_SERVERINFO | CVAR_LATCH );

//*****************************************************************************
//	VARIABLES

// Data for all our game modes.
static	GAMEMODE_s				g_GameModes[NUM_GAMEMODES];

// Our current game mode.
static	GAMEMODE_e				g_CurrentGameMode;

// [BB] Implement the string table and the conversion functions for the GMF and GAMEMODE enums.
#define GENERATE_ENUM_STRINGS  // Start string generation
#include "gamemode_enums.h"
#undef GENERATE_ENUM_STRINGS   // Stop string generation

//*****************************************************************************
//	FUNCTIONS

void GAMEMODE_Construct( void )
{
	// Regular co-op.
	g_GameModes[GAMEMODE_COOPERATIVE].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS;
	strcpy( g_GameModes[GAMEMODE_COOPERATIVE].szName, "Cooperative" );
	strncpy( g_GameModes[GAMEMODE_COOPERATIVE].szShortName, "COOP", 8 );
	strncpy( g_GameModes[GAMEMODE_COOPERATIVE].szF1Texture, "F1_COOP", 8 );

	// Survival co-op.
	g_GameModes[GAMEMODE_SURVIVAL].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_USEMAXLIVES|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_SURVIVAL].szName, "Survival" );
	strncpy( g_GameModes[GAMEMODE_SURVIVAL].szShortName, "SURV", 8 );
	strncpy( g_GameModes[GAMEMODE_SURVIVAL].szF1Texture, "F1_SCP", 8 );

	// Invasion.
	g_GameModes[GAMEMODE_INVASION].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_USEMAXLIVES;
	strcpy( g_GameModes[GAMEMODE_INVASION].szName, "Invasion" );
	strncpy( g_GameModes[GAMEMODE_INVASION].szShortName, "INVAS", 8 );
	strncpy( g_GameModes[GAMEMODE_INVASION].szF1Texture, "F1_INV", 8 );

	// Regular deathmatch.
	g_GameModes[GAMEMODE_DEATHMATCH].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS;
	strcpy( g_GameModes[GAMEMODE_DEATHMATCH].szName, "Deathmatch" );
	strncpy( g_GameModes[GAMEMODE_DEATHMATCH].szShortName, "DM", 8 );
	strncpy( g_GameModes[GAMEMODE_DEATHMATCH].szF1Texture, "F1_DM", 8 );

	// Teamplay DM.
	g_GameModes[GAMEMODE_TEAMPLAY].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS|GMF_PLAYERSONTEAMS;
	strcpy( g_GameModes[GAMEMODE_TEAMPLAY].szName, "Team Deathmatch" );
	strncpy( g_GameModes[GAMEMODE_TEAMPLAY].szShortName, "TDM", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMPLAY].szF1Texture, "F1_TDM", 8 );

	// Duel.
	g_GameModes[GAMEMODE_DUEL].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS|GMF_MAPRESETS|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_DUEL].szName, "Duel" );
	strncpy( g_GameModes[GAMEMODE_DUEL].szShortName, "DUEL", 8 );
	strncpy( g_GameModes[GAMEMODE_DUEL].szF1Texture, "F1_DUEL", 8 );

	// Terminator DM.
	g_GameModes[GAMEMODE_TERMINATOR].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS;
	strcpy( g_GameModes[GAMEMODE_TERMINATOR].szName, "Terminator" );
	strncpy( g_GameModes[GAMEMODE_TERMINATOR].szShortName, "TERM", 8 );
	strncpy( g_GameModes[GAMEMODE_TERMINATOR].szF1Texture, "F1_TERM", 8 );

	// Last man standing.
	g_GameModes[GAMEMODE_LASTMANSTANDING].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNWINS|GMF_DONTSPAWNMAPTHINGS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_USEMAXLIVES|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_LASTMANSTANDING].szName, "Last Man Standing" );
	strncpy( g_GameModes[GAMEMODE_LASTMANSTANDING].szShortName, "LMS", 8 );
	strncpy( g_GameModes[GAMEMODE_LASTMANSTANDING].szF1Texture, "F1_LMS", 8 );

	// Team LMS.
	g_GameModes[GAMEMODE_TEAMLMS].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNWINS|GMF_DONTSPAWNMAPTHINGS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_PLAYERSONTEAMS|GMF_USEMAXLIVES|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_TEAMLMS].szName, "Team Last Man Standing" );
	strncpy( g_GameModes[GAMEMODE_TEAMLMS].szShortName, "TLMS", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMLMS].szF1Texture, "F1_TLMS", 8 );

	// Possession DM.
	// [BB] Even though possession doesn't do a full map reset, it still resets the map time during its partial reset.
	g_GameModes[GAMEMODE_POSSESSION].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNPOINTS|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_POSSESSION].szName, "Possession" );
	strncpy( g_GameModes[GAMEMODE_POSSESSION].szShortName, "POSS", 8 );
	strncpy( g_GameModes[GAMEMODE_POSSESSION].szF1Texture, "F1_POSS", 8 );

	// Team possession.
	g_GameModes[GAMEMODE_TEAMPOSSESSION].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS|GMF_MAPRESET_RESETS_MAPTIME;
	strcpy( g_GameModes[GAMEMODE_TEAMPOSSESSION].szName, "Team Possession" );
	strncpy( g_GameModes[GAMEMODE_TEAMPOSSESSION].szShortName, "TM POSS", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMPOSSESSION].szF1Texture, "F1_TPOSS", 8 );

	// Regular teamgame.
	g_GameModes[GAMEMODE_TEAMGAME].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strcpy( g_GameModes[GAMEMODE_TEAMGAME].szName, "Team Game" );
	strncpy( g_GameModes[GAMEMODE_TEAMGAME].szShortName, "TM GAME", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMGAME].szF1Texture, "F1_TMGM", 8 );

	// Capture the flag.
	g_GameModes[GAMEMODE_CTF].ulFlags = GMF_TEAMGAME|GMF_USEFLAGASTEAMITEM|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS|GMF_USETEAMITEM;
	strcpy( g_GameModes[GAMEMODE_CTF].szName, "Capture the Flag" );
	strncpy( g_GameModes[GAMEMODE_CTF].szShortName, "CTF", 8 );
	strncpy( g_GameModes[GAMEMODE_CTF].szF1Texture, "F1_CTF", 8 );

	// One flag CTF.
	g_GameModes[GAMEMODE_ONEFLAGCTF].ulFlags = GMF_TEAMGAME|GMF_USEFLAGASTEAMITEM|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS|GMF_USETEAMITEM;
	strcpy( g_GameModes[GAMEMODE_ONEFLAGCTF].szName, "1-Flag CTF" );
	strncpy( g_GameModes[GAMEMODE_ONEFLAGCTF].szShortName, "1F-CTF", 8 );
	strncpy( g_GameModes[GAMEMODE_ONEFLAGCTF].szF1Texture, "F1_1FCTF", 8 );

	// Skulltag.
	g_GameModes[GAMEMODE_SKULLTAG].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS|GMF_USETEAMITEM;
	strcpy( g_GameModes[GAMEMODE_SKULLTAG].szName, "Skulltag" );
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szShortName, "ST", 8 );
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szF1Texture, "F1_ST", 8 );

	// Domination
	g_GameModes[GAMEMODE_DOMINATION].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strcpy( g_GameModes[GAMEMODE_DOMINATION].szName, "Domination" );
	strncpy( g_GameModes[GAMEMODE_DOMINATION].szShortName, "DOM", 8 );
	strncpy( g_GameModes[GAMEMODE_DOMINATION].szF1Texture, "F1_DOM", 8 );

	// [AK] Set the default values of all flagsets for all gamemodes.
	for ( int i = GAMEMODE_COOPERATIVE; i < NUM_GAMEMODES; i++ )
	{
		for ( int j = 0; j <= 1; j++ )
		{
			g_GameModes[i].lDMFlags[j] = 0;
			g_GameModes[i].lDMFlags[j] = 0;
			g_GameModes[i].lCompatFlags[j] = 0;
			g_GameModes[i].lCompatFlags[j] = 0;
			g_GameModes[i].lZaDMFlags[j] = 0;
			g_GameModes[i].lZaCompatFlags[j] = 0;
		}
	}

	// Our default game mode is co-op.
	g_CurrentGameMode = GAMEMODE_COOPERATIVE;

}

//*****************************************************************************
//
void GAMEMODE_Tick( void )
{
	static GAMESTATE_e oldState = GAMESTATE_UNSPECIFIED;
	const GAMESTATE_e state = GAMEMODE_GetState();

	// [BB] If the state change, potentially trigger an event and update the saved state.
	if ( oldState != state )
	{
		// [BB] Apparently the round just ended.
		if ( ( oldState == GAMESTATE_INPROGRESS ) && ( state == GAMESTATE_INRESULTSEQUENCE ) )
			GAMEMODE_HandleEvent ( GAMEEVENT_ROUND_ENDS );
		// [BB] Changing from GAMESTATE_INPROGRESS to anything but GAMESTATE_INRESULTSEQUENCE means the roudn was aborted.
		else if ( oldState == GAMESTATE_INPROGRESS )
			GAMEMODE_HandleEvent ( GAMEEVENT_ROUND_ABORTED );
		// [BB] Changing from anything to GAMESTATE_INPROGRESS means the round started.
		else if ( state == GAMESTATE_INPROGRESS )
			GAMEMODE_HandleEvent ( GAMEEVENT_ROUND_STARTS );			

		oldState = state;
	}
}

//*****************************************************************************
//
int GAMEMODE_ParserMustGetEnumName ( FScanner &sc, const char *EnumName, const char *FlagPrefix, int (*GetValueFromName) ( const char *Name ), const bool StringAlreadyParse = false )
{
	if ( StringAlreadyParse == false )
		sc.MustGetString ();
	FString flagname = FlagPrefix;
	flagname += sc.String;
	flagname.ToUpper();
	const int flagNum = GetValueFromName ( flagname.GetChars() );
	if ( flagNum == -1 )
		sc.ScriptError ( "Unknown %s '%s', on line %d in GAMEMODE.", EnumName, sc.String, sc.Line );
	return flagNum;
}

FFlagCVar *GAMEMODE_ParserMustGetFlagset ( FScanner &sc, const GAMEMODE_e GameMode, LONG **GameModeFlagset )
{
	sc.MustGetString();
	FBaseCVar *cvar = FindCVar( sc.String, NULL );

	// [AK] Make sure this a flag-type CVar.
	if (( cvar == NULL ) || ( cvar->IsFlagCVar() == false ))
		sc.ScriptError ( "'%s' is not a valid flag CVar.", sc.String );

	FFlagCVar* flag = static_cast<FFlagCVar *>( cvar );
	FIntCVar* flagset = flag->GetValueVar();

	// [AK] Make sure the flag belongs to a valid gameplay or compatibility flagset.
	if ( flagset == &dmflags )
		*GameModeFlagset = &g_GameModes[GameMode].lDMFlags[0];
	else if ( flagset == &dmflags2 )
		*GameModeFlagset = &g_GameModes[GameMode].lDMFlags2[0];
	else if ( flagset == &compatflags )
		*GameModeFlagset = &g_GameModes[GameMode].lCompatFlags[0];
	else if ( flagset == &compatflags2 )
		*GameModeFlagset = &g_GameModes[GameMode].lCompatFlags2[0];
	else if ( flagset == &zadmflags )
		*GameModeFlagset = &g_GameModes[GameMode].lZaDMFlags[0];
	else if ( flagset == &zacompatflags )
		*GameModeFlagset = &g_GameModes[GameMode].lZaCompatFlags[0];
	else
		sc.ScriptError ( "Invalid gameplay or compatibility flag '%s'.", sc.String, sc.Line );

	return flag;
}

//*****************************************************************************
//
void GAMEMODE_ParseGamemodeInfoLump ( FScanner &sc, const GAMEMODE_e GameMode )
{
	TEAMINFO team;
	LONG *flagset = NULL;

	sc.MustGetStringName("{");
	while (!sc.CheckString("}"))
	{
		sc.MustGetString();

		if (0 == stricmp (sc.String, "removeflag"))
		{
			g_GameModes[GameMode].ulFlags &= ~GAMEMODE_ParserMustGetEnumName( sc, "flag", "GMF_", GetValueGMF );
		}
		else if (0 == stricmp (sc.String, "addflag"))
		{
			g_GameModes[GameMode].ulFlags |= GAMEMODE_ParserMustGetEnumName( sc, "flag", "GMF_", GetValueGMF );
		}
		else if ((0 == stricmp (sc.String, "gamesettings")) || (0 == stricmp (sc.String, "lockedgamesettings")))
		{
			bool bLockFlags = !stricmp( sc.String, "lockedgamesettings" );

			sc.MustGetStringName( "{" );
			while ( !sc.CheckString( "}" ))
			{
				FFlagCVar *flag = GAMEMODE_ParserMustGetFlagset( sc, GameMode, &flagset );
				ULONG ulBit = flag->GetBitVal();
				bool bEnableFlag;
	
				// [AK] There must be an equal sign following the name of the flag.
				sc.MustGetStringName( "=" );
				sc.GetString();

				if ( stricmp( sc.String, "true" ) == 0 )
					bEnableFlag = true;
				else if ( stricmp( sc.String, "false" ) == 0 )
					bEnableFlag = false;
				else
					bEnableFlag = !!atoi( sc.String );

				// [AK] Enable or disable the flag as desired.
				if ( bEnableFlag )
					flagset[FLAGSET_VALUE] |= ulBit;
				else
					flagset[FLAGSET_VALUE] &= ~ulBit;

				flagset[FLAGSET_MASK] |= ulBit;

				// [AK] Lock this flag so it can't be manually changed.
				if ( bLockFlags )
					flagset[FLAGSET_LOCKEDMASK] |= ulBit;
			}
		}
		else if (0 == stricmp (sc.String, "removegamesetting"))
		{
			FFlagCVar *flag = GAMEMODE_ParserMustGetFlagset( sc, GameMode, &flagset );
			ULONG ulBit = flag->GetBitVal();

			flagset[FLAGSET_VALUE] &= ~ulBit;
			flagset[FLAGSET_MASK] &= ~ulBit;
			flagset[FLAGSET_LOCKEDMASK] &= ~ulBit;
		}
		else
			sc.ScriptError ( "Unknown option '%s', on line %d in GAMEMODE.", sc.String, sc.Line );
	}

	// [AK] Get the game mode type (cooperative, deathmatch, or team game). There shouldn't be more than one enabled or none at all.
	ULONG ulFlags = g_GameModes[GameMode].ulFlags & ( GMF_COOPERATIVE | GMF_DEATHMATCH | GMF_TEAMGAME );
	if (( ulFlags == 0 ) || (( ulFlags & ( ulFlags - 1 )) != 0 ))
		sc.ScriptError( "Can't determine if '%s' is cooperative, deathmatch, or team-based.", g_GameModes[GameMode].szName );

	// [AK] Get the type of "players earn" flag this game mode is currently using.
	ulFlags = g_GameModes[GameMode].ulFlags & ( GMF_PLAYERSEARNKILLS | GMF_PLAYERSEARNFRAGS | GMF_PLAYERSEARNPOINTS | GMF_PLAYERSEARNWINS );

	// [AK] If all of these flags were removed or if more than one was added, then throw an error.
	if ( ulFlags == 0 )
		sc.ScriptError( "Players have no way of earning kills, frags, points, or wins in '%s'.", g_GameModes[GameMode].szName );
	else if (( ulFlags & ( ulFlags - 1 )) != 0 )
		sc.ScriptError( "There is more than one PLAYERSEARN flag enabled in '%s'.", g_GameModes[GameMode].szName );
}

//*****************************************************************************
//
void GAMEMODE_ParseGamemodeInfo( void )
{
	int lastlump = 0, lump;

	while ((lump = Wads.FindLump ("GAMEMODE", &lastlump)) != -1)
	{
		FScanner sc(lump);
		while (sc.GetString ())
		{
			GAMEMODE_e GameMode = static_cast<GAMEMODE_e>( GAMEMODE_ParserMustGetEnumName( sc, "gamemode", "GAMEMODE_", GetValueGAMEMODE_e, true ) );
			GAMEMODE_ParseGamemodeInfoLump ( sc, GameMode );
		}
	}
}

//*****************************************************************************
//
ULONG GAMEMODE_GetFlags( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( 0 );

	return ( g_GameModes[GameMode].ulFlags );
}

//*****************************************************************************
//
ULONG GAMEMODE_GetCurrentFlags( void )
{
	return ( g_GameModes[g_CurrentGameMode].ulFlags );
}

//*****************************************************************************
//
char *GAMEMODE_GetShortName( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( NULL );

	return ( g_GameModes[GameMode].szShortName );
}

//*****************************************************************************
//
char *GAMEMODE_GetName( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( NULL );

	return ( g_GameModes[GameMode].szName );
}

//*****************************************************************************
//
char *GAMEMODE_GetCurrentName( void )
{
	return ( g_GameModes[g_CurrentGameMode].szName );
}

//*****************************************************************************
//
char *GAMEMODE_GetF1Texture( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( NULL );

	return ( g_GameModes[GameMode].szF1Texture );
}

//*****************************************************************************
//
int GAMEMODE_GetFlagsetMask( GAMEMODE_e GameMode, FIntCVar *Flagset, bool bLocked )
{
	ULONG ulMask = bLocked ? FLAGSET_LOCKEDMASK : FLAGSET_MASK;

	if ( Flagset == &dmflags )
		return ( g_GameModes[GameMode].lDMFlags[ulMask] );
	else if ( Flagset == &dmflags2 )
		return ( g_GameModes[GameMode].lDMFlags2[ulMask] );
	else if ( Flagset == &compatflags )
		return ( g_GameModes[GameMode].lCompatFlags[ulMask] );
	else if ( Flagset == &compatflags2 )
		return ( g_GameModes[GameMode].lCompatFlags2[ulMask] );
	else if ( Flagset == &zadmflags )
		return ( g_GameModes[GameMode].lZaDMFlags[ulMask] );
	else if ( Flagset == &zacompatflags )
		return ( g_GameModes[GameMode].lZaCompatFlags[ulMask] );
	
	// [AK] We passed an invalid flagset, just return zero.
	return ( 0 );
}

//*****************************************************************************
//
int GAMEMODE_GetCurrentFlagsetMask( FIntCVar *Flagset, bool bLocked )
{
	return ( GAMEMODE_GetFlagsetMask( g_CurrentGameMode, Flagset, bLocked ) );
}

//*****************************************************************************
//
void GAMEMODE_DetermineGameMode( void )
{
	g_CurrentGameMode = GAMEMODE_COOPERATIVE;
	if ( survival )
		g_CurrentGameMode = GAMEMODE_SURVIVAL;
	if ( invasion )
		g_CurrentGameMode = GAMEMODE_INVASION;
	if ( deathmatch )
		g_CurrentGameMode = GAMEMODE_DEATHMATCH;
	if ( teamplay )
		g_CurrentGameMode = GAMEMODE_TEAMPLAY;
	if ( duel )
		g_CurrentGameMode = GAMEMODE_DUEL;
	if ( terminator )
		g_CurrentGameMode = GAMEMODE_TERMINATOR;
	if ( lastmanstanding )
		g_CurrentGameMode = GAMEMODE_LASTMANSTANDING;
	if ( teamlms )
		g_CurrentGameMode = GAMEMODE_TEAMLMS;
	if ( possession )
		g_CurrentGameMode = GAMEMODE_POSSESSION;
	if ( teampossession )
		g_CurrentGameMode = GAMEMODE_TEAMPOSSESSION;
	if ( teamgame )
		g_CurrentGameMode = GAMEMODE_TEAMGAME;
	if ( ctf )
		g_CurrentGameMode = GAMEMODE_CTF;
	if ( oneflagctf )
		g_CurrentGameMode = GAMEMODE_ONEFLAGCTF;
	if ( skulltag )
		g_CurrentGameMode = GAMEMODE_SKULLTAG;
	if ( domination )
		g_CurrentGameMode = GAMEMODE_DOMINATION;
}

//*****************************************************************************
//
bool GAMEMODE_IsGameWaitingForPlayers( void )
{
	if ( survival )
		return ( SURVIVAL_GetState( ) == SURVS_WAITINGFORPLAYERS );
	else if ( invasion )
		return ( INVASION_GetState( ) == IS_WAITINGFORPLAYERS );
	else if ( duel )
		return ( DUEL_GetState( ) == DS_WAITINGFORPLAYERS );
	else if ( teamlms || lastmanstanding )
		return ( LASTMANSTANDING_GetState( ) == LMSS_WAITINGFORPLAYERS );
	else if ( possession || teampossession )
		return ( POSSESSION_GetState( ) == PSNS_WAITINGFORPLAYERS );
	// [BB] Non-coop game modes need two or more players.
	else if ( ( GAMEMODE_GetCurrentFlags() & GMF_COOPERATIVE ) == false )
		return ( GAME_CountActivePlayers( ) < 2 );
	// [BB] For coop games one player is enough.
	else
		return ( GAME_CountActivePlayers( ) < 1 );
}

//*****************************************************************************
//
bool GAMEMODE_IsGameInCountdown( void )
{
	if ( survival )
		return ( SURVIVAL_GetState( ) == SURVS_COUNTDOWN );
	else if ( invasion )
		return ( ( INVASION_GetState( ) == IS_FIRSTCOUNTDOWN ) || ( INVASION_GetState( ) == IS_COUNTDOWN ) );
	else if ( duel )
		return ( DUEL_GetState( ) == DS_COUNTDOWN );
	else if ( teamlms || lastmanstanding )
		return ( LASTMANSTANDING_GetState( ) == LMSS_COUNTDOWN );
	// [BB] What about PSNS_PRENEXTROUNDCOUNTDOWN?
	else if ( possession || teampossession )
		return ( ( POSSESSION_GetState( ) == PSNS_COUNTDOWN ) || ( POSSESSION_GetState( ) == PSNS_NEXTROUNDCOUNTDOWN ) );
	// [BB] The other game modes don't have a countdown.
	else
		return ( false );
}

//*****************************************************************************
//
bool GAMEMODE_IsGameInProgress( void )
{
	// [BB] Since there is currently no way unified way to check the state of
	// the active game mode, we have to do it manually.
	if ( survival )
		return ( SURVIVAL_GetState( ) == SURVS_INPROGRESS );
	else if ( invasion )
		return ( ( INVASION_GetState( ) == IS_INPROGRESS ) || ( INVASION_GetState( ) == IS_BOSSFIGHT ) || ( INVASION_GetState( ) == IS_WAVECOMPLETE ) );
	else if ( duel )
		return ( DUEL_GetState( ) == DS_INDUEL );
	else if ( teamlms || lastmanstanding )
		return ( LASTMANSTANDING_GetState( ) == LMSS_INPROGRESS );
	else if ( possession || teampossession )
		return ( ( POSSESSION_GetState( ) == PSNS_INPROGRESS ) || ( POSSESSION_GetState( ) == PSNS_ARTIFACTHELD ) );
	// [BB] In non-coop game modes without warmup phase, we just say the game is
	// in progress when there are two or more players and the game is not frozen
	// due to the end level delay.
	else if ( ( GAMEMODE_GetCurrentFlags() & GMF_COOPERATIVE ) == false )
		return ( ( GAME_CountActivePlayers( ) >= 2 ) && ( GAME_GetEndLevelDelay () == 0 ) );
	// [BB] For coop games one player is enough.
	else
		return ( ( GAME_CountActivePlayers( ) >= 1 ) && ( GAME_GetEndLevelDelay () == 0 ) );
}

//*****************************************************************************
//
bool GAMEMODE_IsGameInResultSequence( void )
{
	if ( survival )
		return ( SURVIVAL_GetState( ) == SURVS_MISSIONFAILED );
	else if ( invasion )
		return ( INVASION_GetState( ) == IS_MISSIONFAILED );
	else if ( duel )
		return ( DUEL_GetState( ) == DS_WINSEQUENCE );
	else if ( teamlms || lastmanstanding )
		return ( LASTMANSTANDING_GetState( ) == LMSS_WINSEQUENCE );
	// [BB] The other game modes don't have such a sequnce. Arguably, possession
	// with PSNS_HOLDERSCORED could also be considered for this.
	// As substitute for such a sequence we consider whether the game is
	// frozen because of the end level delay.
	else
		return ( GAME_GetEndLevelDelay () > 0 );
}

//*****************************************************************************
//
bool GAMEMODE_IsGameInProgressOrResultSequence( void )
{
	return ( GAMEMODE_IsGameInProgress() || GAMEMODE_IsGameInResultSequence() );
}

//*****************************************************************************
//
bool GAMEMODE_IsLobbyMap( void )
{
	return level.flagsZA & LEVEL_ZA_ISLOBBY || stricmp(level.mapname, lobby) == 0;
}

//*****************************************************************************
//
bool GAMEMODE_IsLobbyMap( const char* mapname )
{
	// [BB] The level is not loaded yet, so we can't use level.flags2 directly.
	const level_info_t *levelinfo = FindLevelInfo( mapname, false );

	if (levelinfo == NULL)
	{
		return false;
	}

	return levelinfo->flagsZA & LEVEL_ZA_ISLOBBY || stricmp( levelinfo->mapname, lobby ) == 0;
}

//*****************************************************************************
//
bool GAMEMODE_IsNextMapCvarLobby( void )
{
	// If we're using a CVAR lobby and we're not on the lobby map, the next map
	// should always be the lobby.
	return strcmp(lobby, "") != 0 && stricmp(lobby, level.mapname) != 0;
}

//*****************************************************************************
//
bool GAMEMODE_IsTimelimitActive( void )
{
	// [AM] If the map is a lobby, ignore the timelimit.
	if ( GAMEMODE_IsLobbyMap( ) )
		return false;

	// [BB] In gamemodes that reset the time during a map reset, the timelimit doesn't make sense when the game is not in progress.
	if ( ( GAMEMODE_GetCurrentFlags() & GMF_MAPRESET_RESETS_MAPTIME ) && ( GAMEMODE_IsGameInProgress( ) == false ) )
		return false;

	// [BB] Teamlms doesn't support timelimit, so just turn it off in this mode.
	if ( teamlms )
		return false;

	// [BB] SuperGod insisted to have timelimit in coop, e.g. for jumpmaze, but its implementation conceptually doesn't work in invasion or survival.
	return (/*( deathmatch || teamgame ) &&*/ ( invasion == false ) && ( survival == false ) && timelimit );
}

//*****************************************************************************
//
void GAMEMODE_GetTimeLeftString( FString &TimeLeftString )
{
	LONG	lTimeLeft = (LONG)( timelimit * ( TICRATE * 60 )) - level.time;
	ULONG	ulHours, ulMinutes, ulSeconds;

	if ( lTimeLeft <= 0 )
		ulHours = ulMinutes = ulSeconds = 0;
	else
	{
		ulHours = lTimeLeft / ( TICRATE * 3600 );
		lTimeLeft -= ulHours * TICRATE * 3600;
		ulMinutes = lTimeLeft / ( TICRATE * 60 );
		lTimeLeft -= ulMinutes * TICRATE * 60;
		ulSeconds = lTimeLeft / TICRATE;
	}

	if ( ulHours )
		TimeLeftString.Format ( "%02d:%02d:%02d", static_cast<unsigned int> (ulHours), static_cast<unsigned int> (ulMinutes), static_cast<unsigned int> (ulSeconds) );
	else
		TimeLeftString.Format ( "%02d:%02d", static_cast<unsigned int> (ulMinutes), static_cast<unsigned int> (ulSeconds) );
}

//*****************************************************************************
//
void GAMEMODE_RespawnDeadSpectators( BYTE Playerstate )
{
	// [BB] This is server side.
	if ( NETWORK_InClientMode() )
	{
		return;
	}

	// [BB] Any player spawning in this game state would fail.
	// [BB] The same is true when we are starting a new game at the moment.
	if ( ( gamestate == GS_STARTUP ) || ( gamestate == GS_FULLCONSOLE ) || ( gameaction == ga_newgame )  || ( gameaction == ga_newgame2 ) )
	{
		return;
	}

	// Respawn any players who were downed during the previous round.
	for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
	{
		if (( playeringame[ulIdx] == false ) ||
			( PLAYER_IsTrueSpectator( &players[ulIdx] )))
		{
			continue;
		}

		// We don't want to respawn players as soon as the map starts; we only
		// [BB] respawn all dead players and dead spectators.
		if ((( players[ulIdx].mo == NULL ) ||
			( players[ulIdx].mo->health > 0 )) &&
			( players[ulIdx].bDeadSpectator == false ))
		{
			continue;
		}

		players[ulIdx].bSpectating = false;
		players[ulIdx].bDeadSpectator = false;
		if ( GAMEMODE_GetCurrentFlags() & GMF_USEMAXLIVES )
		{
			PLAYER_SetLivesLeft ( &players[ulIdx], GAMEMODE_GetMaxLives() - 1 );
		}
		players[ulIdx].playerstate = Playerstate;

		APlayerPawn *oldactor = players[ulIdx].mo;

		GAMEMODE_SpawnPlayer( ulIdx );

		// [CK] Ice corpses that are persistent between rounds must not affect
		// the client post-death in any gamemode with a countdown.
		if (( oldactor ) && ( oldactor->health > 0 || oldactor->flags & MF_ICECORPSE ))
		{
			if ( NETWORK_GetState( ) == NETSTATE_SERVER )
				SERVERCOMMANDS_DestroyThing( oldactor );

			oldactor->Destroy( );
		}


		// [BB] If he's a bot, tell him that he successfully joined.
		if ( players[ulIdx].bIsBot && players[ulIdx].pSkullBot )
			players[ulIdx].pSkullBot->PostEvent( BOTEVENT_JOINEDGAME );
	}

	// [BB] Dead spectators were allowed to use chasecam, but are not necessarily allowed to use it
	// when alive again. Re-applying dmflags2 takes care of this.
	dmflags2 = dmflags2;
}

void GAMEMODE_RespawnDeadSpectatorsAndPopQueue( BYTE Playerstate )
{
	GAMEMODE_RespawnDeadSpectators( Playerstate );
	// Let anyone who's been waiting in line join now.
	JOINQUEUE_PopQueue( -1 );
}

//*****************************************************************************
//
void GAMEMODE_RespawnAllPlayers( BOTEVENT_e BotEvent, playerstate_t PlayerState )
{
	// [BB] This is server side.
	if ( NETWORK_InClientMode() == false )
	{
		// Respawn the players.
		for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
		{
			if (( playeringame[ulIdx] == false ) ||
				( PLAYER_IsTrueSpectator( &players[ulIdx] )))
			{
				continue;
			}

			// [BB] Disassociate the player body, but don't delete it right now. The clients
			// still need the old body while the player is respawned so that they can properly
			// transfer their camera if they are spying through the eyes of the respawned player.
			APlayerPawn* pOldPlayerBody = players[ulIdx].mo;
			players[ulIdx].mo = NULL;

			players[ulIdx].playerstate = PlayerState;
			GAMEMODE_SpawnPlayer( ulIdx );

			if ( pOldPlayerBody )
			{
				if ( NETWORK_GetState( ) == NETSTATE_SERVER )
					SERVERCOMMANDS_DestroyThing( pOldPlayerBody );

				pOldPlayerBody->Destroy( );
			}

			if ( players[ulIdx].pSkullBot && ( BotEvent < NUM_BOTEVENTS ) )
				players[ulIdx].pSkullBot->PostEvent( BotEvent );
		}
	}
}

//*****************************************************************************
//
void GAMEMODE_SpawnPlayer( const ULONG ulPlayer, bool bClientUpdate )
{
	// Spawn the player at their appropriate team start.
	if ( GAMEMODE_GetCurrentFlags() & GMF_TEAMGAME )
	{
		if ( players[ulPlayer].bOnTeam )
			G_TeamgameSpawnPlayer( ulPlayer, players[ulPlayer].Team, bClientUpdate );
		else
			G_TemporaryTeamSpawnPlayer( ulPlayer, bClientUpdate );
	}
	// If deathmatch, just spawn at a random spot.
	else if ( GAMEMODE_GetCurrentFlags() & GMF_DEATHMATCH )
		G_DeathMatchSpawnPlayer( ulPlayer, bClientUpdate );
	// Otherwise, just spawn at their normal player start.
	else
		G_CooperativeSpawnPlayer( ulPlayer, bClientUpdate );
}

//*****************************************************************************
//
void GAMEMODE_ResetPlayersKillCount( const bool bInformClients )
{
	// Reset everyone's kill count.
	for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
	{
		players[ulIdx].killcount = 0;
		players[ulIdx].RailgunShots = 0;
		// [BB] Also reset the things for ZADF_AWARD_DAMAGE_INSTEAD_KILLS.
		players[ulIdx].lPointCount = 0;
		players[ulIdx].ulUnrewardedDamageDealt = 0;

		// [BB] Notify the clients about the killcount change.
		if ( playeringame[ulIdx] && bInformClients && (NETWORK_GetState() == NETSTATE_SERVER) )
		{
			SERVERCOMMANDS_SetPlayerKillCount ( ulIdx );
			SERVERCOMMANDS_SetPlayerPoints ( ulIdx );
		}
	}
}

//*****************************************************************************
//
bool GAMEMODE_AreSpectatorsFordiddenToChatToPlayers( void )
{
	if ( ( lmsspectatorsettings & LMS_SPF_CHAT ) == false )
	{
		if (( teamlms || lastmanstanding ) && ( LASTMANSTANDING_GetState( ) == LMSS_INPROGRESS ))
			return true;

		if ( ( zadmflags & ZADF_ALWAYS_APPLY_LMS_SPECTATORSETTINGS ) && GAMEMODE_IsGameInProgress() )
			return true;
	}

	return false;
}

//*****************************************************************************
//
bool GAMEMODE_IsClientFordiddenToChatToPlayers( const ULONG ulClient )
{
	// [BB] If it's not a valid client, there are no restrictions. Note:
	// ulClient == MAXPLAYERS means the server wants to say something.
	if ( ulClient >= MAXPLAYERS )
		return false;

	// [BB] Ingame players are allowed to chat to other players.
	if ( players[ulClient].bSpectating == false )
		return false;

	return GAMEMODE_AreSpectatorsFordiddenToChatToPlayers();
}

//*****************************************************************************
//
bool GAMEMODE_PreventPlayersFromJoining( ULONG ulExcludePlayer )
{
	// [BB] No free player slots.
	if ( ( NETWORK_GetState( ) == NETSTATE_SERVER ) && ( SERVER_CalcNumNonSpectatingPlayers( ulExcludePlayer ) >= static_cast<unsigned> (sv_maxplayers) ) )
		return true;

	// [BB] Don't let players join during intermission. They will be put in line and the queue is popped once intermission ends.
	if ( gamestate == GS_INTERMISSION )
		return true;

	// [BB] Duel in progress.
	if ( duel && ( DUEL_CountActiveDuelers( ) >= 2 ) )
		return true;

	// [BB] If lives are limited, players are not allowed to join most of the time.
	// [BB] The ga_worlddone check makes sure that in game players (at least in survival and survival invasion) aren't forced to spectate after a "changemap" map change.
	// [BB] The ga_newgame check fixes some problem when starting a survival invasion skirmish with bots while already in a survival invasion game with bots
	// (the consoleplayer is spawned as spectator in this case and leaves a ghost player upon joining)
	if ( ( gameaction != ga_worlddone ) && ( gameaction != ga_newgame ) && GAMEMODE_AreLivesLimited() && GAMEMODE_IsGameInProgressOrResultSequence() )
			return true;

	return false;
}

//*****************************************************************************
//
bool GAMEMODE_AreLivesLimited( void )
{
	// [BB] Invasion is a special case: If sv_maxlives == 0 in invasion, players have infinite lives.
	return ( ( ( sv_maxlives > 0 ) || ( invasion == false ) ) && ( GAMEMODE_GetCurrentFlags() & GMF_USEMAXLIVES ) );
}

//*****************************************************************************
//
bool GAMEMODE_IsPlayerCarryingGameModeItem( player_t *player )
{
	GAMEMODE_e mode = GAMEMODE_GetCurrentMode( );

	// [AK] Check if this player is carrying a team item like an enemy team's flag or skull.
	if (( GAMEMODE_GetCurrentFlags( ) & GMF_USETEAMITEM ) && ( player->mo != NULL ))
	{
		if ( TEAM_FindOpposingTeamsItemInPlayersInventory( player ))
			return true;

		// [AK] Check if the player is carrying any team item. This can be useful for mods that
		// might define their own special items.
		AInventory *item = player->mo->FindInventory( PClass::FindClass( "TeamItem" ), true );

		// [AK] The player shouldn't have the white flag when we're not playing one-flag CTF.
		return (( item ) && (( item->IsKindOf( PClass::FindClass( "WhiteFlag" )) == false ) || ( mode == GAMEMODE_ONEFLAGCTF )));
	}

	// [AK] Check if this player is carrying the terminator sphere while playing terminator.
	if (( mode == GAMEMODE_TERMINATOR ) && ( player->cheats2 & CF2_TERMINATORARTIFACT ))
		return true;

	// [AK] Check if this player is carrying the hellstone while playing (team) possession.
	if ((( mode == GAMEMODE_POSSESSION ) || ( mode == GAMEMODE_TEAMPOSSESSION )) && ( player->cheats2 & CF2_POSSESSIONARTIFACT ))
		return true;

	return false;
}

//*****************************************************************************
//
unsigned int GAMEMODE_GetMaxLives( void )
{
	return ( ( sv_maxlives > 0 ) ? static_cast<unsigned int> ( sv_maxlives ) : 1 );
}

//*****************************************************************************
//
void GAMEMODE_AdjustActorSpawnFlags ( AActor *pActor )
{
	if ( pActor == NULL )
		return;

	// [BB] Since several Skulltag versions added NOGRAVITY to some spheres on default, allow the user to restore this behavior.
	if ( zacompatflags & ZACOMPATF_NOGRAVITY_SPHERES )
	{
		if ( ( stricmp ( pActor->GetClass()->TypeName.GetChars(), "InvulnerabilitySphere" ) == 0 )
			|| ( stricmp ( pActor->GetClass()->TypeName.GetChars(), "Soulsphere" ) == 0 )
			|| ( stricmp ( pActor->GetClass()->TypeName.GetChars(), "Megasphere" ) == 0 ) 
			|| ( stricmp ( pActor->GetClass()->TypeName.GetChars(), "BlurSphere" ) == 0 ) )
			pActor->flags |= MF_NOGRAVITY;
	}
}

//*****************************************************************************
//
void GAMEMODE_SpawnSpecialGamemodeThings ( void )
{
	// [BB] The server will let the clients know of any necessary spawns.
	if ( NETWORK_InClientMode() == false )
	{
		// Spawn the terminator artifact in terminator mode.
		if ( terminator )
			GAME_SpawnTerminatorArtifact( );

		// Spawn the possession artifact in possession/team possession mode.
		if ( possession || teampossession )
			GAME_SpawnPossessionArtifact( );
	}
}

//*****************************************************************************
//
void GAMEMODE_ResetSpecalGamemodeStates ( void )
{
	// [BB] If playing Domination reset ownership, even the clients can do this.
	if ( domination )
		DOMINATION_Reset();

	// [BB] If playing possession make sure to end the held countdown, even the clients can do this.
	if ( possession || teampossession )
	{
		POSSESSION_SetArtifactHoldTicks ( 0 );
		if ( POSSESSION_GetState() == PSNS_ARTIFACTHELD )
			POSSESSION_SetState( PSNS_PRENEXTROUNDCOUNTDOWN );
	}
}

//*****************************************************************************
//
bool GAMEMODE_IsSpectatorAllowedSpecial ( const int Special )
{
	return ( ( Special == Teleport ) || ( Special == Teleport_NoFog ) || ( Special == Teleport_NoStop ) || ( Special == Teleport_Line ) );
}

//*****************************************************************************
//
bool GAMEMODE_IsHandledSpecial ( AActor *Activator, int Special )
{
	// [BB] Non-player activated specials are never handled by the client.
	if ( Activator == NULL || Activator->player == NULL )
		return ( NETWORK_InClientMode() == false );

	// [EP/BB] Spectators activate a very limited amount of specials and ignore all others.
	if ( Activator->player->bSpectating )
		return ( GAMEMODE_IsSpectatorAllowedSpecial( Special ) );

	// [BB] Clients predict a very limited amount of specials for the local player and ignore all others (spectators were already handled)
	if ( NETWORK_InClientMode() )
		return ( NETWORK_IsConsolePlayer ( Activator ) && NETWORK_IsClientPredictedSpecial( Special ) );

	// [BB] Neither spectator, nor client.
	return true;
}

//*****************************************************************************
//
GAMESTATE_e GAMEMODE_GetState( void )
{
	if ( GAMEMODE_IsGameWaitingForPlayers() )
		return GAMESTATE_WAITFORPLAYERS;
	else if ( GAMEMODE_IsGameInCountdown() )
		return GAMESTATE_COUNTDOWN;
	else if ( GAMEMODE_IsGameInProgress() )
		return GAMESTATE_INPROGRESS;
	else if ( GAMEMODE_IsGameInResultSequence() )
		return GAMESTATE_INRESULTSEQUENCE;

	// [BB] Some of the above should apply, but this function always has to return something.
	return GAMESTATE_UNSPECIFIED;
}

//*****************************************************************************
//
void GAMEMODE_SetState( GAMESTATE_e GameState )
{
	if( GameState == GAMESTATE_WAITFORPLAYERS )
	{
		if ( survival )
			SURVIVAL_SetState( SURVS_WAITINGFORPLAYERS );
		else if ( invasion )
			INVASION_SetState( IS_WAITINGFORPLAYERS );
		else if ( duel )
			DUEL_SetState( DS_WAITINGFORPLAYERS );
		else if ( teamlms || lastmanstanding )
			LASTMANSTANDING_SetState( LMSS_WAITINGFORPLAYERS );
		else if ( possession || teampossession )
			POSSESSION_SetState( PSNS_WAITINGFORPLAYERS );
	}
	else if( GameState == GAMESTATE_COUNTDOWN )
	{
		if ( survival )
			SURVIVAL_SetState( SURVS_COUNTDOWN );
		else if ( invasion )
			INVASION_SetState( IS_FIRSTCOUNTDOWN );
		else if ( duel )
			DUEL_SetState( DS_COUNTDOWN );
		else if ( teamlms || lastmanstanding )
			LASTMANSTANDING_SetState( LMSS_COUNTDOWN );
		else if ( possession || teampossession )
			POSSESSION_SetState( PSNS_COUNTDOWN );
	}
	else if( GameState == GAMESTATE_INPROGRESS )
	{
		if ( survival )
			SURVIVAL_SetState( SURVS_INPROGRESS );
		else if ( invasion )
			INVASION_SetState( IS_INPROGRESS );
		else if ( duel )
			DUEL_SetState( DS_INDUEL );
		else if ( teamlms || lastmanstanding )
			LASTMANSTANDING_SetState( LMSS_INPROGRESS );
		else if ( possession || teampossession )
			POSSESSION_SetState( PSNS_INPROGRESS );
	}
	else if( GameState == GAMESTATE_INRESULTSEQUENCE )
	{
		if ( survival )
			SURVIVAL_SetState( SURVS_MISSIONFAILED );
		else if ( invasion )
			INVASION_SetState( IS_MISSIONFAILED );
		else if ( duel )
			DUEL_SetState( DS_WINSEQUENCE );
		else if ( teamlms || lastmanstanding )
			LASTMANSTANDING_SetState( LMSS_WINSEQUENCE );
	}
}

//*****************************************************************************
//
void GAMEMODE_HandleEvent ( const GAMEEVENT_e Event, AActor *pActivator, const int DataOne, const int DataTwo )
{
	// [BB] Clients don't start scripts.
	if ( NETWORK_InClientMode() )
		return;

	// [BB] The activator of the event activates the event script.
	// The first argument is the type, e.g. GAMEEVENT_PLAYERFRAGS,
	// the second and third are specific to the event, e.g. the second is the number of the fragged player.
	// The third argument will be zero if it isn't used in the script.
	FBehavior::StaticStartTypedScripts( SCRIPT_Event, pActivator, true, Event, false, false, DataOne, DataTwo );
}

//*****************************************************************************
//
void GAMEMODE_DisplayStandardMessage( const char *pszMessage, const bool bInformClients )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut	*pMsg;

		// Display the HUD message.
		pMsg = new DHUDMessageFadeOut( BigFont, pszMessage,
			160.4f,
			75.0f,
			320,
			200,
			CR_RED,
			3.0f,
			2.0f );

		StatusBar->AttachMessage( pMsg, MAKE_ID('C','N','T','R') );
	}
	// If necessary, send it to clients.
	else if ( bInformClients )
	{
		SERVERCOMMANDS_PrintHUDMessage( pszMessage, 160.4f, 75.0f, 320, 200, HUDMESSAGETYPE_FADEOUT, CR_RED, 3.0f, 0.0f, 2.0f, "BigFont", MAKE_ID( 'C', 'N', 'T', 'R' ) );
	}
}

//*****************************************************************************
// [BB] Expects pszMessage already to be colorized with V_ColorizeString.
void GAMEMODE_DisplayCNTRMessage( const char *pszMessage, const bool bInformClients, const ULONG ulPlayerExtra, const ULONG ulFlags )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut *pMsg = new DHUDMessageFadeOut( BigFont, pszMessage,
			1.5f,
			TEAM_MESSAGE_Y_AXIS,
			0,
			0,
			CR_UNTRANSLATED,
			3.0f,
			0.25f );
		StatusBar->AttachMessage( pMsg, MAKE_ID( 'C','N','T','R' ));
	}
	// If necessary, send it to clients.
	else if ( bInformClients )
	{
		SERVERCOMMANDS_PrintHUDMessage( pszMessage, 1.5f, TEAM_MESSAGE_Y_AXIS, 0, 0, HUDMESSAGETYPE_FADEOUT, CR_UNTRANSLATED, 3.0f, 0.0f, 0.25f, "BigFont", MAKE_ID( 'C', 'N', 'T', 'R' ), ulPlayerExtra, ServerCommandFlags::FromInt( ulFlags ) );
	}
}

//*****************************************************************************
// [BB] Expects pszMessage already to be colorized with V_ColorizeString.
void GAMEMODE_DisplaySUBSMessage( const char *pszMessage, const bool bInformClients, const ULONG ulPlayerExtra, const ULONG ulFlags )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut *pMsg = new DHUDMessageFadeOut( SmallFont, pszMessage,
			1.5f,
			TEAM_MESSAGE_Y_AXIS_SUB,
			0,
			0,
			CR_UNTRANSLATED,
			3.0f,
			0.25f );
		StatusBar->AttachMessage( pMsg, MAKE_ID( 'S','U','B','S' ));
	}
	// If necessary, send it to clients.
	else if ( bInformClients )
	{
		SERVERCOMMANDS_PrintHUDMessage( pszMessage, 1.5f, TEAM_MESSAGE_Y_AXIS_SUB, 0, 0, HUDMESSAGETYPE_FADEOUT, CR_UNTRANSLATED, 3.0f, 0.0f, 0.25f, "SmallFont", MAKE_ID( 'S', 'U', 'B', 'S' ), ulPlayerExtra, ServerCommandFlags::FromInt( ulFlags ) );
	}
}

//*****************************************************************************
//
GAMEMODE_e GAMEMODE_GetCurrentMode( void )
{
	return ( g_CurrentGameMode );
}

//*****************************************************************************
//
void GAMEMODE_SetCurrentMode( GAMEMODE_e GameMode )
{
	UCVarValue	 Val;
	g_CurrentGameMode = GameMode;	
	
	// [RC] Set all the CVars. We can't just use "= true;" because of the latched cvars.
	// (Hopefully Blzut's update will save us from this garbage.)

	Val.Bool = false;
	// [BB] Even though setting deathmatch and teamgame to false will set cooperative to true,
	// we need to set cooperative to false here first to clear survival and invasion.
	cooperative.ForceSet( Val, CVAR_Bool );
	deathmatch.ForceSet( Val, CVAR_Bool );
	teamgame.ForceSet( Val, CVAR_Bool );
	instagib.ForceSet( Val, CVAR_Bool );
	buckshot.ForceSet( Val, CVAR_Bool );

	Val.Bool = true;
	switch ( GameMode )
	{
	case GAMEMODE_COOPERATIVE:

		cooperative.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_SURVIVAL:

		survival.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_INVASION:

		invasion.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DEATHMATCH:

		deathmatch.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMPLAY:

		teamplay.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DUEL:

		duel.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TERMINATOR:

		terminator.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_LASTMANSTANDING:

		lastmanstanding.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMLMS:

		teamlms.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_POSSESSION:

		possession.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMPOSSESSION:

		teampossession.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMGAME:

		teamgame.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_CTF:

		ctf.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_ONEFLAGCTF:

		oneflagctf.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_SKULLTAG:

		skulltag.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DOMINATION:

		domination.ForceSet( Val, CVAR_Bool );
		break;
	default:
		break;
	}
}

//*****************************************************************************
//
MODIFIER_e GAMEMODE_GetModifier( void )
{
	if ( instagib )
		return MODIFIER_INSTAGIB;
	else if ( buckshot )
		return MODIFIER_BUCKSHOT;
	else
		return MODIFIER_NONE;
}

//*****************************************************************************
//
void GAMEMODE_SetModifier( MODIFIER_e Modifier )
{
	UCVarValue	 Val;

	// Turn them all off.
	Val.Bool = false;
	instagib.ForceSet( Val, CVAR_Bool );
	buckshot.ForceSet( Val, CVAR_Bool );

	// Turn the selected one on.
	Val.Bool = true;
	switch ( Modifier )
	{
	case MODIFIER_INSTAGIB:
		instagib.ForceSet( Val, CVAR_Bool );
		break;
	case MODIFIER_BUCKSHOT:
		buckshot.ForceSet( Val, CVAR_Bool );
		break;
	default:
		break;
	}
}

//*****************************************************************************
//
ULONG GAMEMODE_GetCountdownTicks( void )
{
	if ( g_CurrentGameMode == GAMEMODE_SURVIVAL )
		return ( SURVIVAL_GetCountdownTicks( ));
	else if ( g_CurrentGameMode == GAMEMODE_INVASION )
		return ( INVASION_GetCountdownTicks( ));
	else if ( g_CurrentGameMode == GAMEMODE_DUEL )
		return ( DUEL_GetCountdownTicks( ));
	else if (( g_CurrentGameMode == GAMEMODE_LASTMANSTANDING ) || ( g_CurrentGameMode == GAMEMODE_TEAMLMS ))
		return ( LASTMANSTANDING_GetCountdownTicks( ));
	else if (( g_CurrentGameMode == GAMEMODE_POSSESSION ) || ( g_CurrentGameMode == GAMEMODE_TEAMPOSSESSION ))
		return ( POSSESSION_GetCountdownTicks( ));

	// [AK] The other gamemodes don't have a countdown, so just return zero.
	return ( 0 );
}

//*****************************************************************************
//
player_t *GAMEMODE_GetArtifactCarrier( void )
{
	for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
	{
		if ( playeringame[ulIdx] == false )
			continue;

		// [AK] Is this player carrying the terminator artifact?
		if ( g_CurrentGameMode == GAMEMODE_TERMINATOR )
		{
			if ( players[ulIdx].cheats2 & CF2_TERMINATORARTIFACT )
				return ( &players[ulIdx] );
		}
		// [AK] Is this player carrying the possession artifact?
		else if (( g_CurrentGameMode == GAMEMODE_POSSESSION ) || ( g_CurrentGameMode == GAMEMODE_TEAMPOSSESSION ))
		{
			if ( players[ulIdx].cheats2 & CF2_POSSESSIONARTIFACT )
				return ( &players[ulIdx] );
		}
		// [AK] Is this player carrying the white flag?
		else if (( players[ulIdx].mo ) && ( players[ulIdx].mo->FindInventory( PClass::FindClass( "WhiteFlag" ), true )))
		{
			return ( &players[ulIdx] );
		}
	}

	return ( NULL );
}

//*****************************************************************************
//
void GAMEMODE_SetLimit( GAMELIMIT_e GameLimit, int value )
{
	UCVarValue Val;
	Val.Int = value;

	switch ( GameLimit )
	{
		case GAMELIMIT_FRAGS:
			fraglimit.ForceSet( Val, CVAR_Int );
			break;

		case GAMELIMIT_POINTS:
			pointlimit.ForceSet( Val, CVAR_Int );
			break;

		case GAMELIMIT_DUELS:
			duellimit.ForceSet( Val, CVAR_Int );
			break;

		case GAMELIMIT_WINS:
			winlimit.ForceSet( Val, CVAR_Int );
			break;

		case GAMELIMIT_WAVES:
			wavelimit.ForceSet( Val, CVAR_Int );
			break;
	}
}

//*****************************************************************************
//
void GAMEMODE_ReconfigureGameSettings( bool bLockedOnly )
{
	ULONG ulMask = bLockedOnly ? FLAGSET_LOCKEDMASK : FLAGSET_MASK;
	GAMEMODE_s *GameMode = &g_GameModes[g_CurrentGameMode];
	UCVarValue value;

	// [AK] Apply the mask to dmflags, but don't change the values of any unlocked flags.
	value.Int = ( dmflags & ~GameMode->lDMFlags[ulMask] ) | ( GameMode->lDMFlags[FLAGSET_VALUE] & GameMode->lDMFlags[ulMask] );
	dmflags.ForceSet( value, CVAR_Int );

	// ...and dmflags2.
	value.Int = ( dmflags2 & ~GameMode->lDMFlags2[ulMask] ) | ( GameMode->lDMFlags2[FLAGSET_VALUE] & GameMode->lDMFlags2[ulMask] );
	dmflags2.ForceSet( value, CVAR_Int );

	// ...and compatflags.
	value.Int = ( compatflags & ~GameMode->lCompatFlags[ulMask] ) | ( GameMode->lCompatFlags[FLAGSET_VALUE] & GameMode->lCompatFlags[ulMask] );
	compatflags.ForceSet( value, CVAR_Int );

	// ...and compatflags2.
	value.Int = ( compatflags2 & ~GameMode->lCompatFlags2[ulMask] ) | ( GameMode->lCompatFlags2[FLAGSET_VALUE] & GameMode->lCompatFlags2[ulMask] );
	compatflags2.ForceSet( value, CVAR_Int );

	// ...and zadmflags.
	value.Int = ( zadmflags & ~GameMode->lZaDMFlags[ulMask] ) | ( GameMode->lZaDMFlags[FLAGSET_VALUE] & GameMode->lZaDMFlags[ulMask] );
	zadmflags.ForceSet( value, CVAR_Int );

	// ...and zacompatflags.
	value.Int = ( zacompatflags & ~GameMode->lZaCompatFlags[ulMask] ) | ( GameMode->lZaCompatFlags[FLAGSET_VALUE] & GameMode->lZaCompatFlags[ulMask] );
	zacompatflags.ForceSet( value, CVAR_Int );
}