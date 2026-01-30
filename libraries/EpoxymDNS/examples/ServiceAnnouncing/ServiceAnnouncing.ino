/*
 * EpoxymDNS Service Announcing Example
 * 
 * Demonstrates how to advertise services on the mDNS network.
 * This example shows how to publish a web service with TXT records
 * that other devices can discover.
 */

#include <Arduino.h>
#include <ESPmDNS.h>

// Example service configuration
struct ServiceConfig {
  const char* name;
  const char* service;
  const char* proto;
  uint16_t port;
  struct {
    const char* key;
    const char* value;
  } txtRecords[5];
  int txtCount;
};

// Configure multiple services to announce
ServiceConfig services[] = {
  {
    name: "MyCharger",
    service: "openevse",
    proto: "tcp",
    port: 80,
    txtRecords: {
      {"version", "8.2.3"},
      {"id", "abc123def"},
      {"model", "OpenEVSE v3"},
      {"features", "solar,load-sharing"},
      {}
    },
    txtCount: 4
  },
  {
    name: "MyWebServer",
    service: "http",
    proto: "tcp",
    port: 8080,
    txtRecords: {
      {"path", "/"},
      {"product", "EpoxyDuino Demo"},
      {}
    },
    txtCount: 2
  }
};
const int numServices = sizeof(services) / sizeof(services[0]);

void printServiceStatus(const ServiceConfig& svc) {
  Serial.print("\n  Service Name: ");
  Serial.println(svc.name);
  Serial.print("  Service Type: _");
  Serial.print(svc.service);
  Serial.print("._");
  Serial.println(svc.proto);
  Serial.print("  Port: ");
  Serial.println(svc.port);
  
  if (svc.txtCount > 0) {
    Serial.println("  TXT Records:");
    for (int i = 0; i < svc.txtCount; i++) {
      Serial.print("    ");
      Serial.print(svc.txtRecords[i].key);
      Serial.print(" = ");
      Serial.println(svc.txtRecords[i].value);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EpoxymDNS Service Announcing Example ===\n");

  // Initialize MDNS with hostname
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

  // Announce services
  Serial.println("Publishing services on the network...\n");
  
  bool allSuccess = true;
  for (int i = 0; i < numServices; i++) {
    const ServiceConfig& svc = services[i];
    
    // Publish the service
    bool serviceAdded = MDNS.publishService(
      svc.name,
      svc.service,
      svc.proto,
      svc.port
    );
    
    if (serviceAdded) {
      Serial.print("✓ Published service: ");
      Serial.println(svc.name);
      
      // Add TXT records
      for (int j = 0; j < svc.txtCount; j++) {
        MDNS.addServiceTxt(
          svc.service,
          svc.proto,
          svc.txtRecords[j].key,
          svc.txtRecords[j].value
        );
      }
    } else {
      Serial.print("✗ Failed to publish service: ");
      Serial.println(svc.name);
      allSuccess = false;
    }
  }

  if (allSuccess) {
    Serial.println("\n✓ All services published successfully!\n");
    Serial.println("Service details:");
    for (int i = 0; i < numServices; i++) {
      printServiceStatus(services[i]);
    }
  }

  Serial.println("\n=== Announcing Active ===");
  Serial.println("\nOther devices can now discover these services:");
  for (int i = 0; i < numServices; i++) {
    Serial.print("  - ");
    Serial.print(services[i].name);
    Serial.print(" (_");
    Serial.print(services[i].service);
    Serial.print("._");
    Serial.print(services[i].proto);
    Serial.println(")");
  }
  Serial.println("\nThe device will continue to announce these services");
  Serial.println("until it is shut down or the services are unpublished.");
}

void loop() {
  // Keep the services alive by processing Avahi events
  MDNS.processAvahiEvents(100);  // Process events with 100ms timeout
  delay(900);  // Total 1 second between iterations
}
