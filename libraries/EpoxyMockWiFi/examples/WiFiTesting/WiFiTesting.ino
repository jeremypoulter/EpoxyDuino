/*
 * WiFi Testing Example
 * 
 * Demonstrates how to use the mock WiFi functions for testing
 * different WiFi scenarios under EpoxyDuino.
 */

#include <Arduino.h>
#include <WiFi.h>

void testWiFiConnection();
void testWiFiDisconnect();
void testStaticIP();
void testAPMode();

void setup() {
    Serial.begin(115200);
    
    #if defined(EPOXY_DUINO)
    Serial.setLineModeUnix();
    #endif
    
    delay(100);
    
    Serial.println("\n\nWiFi Testing Example");
    Serial.println("=====================");
    
    #if defined(EPOXY_DUINO)
    Serial.println("Running under EpoxyDuino - demonstrating mock WiFi functions\n");
    
    testWiFiConnection();
    testWiFiDisconnect();
    testStaticIP();
    testAPMode();
    
    Serial.println("\n=== All Tests Complete ===");
    #else
    Serial.println("This example is designed for EpoxyDuino testing.");
    Serial.println("On real hardware, it won't do anything interesting.");
    #endif
}

void loop() {
    // Nothing to do in loop for this test
    delay(1000);
}

void testWiFiConnection() {
    Serial.println("TEST: WiFi Connection (EPX_OK)");
    Serial.println("------------------------------");
    
    #if defined(EPOXY_DUINO)
    // Reset WiFi state
    WiFi.mockReset();
    
    // Simulate connection process
    Serial.println("Initial status: Disconnected");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    
    // Connect using EPX_OK – guaranteed success, localIP = host IP
    WiFi.begin(EPX_SSID_OK, "anypassword");
    Serial.println("Called WiFi.begin(EPX_SSID_OK)");
    
    Serial.print("Status (expect WL_CONNECTED=3): ");
    Serial.println(WiFi.status());
    Serial.print("Connected: ");
    Serial.println(WiFi.isConnected() ? "Yes" : "No");
    Serial.print("IP (host address): ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    // Manually override IP for testing
    WiFi.mockSetLocalIP(IPAddress(10, 0, 0, 50));
    Serial.print("After mockSetLocalIP: ");
    Serial.println(WiFi.localIP());
    
    Serial.println();
    #endif
}

void testWiFiDisconnect() {
    Serial.println("TEST: WiFi Disconnect / Bad SSIDs");
    Serial.println("----------------------------------");
    
    #if defined(EPOXY_DUINO)
    WiFi.mockReset();

    // EPX_BADPASS – auth failure
    WiFi.begin(EPX_SSID_BADPASS, "wrongpass");
    Serial.print("EPX_BADPASS status (expect WL_CONNECT_FAILED=4): ");
    Serial.println(WiFi.status());

    // EPX_TIMEOUT – simulated timeout
    WiFi.begin(EPX_SSID_TIMEOUT, "anypassword");
    Serial.print("EPX_TIMEOUT status (expect WL_CONNECTION_LOST=5): ");
    Serial.println(WiFi.status());

    // EPX_NOIP – connected but no IP
    WiFi.begin(EPX_SSID_NOIP);
    Serial.print("EPX_NOIP status (expect WL_CONNECTED=3): ");
    Serial.println(WiFi.status());
    Serial.print("EPX_NOIP localIP (expect 0.0.0.0): ");
    Serial.println(WiFi.localIP());

    // Disconnect
    WiFi.begin(EPX_SSID_OK, "anypassword");
    WiFi.disconnect();
    Serial.print("After disconnect: Connected = ");
    Serial.println(WiFi.isConnected() ? "Yes" : "No");
    
    // Mock status override
    WiFi.mockSetStatus(WL_CONNECTION_LOST);
    Serial.print("After mockSetStatus(WL_CONNECTION_LOST): ");
    Serial.println(WiFi.status());
    
    Serial.println();
    #endif
}

void testStaticIP() {
    Serial.println("TEST: Static IP Configuration");
    Serial.println("------------------------------");
    
    #if defined(EPOXY_DUINO)
    WiFi.mockReset();
    
    // Configure static IP before connecting
    IPAddress staticIP(192, 168, 1, 150);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);
    
    WiFi.config(staticIP, gateway, subnet, dns);
    Serial.println("Configured static IP");
    
    WiFi.begin(EPX_SSID_OK, "anypassword");
    
    Serial.print("IP (expect 192.168.1.150): ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    
    Serial.println();
    #endif
}

void testAPMode() {
    Serial.println("TEST: Access Point Mode");
    Serial.println("------------------------");
    
    #if defined(EPOXY_DUINO)
    WiFi.mockReset();
    
    // Start in AP mode
    WiFi.softAP("TestAP", "password123", 6);
    
    Serial.print("Mode (expect WIFI_AP=2): ");
    Serial.println(WiFi.getMode());
    Serial.print("AP SSID: ");
    Serial.println(WiFi.softAPSSID());
    Serial.print("AP IP (expect 192.168.4.1): ");
    Serial.println(WiFi.softAPIP());
    Serial.print("AP MAC: ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("Connected Stations: ");
    Serial.println(WiFi.softAPgetStationNum());
    
    // Test AP+STA mode
    WiFi.begin(EPX_SSID_OK, "anypassword");
    Serial.print("Mode after WiFi.begin() (expect AP+STA=3): ");
    Serial.println(WiFi.getMode());
    
    Serial.println();
    #endif
}

