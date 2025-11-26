/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 * Copyright (c) 2025 Brian T. Park
 */

/**
 * @file Mock implementation of WiFi libraries for Arduino platforms
 * to allow code written against ESP8266WiFi, ESP32 WiFi, and other
 * Arduino WiFi APIs to compile under EpoxyDuino.
 * 
 * This implementation provides a unified API that is compatible with
 * multiple Arduino WiFi implementations, with a focus on ESP8266 and ESP32.
 */

#ifndef EPOXY_MOCK_WIFI_H
#define EPOXY_MOCK_WIFI_H

#include <Arduino.h>
#include <IPAddress.h>
#include <Print.h>
#include <functional>

//-----------------------------------------------------------------------------
// WiFi Status Codes (common across platforms)
//-----------------------------------------------------------------------------

typedef enum {
    WL_NO_SHIELD        = 255,  // For compatibility with Arduino WiFi
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;

typedef enum {
    WL_NO_SHIELD_UNUSED = 255,
    WL_SCAN_RUNNING = 254
} wl_scan_status_t;

//-----------------------------------------------------------------------------
// WiFi Modes
//-----------------------------------------------------------------------------

typedef enum {
    WIFI_OFF = 0,
    WIFI_STA = 1,
    WIFI_AP = 2,
    WIFI_AP_STA = 3
} WiFiMode_t;

// ESP8266 compatibility
#define WIFI_MODE_NULL WIFI_OFF

//-----------------------------------------------------------------------------
// WiFi Encryption Types
//-----------------------------------------------------------------------------

// ESP8266 style
typedef enum {
    ENC_TYPE_WEP = 5,
    ENC_TYPE_TKIP = 2,
    ENC_TYPE_CCMP = 4,
    ENC_TYPE_NONE = 7,
    ENC_TYPE_AUTO = 8
} wl_enc_type;

// ESP32 style (wifi_auth_mode_t)
typedef enum {
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK,
    WIFI_AUTH_WPA2_ENTERPRISE,
    WIFI_AUTH_WPA3_PSK,
    WIFI_AUTH_WPA2_WPA3_PSK,
    WIFI_AUTH_WAPI_PSK,
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

//-----------------------------------------------------------------------------
// WiFi Power Save Modes
//-----------------------------------------------------------------------------

typedef enum {
    WIFI_PS_NONE = 0,
    WIFI_PS_MIN_MODEM,
    WIFI_PS_MAX_MODEM
} wifi_ps_type_t;

//-----------------------------------------------------------------------------
// WiFi Events (ESP8266/ESP32)
//-----------------------------------------------------------------------------

typedef enum {
    WIFI_EVENT_STAMODE_CONNECTED = 0,
    WIFI_EVENT_STAMODE_DISCONNECTED,
    WIFI_EVENT_STAMODE_AUTHMODE_CHANGE,
    WIFI_EVENT_STAMODE_GOT_IP,
    WIFI_EVENT_STAMODE_DHCP_TIMEOUT,
    WIFI_EVENT_SOFTAPMODE_STACONNECTED,
    WIFI_EVENT_SOFTAPMODE_STADISCONNECTED,
    WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED,
    WIFI_EVENT_MAX
} WiFiEvent_t;

// Function pointer type for WiFi event handlers
typedef void (*WiFiEventHandler)(WiFiEvent_t event);
typedef std::function<void(WiFiEvent_t)> WiFiEventCb;

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class WiFiClient;
class WiFiServer;

//-----------------------------------------------------------------------------
// WiFiClass - Main WiFi control class
//-----------------------------------------------------------------------------

class WiFiClass {
private:
    wl_status_t _status;
    WiFiMode_t _mode;
    IPAddress _localIP;
    IPAddress _subnetMask;
    IPAddress _gatewayIP;
    IPAddress _dnsIP;
    String _ssid;
    String _psk;
    String _hostname;
    int8_t _channel;
    uint8_t _macAddress[6];
    bool _autoConnect;
    bool _persistent;

public:
    WiFiClass();

    //-------------------------------------------------------------------------
    // Station Mode - Connection Management
    //-------------------------------------------------------------------------

    /**
     * Start WiFi connection for OPEN networks
     * @param ssid Pointer to the SSID string.
     */
    wl_status_t begin(const char* ssid);

    /**
     * Start WiFi connection with WPA/WPA2 encryption
     * @param ssid Pointer to the SSID string.
     * @param passphrase Passphrase. Valid characters in a passphrase must be between ASCII 32-126 (decimal).
     */
    wl_status_t begin(const char* ssid, const char *passphrase);

    /**
     * Start WiFi connection with static IP configuration
     */
    wl_status_t begin(const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect);

    /**
     * Configure static IP address (call before begin)
     */
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1);
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2);

    /**
     * Disconnect from the network
     * @param wifioff if true, turn off WiFi radio
     */
    bool disconnect(bool wifioff = false);
    
    /**
     * Check if connected to WiFi network
     */
    bool isConnected();

    /**
     * Set whether module will attempt to reconnect to an access point in case it is disconnected
     */
    bool setAutoReconnect(bool autoReconnect);
    
    /**
     * Get the auto reconnect policy
     */
    bool getAutoReconnect();

    /**
     * Set whether module will save WiFi config in flash
     */
    bool persistent(bool persistent);
    bool isPersistent();

    //-------------------------------------------------------------------------
    // Station Mode - Information
    //-------------------------------------------------------------------------

    /**
     * Get the station interface MAC address
     */
    uint8_t* macAddress(uint8_t* mac);
    String macAddress();

    /**
     * Get the interface IP address
     */
    IPAddress localIP();

    /**
     * Get the subnet mask
     */
    IPAddress subnetMask();

    /**
     * Get the gateway IP address
     */
    IPAddress gatewayIP();

    /**
     * Get the DNS server IP address
     */
    IPAddress dnsIP(uint8_t dns_no = 0);

    /**
     * Get current SSID
     */
    String SSID() const;

    /**
     * Get current network RSSI (signal strength)
     */
    int32_t RSSI();

    /**
     * Get the current BSSID
     */
    uint8_t* BSSID();
    String BSSIDstr();

    /**
     * Get the channel of the current network
     */
    int32_t channel();

    /**
     * Return Connection status
     */
    wl_status_t status();

    //-------------------------------------------------------------------------
    // Station Mode - Hostname
    //-------------------------------------------------------------------------

    /**
     * Set the hostname for DHCP
     */
    bool hostname(const String& aHostname);
    bool hostname(const char* aHostname);
    bool setHostname(const char* hostname);  // ESP32 style

    /**
     * Get the hostname
     */
    String hostname();
    const char* getHostname();  // ESP32 style

    //-------------------------------------------------------------------------
    // Access Point Mode
    //-------------------------------------------------------------------------

    /**
     * Set up an access point
     * @param ssid SSID of the AP
     * @param passphrase password, NULL for open network
     * @param channel WiFi channel (1-13)
     * @param ssid_hidden hide SSID
     * @param max_connection maximum simultaneous connected clients (1-4)
     */
    bool softAP(const char* ssid, const char* passphrase = nullptr, int channel = 1, int ssid_hidden = 0, int max_connection = 4);

    /**
     * Configure AP IP address
     */
    bool softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);

    /**
     * Get AP IP address
     */
    IPAddress softAPIP();

    /**
     * Get AP MAC address
     */
    uint8_t* softAPmacAddress(uint8_t* mac);
    String softAPmacAddress();

    /**
     * Get the count of stations connected to the AP
     */
    uint8_t softAPgetStationNum();

    /**
     * Disconnect a specific client from the AP
     */
    bool softAPdisconnect(bool wifioff = false);

    //-------------------------------------------------------------------------
    // WiFi Mode Management
    //-------------------------------------------------------------------------

    /**
     * Set WiFi operating mode
     */
    bool mode(WiFiMode_t m);
    bool mode(WiFiMode_t m, bool persistent);

    /**
     * Get WiFi operating mode
     */
    WiFiMode_t getMode();

    /**
     * Enable STA mode
     */
    bool enableSTA(bool enable);

    /**
     * Enable AP mode
     */
    bool enableAP(bool enable);

    //-------------------------------------------------------------------------
    // Network Scanning
    //-------------------------------------------------------------------------

    /**
     * Start scan WiFi networks available
     * @return Number of discovered networks
     */
    int8_t scanNetworks(bool async = false, bool show_hidden = false, bool passive = false, uint32_t max_ms_per_chan = 300);

    /**
     * Called to get the scan state in Async mode
     * @return scan result or status
     *          -1 if scan not triggered
     *          -2 if scan not finished
     *          >0 number of found WiFi networks
     */
    int8_t scanComplete();

    /**
     * Delete last scan result from RAM
     */
    void scanDelete();

    /**
     * Return the SSID discovered during the network scan
     * @param i specify from which network item want to get the information (0-based index)
     * @return SSID string of the specified item on the networks scanned list
     */
    String SSID(uint8_t networkItem);

    /**
     * Return the encryption type of the networks discovered during the scan
     * @param i specify from which network item want to get the information (0-based index)
     * @return encryption type (wl_enc_type or wifi_auth_mode_t)
     */
    uint8_t encryptionType(uint8_t networkItem);

    /**
     * Return the RSSI of the networks discovered during the scan
     * @param i specify from which network item want to get the information (0-based index)
     * @return RSSI value
     */
    int32_t RSSI(uint8_t networkItem);

    /**
     * Return the BSSID of the networks discovered during the scan
     * @param i specify from which network item want to get the information (0-based index)
     * @return pointer to uint8_t array with length: 6
     */
    uint8_t* BSSID(uint8_t networkItem);

    /**
     * Return the channel of the networks discovered during the scan
     * @param i specify from which network item want to get the information (0-based index)
     * @return channel
     */
    int32_t channel(uint8_t networkItem);

    /**
     * Check if network is hidden (ESP8266/ESP32)
     */
    bool isHidden(uint8_t networkItem);

    //-------------------------------------------------------------------------
    // Power Management
    //-------------------------------------------------------------------------

    /**
     * Set power save mode
     */
    bool setSleep(bool enabled);
    bool setSleepMode(int type);  // ESP8266 compatibility
    bool setSleep(wifi_ps_type_t sleepType);  // ESP32 compatibility

    /**
     * Get sleep mode
     */
    int getSleepMode();  // ESP8266 compatibility
    wifi_ps_type_t getSleep();  // ESP32 compatibility

    /**
     * Set transmit power (ESP32)
     */
    bool setTxPower(int8_t power);
    int8_t getTxPower();

    //-------------------------------------------------------------------------
    // Low Level Control
    //-------------------------------------------------------------------------

    /**
     * Shut down WiFi
     */
    bool disconnect(bool wifioff, bool eraseap);
    bool enableSTA(bool enable, bool persistent);
    bool enableAP(bool enable, bool persistent);

    /**
     * Set WiFi to promiscuous mode
     */
    void setPromiscuous(bool enabled);
    
    /**
     * Get promiscuous mode status
     */
    bool getPromiscuous();

    //-------------------------------------------------------------------------
    // Event Handling (ESP8266/ESP32)
    //-------------------------------------------------------------------------

    /**
     * Register event handler
     */
    void onEvent(WiFiEventHandler handler);
    void onEvent(WiFiEventCb cbEvent, WiFiEvent_t event = WIFI_EVENT_MAX);

    //-------------------------------------------------------------------------
    // WPS (WiFi Protected Setup)
    //-------------------------------------------------------------------------

    /**
     * Start WPS configuration
     */
    bool beginWPSConfig();

    //-------------------------------------------------------------------------
    // SmartConfig
    //-------------------------------------------------------------------------

    /**
     * Start SmartConfig
     */
    bool beginSmartConfig();
    
    /**
     * Check if SmartConfig is done
     */
    bool smartConfigDone();

    /**
     * Stop SmartConfig
     */
    bool stopSmartConfig();

    //-------------------------------------------------------------------------
    // Testing/Mock Functions (EpoxyDuino specific)
    //-------------------------------------------------------------------------

#if defined(EPOXY_DUINO)
    /**
     * Mock functions to simulate WiFi behavior for testing
     */
    void mockSetStatus(wl_status_t status) { _status = status; }
    void mockSetLocalIP(IPAddress ip) { _localIP = ip; }
    void mockSetSSID(const String& ssid) { _ssid = ssid; }
    void mockSetRSSI(int8_t rssi) { (void)rssi; }  // Could store if needed
    void mockReset();
#endif

    //-------------------------------------------------------------------------
    // Deprecated/Legacy Functions
    //-------------------------------------------------------------------------

    // For compatibility, but may do nothing
    void printDiag(Print& dest);
    void waitForConnectResult();
    bool getNetworkInfo(uint8_t i, String &ssid, uint8_t &encType, int32_t &rssi, uint8_t* &bssid, int32_t &channel, bool &isHidden);
};

// Global WiFi instance
extern WiFiClass WiFi;

//-----------------------------------------------------------------------------
// WiFiClient - TCP Client
//-----------------------------------------------------------------------------

class WiFiClient : public Stream {
private:
    bool _connected;

public:
    WiFiClient();
    WiFiClient(int sock);
    virtual ~WiFiClient();

    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    int connect(const String& host, uint16_t port);

    size_t write(uint8_t b);
    size_t write(const uint8_t *buf, size_t size);

    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    void flush();
    void stop();

    uint8_t connected();
    operator bool();

    // Additional ESP8266/ESP32 methods
    IPAddress remoteIP();
    uint16_t remotePort();
    IPAddress localIP();
    uint16_t localPort();

    bool getNoDelay();
    bool setNoDelay(bool nodelay);
};

//-----------------------------------------------------------------------------
// WiFiServer - TCP Server
//-----------------------------------------------------------------------------

class WiFiServer : public Print {
private:
    uint16_t _port;
    bool _listening;

public:
    WiFiServer(uint16_t port);

    void begin();
    void begin(uint16_t port);
    
    WiFiClient available();
    WiFiClient accept();
    
    size_t write(uint8_t b);
    size_t write(const uint8_t *buf, size_t size);

    void stop();
    void close();
    void end();

    operator bool();
};

//-----------------------------------------------------------------------------
// WiFiUDP - UDP Communication (stub)
//-----------------------------------------------------------------------------

class WiFiUDP {
private:
    uint16_t _port;

public:
    WiFiUDP();
    ~WiFiUDP();

    uint8_t begin(uint16_t port);
    void stop();

    int beginPacket(IPAddress ip, uint16_t port);
    int beginPacket(const char *host, uint16_t port);
    int endPacket();

    size_t write(uint8_t b);
    size_t write(const uint8_t *buffer, size_t size);

    int parsePacket();
    int available();
    int read();
    int read(unsigned char* buffer, size_t len);
    int read(char* buffer, size_t len);
    int peek();
    void flush();

    IPAddress remoteIP();
    uint16_t remotePort();
};

//-----------------------------------------------------------------------------
// Additional ESP8266 Compatibility
//-----------------------------------------------------------------------------

// ESP8266-specific: backward compatibility with older ESP8266WiFi
#if defined(EPOXY_CORE_ESP8266) || defined(EPOXY_DUINO)

#define STATION_IF 0
#define SOFTAP_IF 1

// Legacy ESP8266 event system
typedef enum {
    EVENT_STAMODE_CONNECTED = 0,
    EVENT_STAMODE_DISCONNECTED,
    EVENT_STAMODE_AUTHMODE_CHANGE,
    EVENT_STAMODE_GOT_IP,
    EVENT_STAMODE_DHCP_TIMEOUT,
    EVENT_SOFTAPMODE_STACONNECTED,
    EVENT_SOFTAPMODE_STADISCONNECTED,
    EVENT_SOFTAPMODE_PROBEREQRECVED
} System_Event_t;

#endif // EPOXY_CORE_ESP8266

//-----------------------------------------------------------------------------
// Additional ESP32 Compatibility
//-----------------------------------------------------------------------------

#if defined(EPOXY_CORE_ESP32) || defined(EPOXY_DUINO)

// ESP32 WiFiMulti is often used, provide a minimal stub
class WiFiMulti {
public:
    bool addAP(const char* ssid, const char *passphrase = nullptr);
    uint8_t run(uint32_t connectTimeout = 5000);
};

#endif // EPOXY_CORE_ESP32

#endif // EPOXY_MOCK_WIFI_H
