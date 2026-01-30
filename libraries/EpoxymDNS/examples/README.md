# EpoxymDNS Examples

The EpoxymDNS library provides mDNS (Multicast DNS) service discovery and announcement using the Avahi library on native builds. These examples demonstrate the key functionality.

## Examples

### 1. ServiceDiscovery
Demonstrates discovering mDNS services on the local network.

**Features:**
- Searches for OpenEVSE, HTTP, and SSH services
- Displays service details (hostname, IP, port, TXT records)
- Shows how to use `queryService()` and `queryHost()`

**Usage:**
```bash
cd ServiceDiscovery
make
./ServiceDiscovery.out
```

### 2. ServiceAnnouncing
Demonstrates advertising services on the mDNS network using the standard ESPmDNS API.

**Features:**
- Announces HTTP and OpenEVSE services
- Adds TXT record metadata to services
- Uses `addService()` and `addServiceTxt()`

**Usage:**
```bash
cd ServiceAnnouncing
make
./ServiceAnnouncing.out
```

### 3. mDNS_Web_Server
Based on the standard ESPmDNS mDNS_Web_Server example.

**Features:**
- Simple HTTP service announcement
- Shows the minimal setup required
- Compatible with the standard ESP32 ESPmDNS example

**Usage:**
```bash
cd mDNS_Web_Server
make
./mDNS_Web_Server.out
```

## API Compatibility

The EpoxymDNS library maintains API compatibility with the standard ESPmDNS library:

### Service Advertisement
```cpp
MDNS.begin("hostname");           // Start mDNS responder
MDNS.addService("http", "tcp", 80);  // Add service
MDNS.addServiceTxt("http", "tcp", "path", "/");  // Add TXT record
MDNS.removeService("http", "tcp", 80);  // Remove service
```

### Service Discovery
```cpp
int count = MDNS.queryService("http", "tcp");  // Search for services
String hostname = MDNS.hostname(0);  // Get result
IPAddress ip = MDNS.IP(0);
uint16_t port = MDNS.port(0);
```

## Testing

To test the examples together:

1. Start ServiceAnnouncing in one terminal
2. Run ServiceDiscovery in another terminal
3. ServiceDiscovery should find the services announced by ServiceAnnouncing

You can also use `avahi-browse` to verify services:
```bash
avahi-browse -r _http._tcp
avahi-browse -r _openevse._tcp
```

## Requirements

- libavahi-client-dev (Linux/Debian/Ubuntu)
- Avahi daemon running
- EpoxyDuino build environment

## Implementation Notes

The EpoxymDNS library uses:
- **Avahi** for real mDNS service browsing and publishing
- **processAvahiEvents()** to handle asynchronous Avahi callbacks
- Query results cached in `_queryResults` vector

When using in real applications:
- Call `processAvahiEvents()` regularly in the main loop
- Services persist until `removeService()` is called or the program exits
- Discovery queries have configurable timeouts (default 2 seconds)
