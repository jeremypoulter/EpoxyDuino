/*
 * EpoxymDNS Web Server Example
 * 
 * This is an example of advertising an HTTP server via mDNS.
 * The server is accessible via http://esp32.local
 * 
 * This example demonstrates:
 * - Starting an mDNS responder
 * - Adding an HTTP service advertisement
 * - The service is discoverable on the local network
 * 
 * Based on the standard ESPmDNS mDNS_Web_Server example.
 */

#include <Arduino.h>
#include <ESPmDNS.h>

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give serial time to start
  
  Serial.println("\n=== EpoxymDNS Web Server Example ===\n");

  // Initialize MDNS responder
  // The hostname will be "esp32.local"
  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("✓ mDNS responder started");
  Serial.println("  Access server at: http://esp32.local\n");

  // Add HTTP service to MDNS-SD
  // This tells mDNS-capable devices that an HTTP service is available
  MDNS.addService("http", "tcp", 80);
  Serial.println("✓ HTTP service added to mDNS");
  Serial.println("  Service type: _http._tcp");
  Serial.println("  Port: 80\n");

  Serial.println("=== Server Ready ===");
  Serial.println("\nOther devices on the network can now discover this service:");
  Serial.println("  - Browse for _http._tcp services");
  Serial.println("  - Access via http://esp32.local");
  Serial.println("\nNote: In a real application, you would:");
  Serial.println("  1. Connect to WiFi");
  Serial.println("  2. Start a WiFiServer on port 80");
  Serial.println("  3. Handle incoming HTTP requests");
  Serial.println("  4. Call MDNS.update() periodically");
}

void loop() {
  // In a real application, you would handle HTTP requests here
  // The service advertisement is maintained by the system
  delay(1000);
}
