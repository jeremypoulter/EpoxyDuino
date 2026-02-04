# Async Service Discovery Example

Demonstrates the asynchronous mDNS service discovery APIs in EpoxymDNS.

## Features

- **Non-blocking queries**: Start a search and continue running your code
- **Poll-based results**: Check for results whenever you want
- **Full service details**: Hostname, IP address, port, and TXT records
- **Timeout control**: Configure per-query timeout
- **Multiple concurrent queries**: Can run multiple searches in parallel

## Building and Running

### Using EpoxyDuino (native build):

```bash
cd AsyncServiceDiscovery
make
./AsyncServiceDiscovery.out
```

### Using Arduino IDE:

1. Open `AsyncServiceDiscovery.ino`
2. Compile and upload to your board
3. Open serial monitor (115200 baud)

## Usage

The example provides an interactive CLI with these commands:

- **`s`** - Start an async search for `_openevse._tcp` services
- **`q`** - Query the status of active search and get results
- **`h`** - Display help message

## Example Session

```
================================
EpoxymDNS Async Service Discovery
================================

Commands:
  s - Start async search for _openevse._tcp services
  q - Query status / get results from active search
  h - Display help

s

Starting async search for _openevse._tcp services...
(This will not block - you can do other things)
Use 'q' command to check results

Async query started successfully

q

Query status (elapsed: 2341ms):
Still searching... (not complete yet)
Try again in a moment

q

Query status (elapsed: 5012ms):
✓ Query complete!

Found 2 service(s):
    - OpenEVSE @ openevse.local:3000
      IP: 192.168.1.100
      TXT: model=WiFi, version=3.0.0
    - MyCharger @ mycharger.local:3000
      IP: 192.168.1.101
      TXT: model=Gen4, version=2.5.0
```

## API Overview

### Starting a Query

```cpp
mdns_search_once_t* search = mdns_query_async_start(
    NULL,                      // service instance name (NULL for all)
    "_openevse",              // service type
    "_tcp",                   // protocol
    MDNS_IP_PROTOCOL_V4,      // IPv4 or IPv6
    5000,                     // timeout in milliseconds
    10,                       // max results to collect
    NULL                      // callback (NULL to poll instead)
);
```

### Polling for Results

```cpp
mdns_result_t* results = nullptr;

bool isComplete = mdns_query_async_get_results(
    search,          // search handle
    &results,        // output parameter for results
    500              // polling timeout (ms)
);

if (isComplete) {
    // Process results...
    mdns_query_results_free(results);
    mdns_query_async_delete(search);
}
```

### Processing Results

```cpp
for (mdns_result_t* r = results; r; r = r->next) {
    Serial.print("Service: ");
    Serial.println(r->instance_name);
    
    Serial.print("Hostname: ");
    Serial.println(r->hostname);
    
    Serial.print("Port: ");
    Serial.println(r->port);
    
    // Access IP address
    uint32_t ip = r->addr->u_addr.addr;
    
    // Access TXT records
    for (size_t i = 0; i < r->txt_count; i++) {
        Serial.print(r->txt[i].key);
        Serial.print(" = ");
        Serial.println(r->txt[i].value);
    }
}
```

## Key Differences from Synchronous API

### Synchronous (blocking):
```cpp
int n = MDNS.queryService("openevse", "tcp");
// Blocks until query complete or times out

for (int i = 0; i < n; i++) {
    Serial.println(MDNS.hostname(i));
}
```

### Asynchronous (non-blocking):
```cpp
mdns_search_once_t* search = mdns_query_async_start(
    NULL, "_openevse", "_tcp", 
    MDNS_IP_PROTOCOL_V4, 5000, 10, NULL
);

// Do other things while searching...

bool complete = mdns_query_async_get_results(search, &results, 100);
if (complete) {
    // Process results...
}
```

## Benefits

1. **Responsive UIs**: Continue updating displays while searching
2. **Concurrent searches**: Run multiple queries simultaneously
3. **Better resource usage**: Avoid blocking long-running operations
4. **Timeout flexibility**: Control how long to wait between polls
5. **Same result format**: Use identical result processing code

## Memory Management

Always remember to clean up:

```cpp
// Free results first
if (results) {
    mdns_query_results_free(results);
}

// Then delete the search handle
mdns_query_async_delete(search);
```

## See Also

- `ServiceDiscovery` - Synchronous service discovery example
- `ServiceAnnouncing` - How to advertise services
- `mDNS_Web_Server` - Complete web server with mDNS example
