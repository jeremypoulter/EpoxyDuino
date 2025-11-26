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
    Serial.println("TEST: WiFi Connection");
    Serial.println("---------------------");
    
    #if defined(EPOXY_DUINO)
    // Reset WiFi state
    WiFi.mockReset();
    
    // Simulate connection process
    Serial.println("Initial status: Disconnected");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    
    // Begin connection
    WiFi.begin("TestSSID", "TestPassword");
    Serial.println("Called WiFi.begin()");
    
    // In mock, it connects immediately, but we can simulate delay
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    Serial.print("Connected: ");
    Serial.println(WiFi.isConnected() ? "Yes" : "No");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    // Manually set different IP for testing
    WiFi.mockSetLocalIP(IPAddress(10, 0, 0, 50));
    Serial.print("After mockSetLocalIP: ");
    Serial.println(WiFi.localIP());
    
    Serial.println();
    #endif
}

void testWiFiDisconnect() {
    Serial.println("TEST: WiFi Disconnect");
    Serial.println("---------------------");
    
    #if defined(EPOXY_DUINO)
    // Start connected
    WiFi.begin("TestSSID", "TestPassword");
    Serial.print("Initial: Connected = ");
    Serial.println(WiFi.isConnected() ? "Yes" : "No");
    
    // Disconnect
    WiFi.disconnect();
    Serial.print("After disconnect: Connected = ");
    Serial.println(WiFi.isConnected() ? "Yes" : "No");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    
    // Simulate disconnection state
    WiFi.mockSetStatus(WL_CONNECTION_LOST);
    Serial.print("After simulating connection loss: ");
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
    
    WiFi.begin("TestSSID", "TestPassword");
    
    Serial.print("IP: ");
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
    
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode());
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("AP MAC: ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("Connected Stations: ");
    Serial.println(WiFi.softAPgetStationNum());
    
    // Test AP+STA mode
    WiFi.begin("TestSSID", "TestPassword");
    Serial.print("Mode after WiFi.begin(): ");
    Serial.println(WiFi.getMode());
    Serial.println("(Should be AP+STA)");
    
    Serial.println();
    #endif
}
