# EpoxymDNS Service Discovery Example

This example demonstrates how to use EpoxymDNS for mDNS service discovery in native builds.

## Building

```bash
# Using Make
make

# Using PlatformIO (if platformio.ini is added)
pio run -e native
```

## Running

```bash
./ServiceDiscovery.out
```

## Expected Output

The example will:
1. Initialize the MDNS responder with hostname "test-device"
2. Register 2 mock OpenEVSE services with TXT records
3. Query for "_openevse._tcp" services
4. Display discovered services with IP, port, and TXT records
5. Demonstrate host resolution using queryHost()

## Key Features Demonstrated

- `MDNS.begin()` - Initialize MDNS responder
- `MDNS.addMockService()` - Register mock services for testing
- `MDNS.addMockServiceTxt()` - Add TXT records to mock services
- `MDNS.queryService()` - Discover services by type
- `MDNS.hostname()`, `MDNS.IP()`, `MDNS.port()` - Access service details
- `MDNS.numTxt()`, `MDNS.txt()`, `MDNS.txtKey()` - Access TXT records
- `MDNS.queryHost()` - Resolve hostname to IP address
