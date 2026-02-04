/*
 * EpoxymDNS Async Service Discovery Example
 * 
 * Demonstrates asynchronous mDNS service discovery using the async API.
 * This allows non-blocking queries where you can poll for results instead of
 * waiting for the entire query to complete.
 * 
 * Useful for:
 * - Long-running queries that shouldn't block the main loop
 * - Multiple concurrent queries
 * - Responsive UIs that need to update while searching
 */

#include <Arduino.h>
#include <ESPmDNS.h>

// Global state for async query demo
mdns_search_once_t* activeSearch = nullptr;
unsigned long queryStartTime = 0;
unsigned long lastQueryTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("EpoxymDNS Async Service Discovery");
  Serial.println("================================\n");
  
  // Initialize mDNS
  MDNS.begin("mydevice");
  Serial.println("mDNS initialized");
  Serial.println("Starting automatic search (every 2 seconds)...\n");
}

void printResultInfo(mdns_result_t* result) {
  if (!result) {
    Serial.println("    (No result)");
    return;
  }
  
  Serial.print("    - ");
  if (result->instance_name) {
    Serial.print(result->instance_name);
    Serial.print(" @ ");
  }
  if (result->hostname) {
    Serial.print(result->hostname);
  }
  Serial.print(":");
  Serial.println(result->port);
  
  // Print IP address
  if (result->addr) {
    Serial.print("      IP: ");
    // Simple IPv4 display
    uint32_t ip = result->addr->u_addr.addr;
    Serial.print((ip & 0xFF));
    Serial.print(".");
    Serial.print((ip >> 8) & 0xFF);
    Serial.print(".");
    Serial.print((ip >> 16) & 0xFF);
    Serial.print(".");
    Serial.println((ip >> 24) & 0xFF);
  }
  
  // Print TXT records if present
  if (result->txt_count > 0) {
    Serial.print("      TXT: ");
    for (size_t i = 0; i < result->txt_count; i++) {
      if (i > 0) Serial.print(", ");
      if (result->txt[i].key) {
        Serial.print(result->txt[i].key);
        Serial.print("=");
      }
      if (result->txt[i].value) {
        Serial.print(result->txt[i].value);
      }
    }
    Serial.println();
  }
}

void startAsyncSearch() {
  if (activeSearch) {
    // Clean up old search
    mdns_query_async_delete(activeSearch);
    activeSearch = nullptr;
  }
  
  Serial.println("\n--- Starting search for _openevse._tcp services ---");
  Serial.println("Calling mdns_query_async_start...");
  
  // Start an async query
  // Parameters:
  //   - name: NULL (search for all instances)
  //   - service_type: "openevse" (without leading underscore)
  //   - proto: "tcp" (without leading underscore)
  //   - type: MDNS_IP_PROTOCOL_V4 (IPv4 only)
  //   - timeout_ms: 2000 (2 second query timeout)
  //   - max_results: 10 (collect up to 10 results)
  //   - notifier: NULL (no callback, we'll poll instead)
  activeSearch = mdns_query_async_start(
      NULL,
      "openevse",
      "tcp",
      MDNS_IP_PROTOCOL_V4,
      2000,
      10,
      NULL
  );
  
  if (!activeSearch) {
    Serial.println("ERROR: Failed to start async query!");
    return;
  }
  
  Serial.println("Search started successfully");
  queryStartTime = millis();
}

void queryAsyncStatus() {
  if (!activeSearch) {
    Serial.println("No active search.");
    return;
  }
  
  mdns_result_t* results = nullptr;
  
  // Poll for results with 100ms timeout
  bool isComplete = mdns_query_async_get_results(
      activeSearch,
      &results,
      100  // 100ms polling timeout
  );
  
  if (isComplete) {
    unsigned long elapsed = millis() - queryStartTime;
    
    // Process results
    int resultCount = 0;
    for (mdns_result_t* r = results; r; r = r->next) {
      resultCount++;
    }
    
    Serial.print("Query complete (");
    Serial.print(elapsed);
    Serial.print("ms) - Found ");
    Serial.print(resultCount);
    Serial.println(" service(s):");
    
    if (resultCount == 0) {
      Serial.println("  (No services found)");
    } else {
      for (mdns_result_t* r = results; r; r = r->next) {
        printResultInfo(r);
      }
    }
    
    // Clean up
    if (results) {
      mdns_query_results_free(results);
    }
    
    // Delete the search handle
    mdns_query_async_delete(activeSearch);
    activeSearch = nullptr;
  }
}

void loop() {
  unsigned long now = millis();
  
  // Start a new search every 2 seconds
  if (!activeSearch && (now - lastQueryTime >= 2000)) {
    lastQueryTime = now;
    startAsyncSearch();
  }
  
  // Check for results
  if (activeSearch) {
    queryAsyncStatus();
  }
  
  delay(100);
}
