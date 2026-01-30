# EpoxymDNS

Avahi-based ESPmDNS implementation for EpoxyDuino native builds.

## Overview

EpoxymDNS provides mDNS (Multicast DNS) service discovery for native Linux/Mac builds using EpoxyDuino. It is API-compatible with the ESP32 ESPmDNS library while using the Avahi library for real system mDNS integration.

## Features

- **Real mDNS Discovery**: Uses Avahi daemon for actual service discovery on Linux systems
- **Service Discovery**: Query for services using `queryService()`
- **Host Resolution**: Resolve hostnames with `queryHost()`
- **TXT Record Support**: Full access to TXT records associated with services
- **Mock Service Registry**: Register mock services for testing using `addMockService()`
- **API Compatible**: Matches ESP32 ESPmDNS interface exactly

## Requirements

### Build Dependencies

- `libavahi-client-dev` - Avahi client library development files
- `libavahi-common-dev` - Avahi common library development files

### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install libavahi-client-dev libavahi-common-dev
```

**macOS:**
```bash
brew install avahi
```

## Usage

### Basic Service Discovery

```cpp
#include <ESPmDNS.h>

void setup() {
  MDNS.begin("mydevice");
  
  // Query for OpenEVSE services
  int n = MDNS.queryService("openevse", "tcp");
  
  for (int i = 0; i < n; i++) {
    Serial.print("Found: ");
    Serial.print(MDNS.hostname(i));
    Serial.print(" at ");
    Serial.print(MDNS.IP(i));
    Serial.print(":");
    Serial.println(MDNS.port(i));
  }
}
```

### Registering Mock Services (for testing)

```cpp
// In your test setup code:
MDNS.clearMockServices();
MDNS.addMockService("openevse", "tcp", "openevse-1.local", 
                    IPAddress(192, 168, 1, 100), 80);
MDNS.addMockServiceTxt("openevse", "tcp", "openevse-1.local", 
                       "version", "8.2.3");
```

## API Reference

### Service Discovery

- `int queryService(const char* service, const char* proto)` - Query for services
- `IPAddress queryHost(const char* host, uint32_t timeout)` - Resolve hostname to IP

### Result Access

After calling `queryService()`, use these methods to access results:

- `String hostname(int idx)` - Get hostname of result
- `IPAddress IP(int idx)` - Get IP address of result
- `uint16_t port(int idx)` - Get port number of result
- `int numTxt(int idx)` - Get number of TXT records
- `bool hasTxt(int idx, const char* key)` - Check if TXT record exists
- `String txt(int idx, const char* key)` - Get TXT record value by key
- `String txt(int idx, int txtIdx)` - Get TXT record value by index
- `String txtKey(int idx, int txtIdx)` - Get TXT record key by index

### Mock Service Management

These methods are for test simulation:

- `void addMockService(service, proto, hostname, ip, port)` - Add mock service
- `void addMockServiceTxt(service, proto, hostname, key, value)` - Add TXT record
- `void clearMockServices()` - Clear all mock services

### Event Processing

- `void processAvahiEvents(int timeout_ms)` - Process Avahi events (for future integration)

## Implementation Details

### How It Works

1. **Initialization**: `MDNS.begin()` initializes an Avahi client connection
2. **Service Query**: `queryService()` creates a temporary Avahi service browser and resolver
3. **Result Population**: Discovered services are converted to `MDNSService` objects
4. **Mock Fallback**: Mock services are always included in results for testing
5. **Cleanup**: Resources are properly freed after queries

### Avahi Integration

The library uses the Avahi D-Bus interface to discover mDNS services on the local network. Each query:
- Creates an Avahi simple poll for event handling
- Starts a service browser for the requested service type
- Processes events for a configurable timeout (default: 2 seconds)
- Collects discovered services with their TXT records
- Cleans up resources

### Thread Safety

This implementation is not fully thread-safe. Do not call `queryService()` from multiple threads simultaneously. For multi-threaded use cases, implement external synchronization.

### Service Type Format

Service types should be specified as:
- `service` - The service name (e.g., "openevse", "http")
- `proto` - The protocol (e.g., "tcp", "udp")

The library automatically adds underscores, so `queryService("openevse", "tcp")` will search for `_openevse._tcp.local` services.

## Troubleshooting

### Avahi Daemon Not Running

If Avahi is not running on the system, service discovery will fail silently. The library will still return mock services if registered.

**Ubuntu/Debian:**
```bash
sudo systemctl start avahi-daemon
sudo systemctl enable avahi-daemon
```

**macOS:**
The mDNS responder should start automatically.

### Build Errors

If you encounter errors like `avahi.h: No such file`, install the development libraries:
```bash
sudo apt-get install libavahi-client-dev libavahi-common-dev
```

### No Services Found

1. Verify Avahi daemon is running
2. Verify services are actually advertised on the network
3. Check firewall settings (mDNS uses UDP port 5353)
4. Try registering mock services for testing

## Performance Considerations

- Service queries block for the specified timeout (default: 2 seconds)
- Each query creates temporary Avahi resources that are freed afterward
- For frequent queries, consider caching results locally
- Mock services are always checked first and incur minimal overhead

## License

MIT License - Copyright (c) 2025 Jeremy Poulter
