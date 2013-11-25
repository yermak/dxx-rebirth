/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

/*
 *
 * Functions for accessing arguments.
 *
 */

#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <SDL_stdinc.h>
#include "physfsx.h"
#include "args.h"
#include "u_mem.h"
#include "strio.h"
#include "strutil.h"
#include "digi.h"
#include "game.h"
#include "gauges.h"
#include "console.h"
#ifdef USE_UDP
#include "net_udp.h"
#endif

#define MAX_ARGS 1000
#if defined(DXX_BUILD_DESCENT_I)
#define INI_FILENAME "d1x.ini"
#elif defined(DXX_BUILD_DESCENT_II)
#define INI_FILENAME "d2x.ini"
#endif

typedef std::vector<std::string> Arglist;
static Arglist Args;

class argument_exception
{
public:
	const char *arg;
	argument_exception(const char *a) : arg(a) {}
};

class missing_parameter : public argument_exception
{
public:
	missing_parameter(const char *a) : argument_exception(a) {}
};

class unhandled_argument : public argument_exception
{
public:
	unhandled_argument(const char *a) : argument_exception(a) {}
};

class conversion_failure : public argument_exception
{
public:
	const char *value;
	conversion_failure(const char *a, const char *v) : argument_exception(a), value(v) {}
};

struct Arg GameArg;

static void AppendIniArgs(void)
{
	PHYSFS_file *f;
	f = PHYSFSX_openReadBuffered(INI_FILENAME);

	if(f) {
		char line[1024];
		while(!PHYSFS_eof(f) && Args.size() < MAX_ARGS && PHYSFSX_fgets(line, sizeof(line), f))
		{
			static const char separator[] = " ";
			for(char *token = strtok(line, separator); token != NULL; token = strtok(NULL, separator))
				Args.push_back(token);
		}
		PHYSFS_close(f);
	}
}

static const char *arg_string(Arglist::const_iterator &pp, Arglist::const_iterator end)
{
	Arglist::const_iterator arg = pp;
	if (++pp == end)
		throw missing_parameter(arg->c_str());
	return pp->c_str();
}

static long arg_integer(Arglist::const_iterator &pp, Arglist::const_iterator end)
{
	Arglist::const_iterator arg = pp;
	const char *value = arg_string(pp, end);
	char *p;
	long i = strtol(value, &p, 10);
	if (*p)
		throw conversion_failure(arg->c_str(), value);
	return i;
}

static void ReadCmdArgs(void)
{
	GameArg.SysMaxFPS = MAXIMUM_FPS;
#if defined(DXX_BUILD_DESCENT_II)
	GameArg.SndDigiSampleRate = SAMPLE_RATE_22K;
#endif
#ifdef USE_UDP
	GameArg.MplUdpHostAddr = UDP_MANUAL_ADDR_DEFAULT;
#ifdef USE_TRACKER
	GameArg.MplTrackerAddr = TRACKER_ADDR_DEFAULT;
	GameArg.MplTrackerPort = TRACKER_PORT_DEFAULT;
#endif
#endif
	GameArg.DbgVerbose = CON_NORMAL;
	GameArg.DbgBpp 			= 32;
#ifdef OGL
	GameArg.DbgGlIntensity4Ok 	= 1;
	GameArg.DbgGlLuminance4Alpha4Ok = 1;
	GameArg.DbgGlRGBA2Ok 		= 1;
	GameArg.DbgGlReadPixelsOk 	= 1;
	GameArg.DbgGlGetTexLevelParamOk = 1;
#endif
	for (Arglist::const_iterator pp = Args.begin(), end = Args.end(); pp != end; ++pp)
	{
		const char *p = pp->c_str();
	// System Options

		if (!d_stricmp(p, "-help") || !d_stricmp(p, "-h") || !d_stricmp(p, "-?") || !d_stricmp(p, "?"))
			GameArg.SysShowCmdHelp = 1;
		else if (!d_stricmp(p, "-nonicefps"))
			GameArg.SysNoNiceFPS = 1;
		else if (!d_stricmp(p, "-maxfps"))
			GameArg.SysMaxFPS = arg_integer(pp, end);
		else if (!d_stricmp(p, "-hogdir"))
			GameArg.SysHogDir = arg_string(pp, end);
		else if (!d_stricmp(p, "-nohogdir"))
			GameArg.SysNoHogDir = 1;
		else if (!d_stricmp(p, "-use_players_dir"))
			GameArg.SysUsePlayersDir 	= 1;
		else if (!d_stricmp(p, "-lowmem"))
			GameArg.SysLowMem 		= 1;
		else if (!d_stricmp(p, "-pilot"))
			GameArg.SysPilot = arg_string(pp, end);
		else if (!d_stricmp(p, "-window"))
			GameArg.SysWindow 		= 1;
		else if (!d_stricmp(p, "-noborders"))
			GameArg.SysNoBorders 		= 1;
#if defined(DXX_BUILD_DESCENT_I)
		else if (!d_stricmp(p, "-notitles"))
			GameArg.SysNoTitles 		= 1;
#elif defined(DXX_BUILD_DESCENT_II)
		else if (!d_stricmp(p, "-nomovies"))
			GameArg.SysNoMovies 		= 1;
#endif
		else if (!d_stricmp(p, "-autodemo"))
			GameArg.SysAutoDemo 		= 1;

	// Control Options

		else if (!d_stricmp(p, "-nocursor"))
			GameArg.CtlNoCursor 		= 1;
		else if (!d_stricmp(p, "-nomouse"))
			GameArg.CtlNoMouse 		= 1;
		else if (!d_stricmp(p, "-nojoystick"))
			GameArg.CtlNoJoystick 		= 1;
		else if (!d_stricmp(p, "-nostickykeys"))
			GameArg.CtlNoStickyKeys		= 1;

	// Sound Options

		else if (!d_stricmp(p, "-nosound"))
			GameArg.SndNoSound 		= 1;
		else if (!d_stricmp(p, "-nomusic"))
			GameArg.SndNoMusic 		= 1;
#if defined(DXX_BUILD_DESCENT_II)
		else if (!d_stricmp(p, "-sound11k"))
			GameArg.SndDigiSampleRate 		= SAMPLE_RATE_11K;
#endif
#ifdef USE_SDLMIXER
		else if (!d_stricmp(p, "-nosdlmixer"))
			GameArg.SndDisableSdlMixer 	= 1;
#endif

	// Graphics Options

		else if (!d_stricmp(p, "-lowresfont"))
			GameArg.GfxSkipHiresFNT	= 1;
#if defined(DXX_BUILD_DESCENT_II)
		else if (!d_stricmp(p, "-lowresgraphics"))
			GameArg.GfxSkipHiresGFX	= 1;
		else if (!d_stricmp(p, "-lowresmovies"))
			GameArg.GfxSkipHiresMovie 		= 1;
#endif
#ifdef OGL
	// OpenGL Options

		else if (!d_stricmp(p, "-gl_fixedfont"))
			GameArg.OglFixedFont 		= 1;
#endif

	// Multiplayer Options

#ifdef USE_UDP
		else if (!d_stricmp(p, "-udp_hostaddr"))
			GameArg.MplUdpHostAddr = arg_string(pp, end);
		else if (!d_stricmp(p, "-udp_hostport"))
			GameArg.MplUdpHostPort = arg_integer(pp, end);
		else if (!d_stricmp(p, "-udp_myport"))
			GameArg.MplUdpMyPort = arg_integer(pp, end);
#ifdef USE_TRACKER
		else if (!d_stricmp(p, "-tracker_hostaddr"))
			GameArg.MplTrackerAddr = arg_string(pp, end);
		else if (!d_stricmp(p, "-tracker_hostport"))
			GameArg.MplTrackerPort = arg_integer(pp, end);
#endif
#endif

#if defined(DXX_BUILD_DESCENT_I)
		else if (!d_stricmp(p, "-nobm"))
			GameArg.EdiNoBm 		= 1;
#elif defined(DXX_BUILD_DESCENT_II)
#ifdef EDITOR
	// Editor Options

		else if (!d_stricmp(p, "-autoload"))
			GameArg.EdiAutoLoad = arg_string(pp, end);
		else if (!d_stricmp(p, "-macdata"))
			GameArg.EdiMacData 		= 1;
		else if (!d_stricmp(p, "-hoarddata"))
			GameArg.EdiSaveHoardData 	= 1;
#endif
#endif

	// Debug Options

		else if (!d_stricmp(p, "-debug"))
			GameArg.DbgVerbose 	= CON_DEBUG;
		else if (!d_stricmp(p, "-verbose"))
			GameArg.DbgVerbose 	= CON_VERBOSE;

		else if (!d_stricmp(p, "-safelog"))
			GameArg.DbgSafelog 		= 1;
		else if (!d_stricmp(p, "-norun"))
			GameArg.DbgNoRun 		= 1;
		else if (!d_stricmp(p, "-renderstats"))
			GameArg.DbgRenderStats 		= 1;
		else if (!d_stricmp(p, "-text"))
			GameArg.DbgAltTex = arg_string(pp, end);
		else if (!d_stricmp(p, "-tmap"))
			GameArg.DbgTexMap = arg_string(pp, end);
		else if (!d_stricmp(p, "-showmeminfo"))
			GameArg.DbgShowMemInfo 		= 1;
		else if (!d_stricmp(p, "-nodoublebuffer"))
			GameArg.DbgNoDoubleBuffer 	= 1;
		else if (!d_stricmp(p, "-bigpig"))
			GameArg.DbgNoCompressPigBitmap 		= 1;
		else if (!d_stricmp(p, "-16bpp"))
			GameArg.DbgBpp 		= 16;

#ifdef OGL
		else if (!d_stricmp(p, "-gl_oldtexmerge"))
			GameArg.DbgUseOldTextureMerge 		= 1;
		else if (!d_stricmp(p, "-gl_intensity4_ok"))
			GameArg.DbgGlIntensity4Ok 		                       = arg_integer(pp, end);
		else if (!d_stricmp(p, "-gl_luminance4_alpha4_ok"))
			GameArg.DbgGlLuminance4Alpha4Ok                        = arg_integer(pp, end);
		else if (!d_stricmp(p, "-gl_rgba2_ok"))
			GameArg.DbgGlRGBA2Ok 			                       = arg_integer(pp, end);
		else if (!d_stricmp(p, "-gl_readpixels_ok"))
			GameArg.DbgGlReadPixelsOk 		                       = arg_integer(pp, end);
		else if (!d_stricmp(p, "-gl_gettexlevelparam_ok"))
			GameArg.DbgGlGetTexLevelParamOk                        = arg_integer(pp, end);
#else
		else if (!d_stricmp(p, "-hwsurface"))
			GameArg.DbgSdlHWSurface = 1;
		else if (!d_stricmp(p, "-asyncblit"))
			GameArg.DbgSdlASyncBlit = 1;
#endif
		else
			throw unhandled_argument(p);
	}

	if (GameArg.SysMaxFPS < MINIMUM_FPS)
		GameArg.SysMaxFPS = MINIMUM_FPS;
	else if (GameArg.SysMaxFPS > MAXIMUM_FPS)
		GameArg.SysMaxFPS = MAXIMUM_FPS;

	static char sdl_disable_lock_keys[] = "SDL_DISABLE_LOCK_KEYS=0";
	if (GameArg.CtlNoStickyKeys) // Must happen before SDL_Init!
		sdl_disable_lock_keys[sizeof(sdl_disable_lock_keys) - 1] = '1';
	SDL_putenv(sdl_disable_lock_keys);
}

void args_exit(void)
{
	Args.clear();
}

void InitArgs( int argc,char **argv )
{
	int i;

	for (i=1; i < argc; i++ )
		Args.push_back(argv[i]);

	AppendIniArgs();
	try {
		ReadCmdArgs();
	} catch(const missing_parameter& e) {
		Error("Missing parameter for argument \"%s\"", e.arg);
	} catch(const unhandled_argument& e) {
		Error("Unhandled argument \"%s\"", e.arg);
	} catch(const conversion_failure& e) {
		Error("Failed to convert parameter \"%s\" for argument \"%s\"", e.value, e.arg);
	}
}
