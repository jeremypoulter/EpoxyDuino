/*
 * EpoxymDNS Service Discovery Example
 * 
 * Demonstrates real mDNS service discovery using Avahi library.
 * Searches for services on the local network and displays their details.
 */

#include <Arduino.h>
#include <ESPmDNS.h>

void printServiceInfo(int serviceIndex) {
  Serial.print("  Service #");
  Serial.print(serviceIndex + 1);
  Serial.println(":");
  
  Serial.print("    Hostname: ");
  Serial.println(MDNS.hostname(serviceIndex));
  
  Serial.print("    IP:       ");
  Serial.println(MDNS.IP(serviceIndex));
  
  Serial.print("    Port:     ");
  Serial.println(MDNS.port(serviceIndex));
  
  int numTxt = MDNS.numTxt(serviceIndex);
  if (numTxt > 0) {
    Serial.print("    TXT:      ");
    Serial.print(numTxt);
    Serial.println(" record(s)");
    
    for (int j = 0; j < numTxt; j++) {
      Serial.print("              ");
      Serial.print(MDNS.txtKey(serviceIndex, j));
      Serial.print(" = ");
      Serial.println(MDNS.txt(serviceIndex, j));
    }
  }
  
  Serial.println();
}

void discoverService(const char* service, const char* proto) {
  Serial.print("\nSearching for _");
  Serial.print(service);
  Serial.print("._");
  Serial.print(proto);
  Serial.println(" services...");
  Serial.println("(querying network, please wait)");
  
  int n = MDNS.queryService(service, proto);
  
  // Retry a few times to allow Avahi to complete the query
  for (int i = 0; i < 20 && n == 0; i++) {
    delay(100);
    n = MDNS.queryService(service, proto);
  }
  
  Serial.println();
  if (n == 0) {
    Serial.println("✗ No services found");
  } else {
    Serial.print("✓ Found ");
    Serial.print(n);
    Serial.println(" service(s):\n");
    
    for (int i = 0; i < n; i++) {
      printServiceInfo(i);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EpoxymDNS Service Discovery Example ===\n");

  // Initialize MDNS with hostname
  if (MDNS.begin("discovery-device")) {
    Serial.println("✓ MDNS responder started");
    Serial.println("  Hostname: discovery-device.local\n");
  } else {
    Serial.println("✗ Error starting MDNS responder");
    return;
  }

  // Discover OpenEVSE services
  discoverService("openevse", "tcp");
  
  // Discover HTTP services
  discoverService("http", "tcp");
  
  // Discover SSH services
  discoverService("ssh", "tcp");
  
  // Example: Resolve a specific hostname
  Serial.println("Testing host resolution (example hostname)...");
  IPAddress ip = MDNS.queryHost("raspberrypi.local");
  if (ip != IPAddress(0, 0, 0, 0)) {
    Serial.print("✓ raspberrypi.local resolved to: ");
    Serial.println(ip);
  } else {
    Serial.println("✗ raspberrypi.local not found (or not on network)");
  }
  
  Serial.println("\n=== Example Complete ===");
  Serial.println("\nNotes:");
  Serial.println("- This uses Avahi for real mDNS discovery on the network");
  Serial.println("- Requires libavahi-client-dev on Linux systems");
  Serial.println("- Avahi daemon must be running for real service discovery");
  Serial.println("- Add your own services to search for by calling discoverService()");
}

void loop() {
  // Nothing to do - all discovery happens in setup()
  delay(1000);
}
