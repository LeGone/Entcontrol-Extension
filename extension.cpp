// =============================================================
//
// Entcontrol (extension.cpp)
// Copyright Raffael Holz aka. LeGone. All rights reserved.
// http://www.legone.name
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License, version 3.0, as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.
//
// =============================================================

#include "extension.h"

// Webserver
#include "Webserver/HTTP.hpp"

// Updater
#include "LGN/LGN.hpp"

// Graylog Logger
#include "Graylog/GraylogLogger.hpp"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

EntControl gEntControl;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&gEntControl);

ICvar *icvar;
#if SOURCE_ENGINE >= SE_LEFT4DEAD
	CGlobalVars *gpGlobals = NULL;
#endif

extern sp_nativeinfo_t natives[];

ISDKTools *g_pSDKTools;

ConVar sm_entcontrol_http_port("sm_entcontrol_http_port", "0", FCVAR_NOTIFY, "0=use gameserverport(windows only|e.g.27015).");
ConVar sm_entcontrol_http_enabled("sm_entcontrol_http_enabled", "0", FCVAR_NOTIFY, "0=disable webserver|1=enable/start webserver");
ConVar sm_entcontrol_graylog_server("sm_entcontrol_graylog_server", "", FCVAR_NOTIFY, "Graylog server hostname or IP address");
ConVar sm_entcontrol_graylog_port("sm_entcontrol_graylog_port", "12201", FCVAR_NOTIFY, "Graylog server port (default 12201)");
ConVar sm_entcontrol_graylog_enabled("sm_entcontrol_graylog_enabled", "0", FCVAR_NOTIFY, "0=disable graylog|1=enable graylog logging");
ConVar sm_entcontrol_graylog_use_https("sm_entcontrol_graylog_use_https", "1", FCVAR_NOTIFY, "0=use HTTP|1=use HTTPS for graylog communication");
ConVar sm_entcontrol_graylog_endpoint("sm_entcontrol_graylog_endpoint", "/gelf", FCVAR_NOTIFY, "Graylog GELF endpoint path (default /gelf)");

void EntControl::SDK_OnAllLoaded()
{
	sharesys->AddNatives(myself, natives);
	sharesys->AddDependency(myself, "sdktools.ext", false, true);

	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
	if (g_pSDKTools == NULL)
	{
		smutils->LogError(myself, "SDKTools interface not found. TerminateRound native disabled.");
		Graylog::Logger::Critical("SDKTools interface not found. TerminateRound native disabled.");
	}
	else if (g_pSDKTools->GetInterfaceVersion() < 2)
	{
		//<psychonic> THIS ISN'T DA LIMBO STICK. LOW IS BAD
		smutils->LogError(myself, "SDKTools interface is outdated. TerminateRound native disabled.");
		Graylog::Logger::Critical("SDKTools interface is outdated. TerminateRound native disabled.");
	}
	
	HTTP::CreateForwards();
	Graylog::Logger::Initialize();
}

void EntControl::SDK_OnUnload()
{
	HTTP::Stop();
	HTTP::ReleaseForwards();
	Graylog::Logger::Shutdown();
}

class BaseAccessor : public IConCommandBaseAccessor
{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase)
	{
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

void EntControl::OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax)
{
	std::string newVersion;

	newVersion = LGN::get_most_recent_version();
	if (!newVersion.empty())
	{
		if (newVersion != SMEXT_CONF_VERSION)
		{
			std::string message = "New Entcontrol-Version! Your version:" + std::string(SMEXT_CONF_VERSION) + " New version:" + newVersion;
			smutils->LogMessage(myself, message.c_str());
			smutils->LogMessage(myself, "Please download the latest version from https://forums.alliedmods.net/showthread.php?t=157075 or take a look at http://www.legone.name/entcontrol");
			Graylog::Logger::Info(message);
			Graylog::Logger::Info("Please download the latest version from https://forums.alliedmods.net/showthread.php?t=157075 or take a look at http://www.legone.name/entcontrol");
		}
	}

	if (sm_entcontrol_http_enabled.GetBool())
	{
		// Get the port
		unsigned short port = sm_entcontrol_http_port.GetInt();

		// Port is null? Then use the Gameport.
		// DOES work under windows.
		if (port == 0)
			port = g_pSDKTools->GetIServer()->GetUDPPort();

		HTTP::Start(port);

		// Finally set the actually ip
		std::string ip_address = LGN::get_ip_address();
		if (!ip_address.empty())
			HTTP::SetIP(ip_address);
	}
	else
	{
		HTTP::Stop();
	}
}

bool EntControl::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	//g_pCVar = icvar;

#if SOURCE_ENGINE >= SE_LEFT4DEAD
	gpGlobals = ismm->GetCGlobals();
#endif

	ConVar_Register(0, &s_BaseAccessor);

	return true;
}