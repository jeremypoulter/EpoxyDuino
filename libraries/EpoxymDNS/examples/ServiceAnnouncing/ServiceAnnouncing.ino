/*
 * EpoxymDNS Service Announcing Example
 * 
 * Demonstrates how to advertise services on the mDNS network.
 * This example shows how to add a service with TXT records
 * that other devices can discover.
 * 
 * Based on the standard ESPmDNS mDNS_Web_Server example.
 */

#include <Arduino.h>
#include <ESPmDNS.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EpoxymDNS Service Announcing Example ===\n");

  // Initialize MDNS responder with hostname
  const char* hostname = "mydevice";
  if (MDNS.begin(hostname)) {
    Serial.println("✓ MDNS responder started");
    Serial.print("  Hostname: ");
    Serial.print(hostname);
    Serial.println(".local\n");
  } else {
    Serial.println("✗ Error starting MDNS responder");
    return;
  }

  // Add HTTP service
  Serial.println("Publishing services...\n");
  
  if (MDNS.addService("http", "tcp", 80)) {
    Serial.println("✓ Added HTTP service on port 80");
    MDNS.addServiceTxt("http", "tcp", "product", "EpoxyDuino Demo");
    MDNS.addServiceTxt("http", "tcp", "path", "/");
  } else {
    Serial.println("✗ Failed to add HTTP service");
  }

  // Add OpenEVSE service
  if (MDNS.addService("openevse", "tcp", 80)) {
    Serial.println("✓ Added OpenEVSE service on port 80");
    MDNS.addServiceTxt("openevse", "tcp", "version", "8.2.3");
    MDNS.addServiceTxt("openevse", "tcp", "model", "OpenEVSE v3");
    MDNS.addServiceTxt("openevse", "tcp", "features", "solar,load-sharing");
  } else {
    Serial.println("✗ Failed to add OpenEVSE service");
  }

  Serial.println("\n=== Services Published ===");
  Serial.println("\nOther devices can now discover:");
  Serial.println("  - http://mydevice.local");
  Serial.println("  - _openevse._tcp services\n");
  Serial.println("The device will continue to announce these services");
  Serial.println("until it is shut down or the services are removed.");
}

void loop() {
  // Services remain advertised as long as the program runs
  delay(1000);
}
