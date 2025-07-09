// =============================================================
//
// Entcontrol (helper.cpp)
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

#include "Helper.hpp"

#include <iostream>
#include "Graylog/GraylogLogger.hpp"

namespace Helper
{
	namespace Log
	{
		void Error(const char *file, const char *function, long line, std::string sError)
		{
			std::string sFile(file);
			
#if defined _WIN32
			sFile.erase(0, sFile.rfind('\\')+1);
#else
			sFile.erase(0, sFile.rfind('/')+1);
#endif
			std::cout << sFile << "::" << function <<" Line " << line << " -> " << sError << std::endl;
			
			// Also send to Graylog
			Graylog::Logger::Error(sFile, function, line, sError);
		}
	}

	// Taken from SDKTools
	size_t UTIL_StringToSignature(const char *str, char buffer[], size_t maxlength)
	{
		size_t real_bytes = 0;
		size_t length = strlen(str);

		for (size_t i=0; i<length; i++)
		{
			if (real_bytes >= maxlength)
			{
				break;
			}
			buffer[real_bytes++] = (unsigned char)str[i];
			if (str[i] == '\\'
				&& str[i+1] == 'x')
			{
				if (i + 3 >= length)
				{
					continue;
				}
				/* Get the hex part */
				char s_byte[3];
				int r_byte;
				s_byte[0] = str[i+2];
				s_byte[1] = str[i+3];
				s_byte[2] = '\n';
				/* Read it as an integer */
				sscanf(s_byte, "%x", &r_byte);
				/* Save the value */
				buffer[real_bytes-1] = (unsigned char)r_byte;
				/* Adjust index */
				i += 3;
			}
		}

		return real_bytes;
	}

	bool UTIL_VerifySignature(const void *addr, const char *sig, size_t len)
	{
		unsigned char *addr1 = (unsigned char *) addr;
		unsigned char *addr2 = (unsigned char *) sig;

		for (size_t i = 0; i < len; i++)
		{
			if (addr2[i] == '*')
				continue;
			if (addr1[i] != addr2[i])
				return false;
		}

		return true;
	}

	/*
	IServer *GetIServer()
	{
#if defined PLATFORM_WINDOWS
		const char *sigstr;
		char sig[32];
		size_t siglen;
		int offset;
		void *vfunc = NULL;

		// Get the CreateFakeClient function pointer
		if (!(vfunc=SH_GET_ORIG_VFNPTR_ENTRY(engine, &IVEngineServer::CreateFakeClient)))
		{
			return;
		}

		// Get signature string for IVEngineServer::CreateFakeClient()
		sigstr = gameconfs->->GetKeyValue("CreateFakeClient_Windows");

		if (!sigstr)
		{
			return;
		}

		// Convert signature string to signature bytes
		siglen = UTIL_StringToSignature(sigstr, sig, sizeof(sig));

		// Check if we're on the expected function
		if (!UTIL_VerifySignature(vfunc, sig, siglen))
		{
			return;
		}

		// Get the offset into CreateFakeClient
		if (!g_pGameConf->GetOffset("sv", &offset))
		{
			return;
		}

		// Finally we have the interface we were looking for
		return (reinterpret_cast<IServer **>(reinterpret_cast<unsigned char *>(vfunc) + offset));

#elif defined PLATFORM_POSIX
		void *addr;
		if (!g_pGameConf->GetMemSig("sv", &addr) || !addr)
		{
			return;
		}

		return (reinterpret_cast<IServer *>(addr));
#endif
	}
	*/
}
