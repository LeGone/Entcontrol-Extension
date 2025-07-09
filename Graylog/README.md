# Graylog HTTPS Logging for Entcontrol Extension

## Overview

The Entcontrol Extension now supports sending all logging data to a Graylog server using HTTPS connections. This provides centralized logging capability for better monitoring and debugging.

## Configuration

The following ConVars are available to configure Graylog logging:

- `sm_entcontrol_graylog_server` - Graylog server hostname or IP address (default: empty)
- `sm_entcontrol_graylog_port` - Graylog server port (default: 12201)
- `sm_entcontrol_graylog_enabled` - Enable/disable Graylog logging (0=disabled, 1=enabled)
- `sm_entcontrol_graylog_endpoint` - Graylog GELF endpoint path (default: /gelf)

## Example Configuration

```
sm_entcontrol_graylog_server "graylog.example.com"
sm_entcontrol_graylog_port "12201"
sm_entcontrol_graylog_enabled "1"
sm_entcontrol_graylog_use_https "1"
sm_entcontrol_graylog_endpoint "/gelf"
```

## Features

### GELF Format
All log messages are sent in GELF (Graylog Extended Log Format) JSON format, which includes:
- Timestamp
- Log level (Emergency, Alert, Critical, Error, Warning, Notice, Info, Debug)
- Host information
- Message content
- Source information (file, function, line number for errors)
- Application metadata

### Log Levels
The implementation supports standard syslog levels:
- Emergency (0)
- Alert (1)
- Critical (2)
- Error (3)
- Warning (4)
- Notice (5)
- Info (6)
- Debug (7)

### Logging Points
All existing logging points now send data to Graylog:
- Extension initialization and shutdown messages
- HTTP server errors
- Version check notifications
- Custom Helper::Log::Error calls
- Network communication errors

### HTTPS Support
The implementation uses HTTPS for secure communication with the Graylog server, ensuring log data is encrypted in transit.

### Error Handling
The Graylog logger includes robust error handling:
- Silent failures prevent application crashes if Graylog is unavailable
- Graceful degradation when network issues occur
- Configurable enable/disable functionality

## Technical Details

### Dependencies
- dlib library for network communication
- JSON formatting for GELF messages
- SSL/TLS support for HTTPS connections

### Performance Considerations
- Asynchronous logging to prevent blocking the main thread
- Connection pooling support for high-volume logging
- Configurable retry logic with exponential backoff

### Security
- HTTPS encryption for all log transmissions
- JSON escaping to prevent injection attacks
- Secure handling of sensitive data in log messages

## Usage

1. Configure your Graylog server with an HTTP/HTTPS input
2. Set the appropriate ConVars in your server configuration
3. Enable Graylog logging with `sm_entcontrol_graylog_enabled 1`
4. Monitor logs in your Graylog dashboard

## Troubleshooting

- Ensure the Graylog server is reachable from your game server
- Check that the configured port accepts HTTP/HTTPS GELF messages
- Verify SSL/TLS certificates are properly configured for HTTPS
- Monitor server console for any connection errors