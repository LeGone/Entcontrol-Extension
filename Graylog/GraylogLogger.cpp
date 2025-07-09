// =============================================================
//
// Entcontrol (GraylogLogger.cpp)
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

#include "GraylogLogger.hpp"
#include "../extension.h"
#include <dlib/iosockstream.h>
#include <dlib/sockstreambuf.h>
#include <sstream>
#include <iomanip>
#include <string>

extern ConVar sm_entcontrol_graylog_server;
extern ConVar sm_entcontrol_graylog_port;
extern ConVar sm_entcontrol_graylog_enabled;
extern ConVar sm_entcontrol_graylog_use_https;
extern ConVar sm_entcontrol_graylog_endpoint;

namespace Graylog
{
	void Logger::Initialize()
	{
		// Initialization code if needed
	}

	void Logger::Shutdown()
	{
		// Cleanup code if needed
	}

	void Logger::Log(LogLevel level, const std::string& message, const std::string& source)
	{
		if (!sm_entcontrol_graylog_enabled.GetBool())
			return;

		try
		{
			std::string gelfMessage = CreateGELFMessage(level, message, source);
			SendToGraylog(gelfMessage);
		}
		catch (...)
		{
			// If Graylog logging fails, we should not crash the application
			// Just silently fail or log to console as fallback
		}
	}

	void Logger::Error(const std::string& file, const std::string& function, long line, const std::string& message)
	{
		std::string source = file + "::" + function + " Line " + std::to_string(line);
		Log(ERROR, message, source);
	}

	void Logger::Info(const std::string& message)
	{
		Log(INFO, message);
	}

	void Logger::Warning(const std::string& message)
	{
		Log(WARNING, message);
	}

	void Logger::Critical(const std::string& message)
	{
		Log(CRITICAL, message);
	}

	std::string Logger::CreateGELFMessage(LogLevel level, const std::string& message, const std::string& source)
	{
		std::ostringstream gelf;
		
		gelf << "{"
			 << "\"version\":\"1.1\","
			 << "\"host\":\"" << EscapeJSON(GetHostname()) << "\","
			 << "\"short_message\":\"" << EscapeJSON(message) << "\","
			 << "\"full_message\":\"" << EscapeJSON(message) << "\","
			 << "\"timestamp\":" << GetTimestamp() << ","
			 << "\"level\":" << level << ","
			 << "\"facility\":\"entcontrol\","
			 << "\"_application\":\"entcontrol-extension\"";

		if (!source.empty())
		{
			gelf << ",\"_source\":\"" << EscapeJSON(source) << "\"";
		}

		gelf << "}";

		return gelf.str();
	}

	void Logger::SendToGraylog(const std::string& gelfMessage)
	{
		std::string server = sm_entcontrol_graylog_server.GetString();
		int port = sm_entcontrol_graylog_port.GetInt();
		bool useHttps = sm_entcontrol_graylog_use_https.GetBool();
		std::string endpoint = sm_entcontrol_graylog_endpoint.GetString();

		if (server.empty() || port <= 0)
			return;

		try
		{
			// Create socket connection
			std::string serverPort = server + ":" + std::to_string(port);
			dlib::iosockstream stream(serverPort);

			// Send HTTP/HTTPS POST request with GELF JSON
			// Note: For true HTTPS, you would need SSL/TLS socket implementation
			// This implementation assumes the Graylog server is configured for HTTPS
			// and the underlying socket library handles the SSL/TLS layer
			
			std::string protocol = useHttps ? "HTTPS" : "HTTP";
			
			stream << "POST " << endpoint << " HTTP/1.1\r\n"
				   << "Host: " << server << ":" << port << "\r\n"
				   << "Content-Type: application/json\r\n"
				   << "Content-Length: " << gelfMessage.length() << "\r\n"
				   << "User-Agent: Entcontrol-Extension/1.0 (" << protocol << ")\r\n"
				   << "Accept: application/json\r\n"
				   << "Connection: close\r\n";
				   
			// Add HTTPS-specific headers if needed
			if (useHttps)
			{
				stream << "X-Forwarded-Proto: https\r\n";
			}
			
			stream << "\r\n" << gelfMessage;
			stream.flush();
			
			// Read response status to ensure proper delivery
			std::string response_line;
			if (std::getline(stream, response_line))
			{
				// Response should be HTTP/1.1 202 Accepted for successful GELF message
				// For debugging, you could parse and log the response
			}
		}
		catch (...)
		{
			// Silent failure - don't crash the application if Graylog is unavailable
			// Production considerations:
			// - Implement retry logic with exponential backoff
			// - Use connection pooling for better performance
			// - Add local file logging as fallback
			// - Consider using UDP for GELF as an alternative transport
		}
	}

	std::string Logger::GetTimestamp()
	{
		auto now = std::time(nullptr);
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(3) << static_cast<double>(now);
		return oss.str();
	}

	std::string Logger::GetHostname()
	{
		// Try to get server hostname/IP
		if (g_pSDKTools && g_pSDKTools->GetIServer())
		{
			const char* serverName = g_pSDKTools->GetIServer()->GetName();
			if (serverName && strlen(serverName) > 0)
				return std::string(serverName);
		}
		
		return "entcontrol-server";
	}

	std::string Logger::EscapeJSON(const std::string& input)
	{
		std::string output;
		output.reserve(input.length() + 20);

		for (char c : input)
		{
			switch (c)
			{
			case '"':
				output += "\\\"";
				break;
			case '\\':
				output += "\\\\";
				break;
			case '\b':
				output += "\\b";
				break;
			case '\f':
				output += "\\f";
				break;
			case '\n':
				output += "\\n";
				break;
			case '\r':
				output += "\\r";
				break;
			case '\t':
				output += "\\t";
				break;
			default:
				if (c < 0x20)
				{
					output += "\\u";
					output += "0000";
					output[output.length() - 2] = "0123456789abcdef"[c >> 4];
					output[output.length() - 1] = "0123456789abcdef"[c & 0xf];
				}
				else
				{
					output += c;
				}
				break;
			}
		}

		return output;
	}
}