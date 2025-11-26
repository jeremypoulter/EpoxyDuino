/*
 * WiFi Basic Example
 * 
 * Demonstrates basic WiFi functionality with EpoxyMockWiFi library.
 * This example shows how to write code that compiles both on actual
 * Arduino hardware and under EpoxyDuino for testing.
 */

#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "MyNetwork";
const char* password = "MyPassword";

void setup() {
    Serial.begin(115200);
    
    #if defined(EPOXY_DUINO)
    Serial.setLineModeUnix();
    #endif
    
    delay(100);
    
    Serial.println("\n\nWiFi Basic Example");
    Serial.println("===================");
    
    // Print initial status
    Serial.print("WiFi Mode: ");
    Serial.println(WiFi.getMode());
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
    
    // Set hostname
    WiFi.hostname("epoxy-test");
    Serial.print("Hostname: ");
    Serial.println(WiFi.hostname());
    
    // Print MAC address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    
    // Connect to WiFi
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    // Wait for connection (mock library connects immediately)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Subnet mask: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("DNS: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.print("BSSID: ");
        Serial.println(WiFi.BSSIDstr());
        Serial.print("Channel: ");
        Serial.println(WiFi.channel());
    } else {
        Serial.println("Connection failed!");
    }
    
    #if defined(EPOXY_DUINO)
    Serial.println("\nNote: Running under EpoxyDuino with mock WiFi");
    Serial.println("Use WiFi.mockSetStatus() and WiFi.mockSetLocalIP() to simulate states");
    #endif
}

void loop() {
    // Check connection status periodically
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 5000) {
        lastCheck = millis();
        
        Serial.print("WiFi Status: ");
        if (WiFi.isConnected()) {
            Serial.print("Connected to ");
            Serial.print(WiFi.SSID());
            Serial.print(" (");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm)");
        } else {
            Serial.println("Disconnected");
        }
    }
    
    delay(100);
}
