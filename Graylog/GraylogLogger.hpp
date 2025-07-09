// =============================================================
//
// Entcontrol (GraylogLogger.hpp)
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

#pragma once

#include <string>
#include <ctime>

namespace Graylog
{
	enum LogLevel
	{
		EMERGENCY = 0,
		ALERT = 1,
		CRITICAL = 2,
		ERROR = 3,
		WARNING = 4,
		NOTICE = 5,
		INFO = 6,
		DEBUG = 7
	};

	class Logger
	{
	public:
		static void Initialize();
		static void Shutdown();
		static void Log(LogLevel level, const std::string& message, const std::string& source = "");
		static void Error(const std::string& file, const std::string& function, long line, const std::string& message);
		static void Info(const std::string& message);
		static void Warning(const std::string& message);
		static void Critical(const std::string& message);
		
	private:
		static std::string CreateGELFMessage(LogLevel level, const std::string& message, const std::string& source = "");
		static void SendToGraylog(const std::string& gelfMessage);
		static std::string GetTimestamp();
		static std::string GetHostname();
		static std::string EscapeJSON(const std::string& input);
	};
}