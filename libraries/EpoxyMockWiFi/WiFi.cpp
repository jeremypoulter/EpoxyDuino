/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 * Copyright (c) 2025 Brian T. Park
 */

#include "WiFi.h"

//-----------------------------------------------------------------------------
// WiFiClass Implementation
//-----------------------------------------------------------------------------

WiFiClass::WiFiClass()
    : _status(WL_DISCONNECTED)
    , _mode(WIFI_OFF)
    , _localIP(0, 0, 0, 0)
    , _subnetMask(255, 255, 255, 0)
    , _gatewayIP(0, 0, 0, 0)
    , _dnsIP(0, 0, 0, 0)
    , _ssid("")
    , _psk("")
    , _hostname("epoxy-duino")
    , _channel(1)
    , _autoConnect(true)
    , _persistent(true)
{
    // Initialize mock MAC address
    _macAddress[0] = 0xDE;
    _macAddress[1] = 0xAD;
    _macAddress[2] = 0xBE;
    _macAddress[3] = 0xEF;
    _macAddress[4] = 0xFE;
    _macAddress[5] = 0xED;
}

//-----------------------------------------------------------------------------
// Station Mode - Connection Management
//-----------------------------------------------------------------------------

wl_status_t WiFiClass::begin(const char* ssid) {
    return begin(ssid, nullptr);
}

wl_status_t WiFiClass::begin(const char* ssid, const char *passphrase) {
    _ssid = ssid;
    if (passphrase) {
        _psk = passphrase;
    }
    _mode = WIFI_STA;
    _status = WL_CONNECTED;  // Mock: immediately "connected"
    
    // Mock: assign a fake local IP if not configured
    if (_localIP == IPAddress(0, 0, 0, 0)) {
        _localIP = IPAddress(192, 168, 1, 100);
        _gatewayIP = IPAddress(192, 168, 1, 1);
        _dnsIP = IPAddress(192, 168, 1, 1);
    }
    
    return _status;
}

wl_status_t WiFiClass::begin(const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect) {
    (void)bssid;
    (void)connect;
    _channel = channel;
    return begin(ssid, passphrase);
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    _localIP = local_ip;
    _gatewayIP = gateway;
    _subnetMask = subnet;
    return true;
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1) {
    config(local_ip, gateway, subnet);
    _dnsIP = dns1;
    return true;
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
    (void)dns2;  // Mock doesn't support secondary DNS
    return config(local_ip, gateway, subnet, dns1);
}

bool WiFiClass::disconnect(bool wifioff) {
    _status = WL_DISCONNECTED;
    if (wifioff) {
        _mode = WIFI_OFF;
    }
    return true;
}

bool WiFiClass::disconnect(bool wifioff, bool eraseap) {
    (void)eraseap;
    return disconnect(wifioff);
}

bool WiFiClass::isConnected() {
    return _status == WL_CONNECTED;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
    _autoConnect = autoReconnect;
    return true;
}

bool WiFiClass::getAutoReconnect() {
    return _autoConnect;
}

bool WiFiClass::persistent(bool persistent) {
    _persistent = persistent;
    return true;
}

bool WiFiClass::isPersistent() {
    return _persistent;
}

//-----------------------------------------------------------------------------
// Station Mode - Information
//-----------------------------------------------------------------------------

uint8_t* WiFiClass::macAddress(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = _macAddress[i];
    }
    return mac;
}

String WiFiClass::macAddress() {
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
            _macAddress[0], _macAddress[1], _macAddress[2],
            _macAddress[3], _macAddress[4], _macAddress[5]);
    return String(buf);
}

IPAddress WiFiClass::localIP() {
    return _localIP;
}

IPAddress WiFiClass::subnetMask() {
    return _subnetMask;
}

IPAddress WiFiClass::gatewayIP() {
    return _gatewayIP;
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
    (void)dns_no;  // Mock only supports one DNS
    return _dnsIP;
}

String WiFiClass::SSID() const {
    return _ssid;
}

int32_t WiFiClass::RSSI() {
    // Return a mock RSSI value
    return -50;
}

uint8_t* WiFiClass::BSSID() {
    static uint8_t bssid[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    return bssid;
}

String WiFiClass::BSSIDstr() {
    uint8_t* bssid = BSSID();
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
            bssid[0], bssid[1], bssid[2],
            bssid[3], bssid[4], bssid[5]);
    return String(buf);
}

String WiFiClass::BSSIDstr(int /*i*/) {
    return BSSIDstr();
}

int32_t WiFiClass::channel() {
    return _channel;
}

wl_status_t WiFiClass::status() {
    return _status;
}

//-----------------------------------------------------------------------------
// Station Mode - Hostname
//-----------------------------------------------------------------------------

bool WiFiClass::hostname(const String& aHostname) {
    _hostname = aHostname;
    return true;
}

bool WiFiClass::hostname(const char* aHostname) {
    _hostname = aHostname;
    return true;
}

bool WiFiClass::setHostname(const char* hostname) {
    _hostname = hostname;
    return true;
}

String WiFiClass::hostname() {
    return _hostname;
}

const char* WiFiClass::getHostname() {
    return _hostname.c_str();
}

//-----------------------------------------------------------------------------
// Access Point Mode
//-----------------------------------------------------------------------------

bool WiFiClass::softAP(const char* ssid, const char* passphrase, int channel, int ssid_hidden, int max_connection) {
    (void)passphrase;
    (void)ssid_hidden;
    (void)max_connection;
    
    _ssid = ssid;
    _channel = channel;
    _mode = (_mode == WIFI_STA) ? WIFI_AP_STA : WIFI_AP;
    
    return true;
}

bool WiFiClass::softAPsetHostname(const char* hostname) {
    return setHostname(hostname);
}

bool WiFiClass::softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    // In AP mode, these would configure the AP's network
    _localIP = local_ip;
    _gatewayIP = gateway;
    _subnetMask = subnet;
    return true;
}

IPAddress WiFiClass::softAPIP() {
    // Return a typical AP IP address
    return IPAddress(192, 168, 4, 1);
}

uint8_t* WiFiClass::softAPmacAddress(uint8_t* mac) {
    // AP MAC is typically different from STA MAC
    for (int i = 0; i < 6; i++) {
        mac[i] = _macAddress[i];
    }
    mac[5] += 1;  // Simple differentiation
    return mac;
}

String WiFiClass::softAPmacAddress() {
    uint8_t mac[6];
    softAPmacAddress(mac);
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

uint8_t WiFiClass::softAPgetStationNum() {
    // Mock: no clients connected
    return 0;
}

bool WiFiClass::softAPdisconnect(bool wifioff) {
    if (wifioff) {
        _mode = (_mode == WIFI_AP_STA) ? WIFI_STA : WIFI_OFF;
    }
    return true;
}

//-----------------------------------------------------------------------------
// WiFi Mode Management
//-----------------------------------------------------------------------------

bool WiFiClass::mode(WiFiMode_t m) {
    _mode = m;
    if (m == WIFI_OFF) {
        _status = WL_DISCONNECTED;
    }
    return true;
}

bool WiFiClass::mode(WiFiMode_t m, bool persistent) {
    _persistent = persistent;
    return mode(m);
}

WiFiMode_t WiFiClass::getMode() {
    return _mode;
}

bool WiFiClass::enableSTA(bool enable) {
    if (enable) {
        _mode = (_mode == WIFI_AP) ? WIFI_AP_STA : WIFI_STA;
    } else {
        _mode = (_mode == WIFI_AP_STA) ? WIFI_AP : WIFI_OFF;
    }
    return true;
}

bool WiFiClass::enableSTA(bool enable, bool persistent) {
    _persistent = persistent;
    return enableSTA(enable);
}

bool WiFiClass::enableAP(bool enable) {
    if (enable) {
        _mode = (_mode == WIFI_STA) ? WIFI_AP_STA : WIFI_AP;
    } else {
        _mode = (_mode == WIFI_AP_STA) ? WIFI_STA : WIFI_OFF;
    }
    return true;
}

bool WiFiClass::enableAP(bool enable, bool persistent) {
    _persistent = persistent;
    return enableAP(enable);
}

//-----------------------------------------------------------------------------
// Network Scanning
//-----------------------------------------------------------------------------

int8_t WiFiClass::scanNetworks(bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan) {
    (void)async;
    (void)show_hidden;
    (void)passive;
    (void)max_ms_per_chan;
    
    // Mock: return 0 networks found
    return 0;
}

int8_t WiFiClass::scanComplete() {
    // Mock: scan always complete with 0 results
    return 0;
}

void WiFiClass::scanDelete() {
    // Mock: nothing to delete
}

String WiFiClass::SSID(uint8_t networkItem) {
    (void)networkItem;
    return String("");
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
    (void)networkItem;
    return ENC_TYPE_NONE;
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
    (void)networkItem;
    return 0;
}

uint8_t* WiFiClass::BSSID(uint8_t networkItem) {
    (void)networkItem;
    static uint8_t bssid[6] = {0};
    return bssid;
}

int32_t WiFiClass::channel(uint8_t networkItem) {
    (void)networkItem;
    return 0;
}

bool WiFiClass::isHidden(uint8_t networkItem) {
    (void)networkItem;
    return false;
}

//-----------------------------------------------------------------------------
// Power Management
//-----------------------------------------------------------------------------

bool WiFiClass::setSleep(bool enabled) {
    (void)enabled;
    return true;
}

bool WiFiClass::setSleepMode(int type) {
    (void)type;
    return true;
}

bool WiFiClass::setSleep(wifi_ps_type_t sleepType) {
    (void)sleepType;
    return true;
}

int WiFiClass::getSleepMode() {
    return WIFI_PS_NONE;
}

wifi_ps_type_t WiFiClass::getSleep() {
    return WIFI_PS_NONE;
}

bool WiFiClass::setTxPower(int8_t power) {
    (void)power;
    return true;
}

int8_t WiFiClass::getTxPower() {
    return 20;  // Mock: maximum power
}

//-----------------------------------------------------------------------------
// Low Level Control
//-----------------------------------------------------------------------------

void WiFiClass::setPromiscuous(bool enabled) {
    (void)enabled;
}

bool WiFiClass::getPromiscuous() {
    return false;
}

//-----------------------------------------------------------------------------
// Event Handling
//-----------------------------------------------------------------------------

void WiFiClass::onEvent(WiFiEventHandler handler) {
    (void)handler;
}

void WiFiClass::onEvent(WiFiEventCb cbEvent, WiFiEvent_t event) {
    (void)cbEvent;
    (void)event;
}

void WiFiClass::onEvent(WiFiEventHandlerInfo handler) {
    (void)handler;
}

//-----------------------------------------------------------------------------
// WPS
//-----------------------------------------------------------------------------

bool WiFiClass::beginWPSConfig() {
    return false;  // Mock: WPS not supported
}

//-----------------------------------------------------------------------------
// SmartConfig
//-----------------------------------------------------------------------------

bool WiFiClass::beginSmartConfig() {
    return false;  // Mock: SmartConfig not supported
}

bool WiFiClass::smartConfigDone() {
    return false;
}

bool WiFiClass::stopSmartConfig() {
    return true;
}

//-----------------------------------------------------------------------------
// Testing/Mock Functions
//-----------------------------------------------------------------------------

#if defined(EPOXY_DUINO)
void WiFiClass::mockReset() {
    _status = WL_DISCONNECTED;
    _mode = WIFI_OFF;
    _localIP = IPAddress(0, 0, 0, 0);
    _subnetMask = IPAddress(255, 255, 255, 0);
    _gatewayIP = IPAddress(0, 0, 0, 0);
    _dnsIP = IPAddress(0, 0, 0, 0);
    _ssid = "";
    _psk = "";
    _hostname = "epoxy-duino";
    _channel = 1;
    _autoConnect = true;
    _persistent = true;
}
#endif

//-----------------------------------------------------------------------------
// Deprecated/Legacy Functions
//-----------------------------------------------------------------------------

void WiFiClass::printDiag(Print& dest) {
    dest.println("WiFi Mock Diagnostics");
    dest.print("Mode: ");
    dest.println(_mode);
    dest.print("Status: ");
    dest.println(_status);
    dest.print("SSID: ");
    dest.println(_ssid);
    dest.print("IP: ");
    dest.println(_localIP);
}

void WiFiClass::waitForConnectResult() {
    // Mock: do nothing, already "connected"
}

bool WiFiClass::getNetworkInfo(uint8_t i, String &ssid, uint8_t &encType, int32_t &rssi, uint8_t* &bssid, int32_t &channel, bool &isHidden) {
    (void)i;
    (void)ssid;
    (void)encType;
    (void)rssi;
    (void)bssid;
    (void)channel;
    (void)isHidden;
    return false;
}

// Global instance
WiFiClass WiFi;

//-----------------------------------------------------------------------------
// WiFiClient Implementation
//-----------------------------------------------------------------------------

WiFiClient::WiFiClient()
    : _connected(false)
{
}

WiFiClient::WiFiClient(int sock)
    : _connected(false)
{
    (void)sock;
}

WiFiClient::~WiFiClient() {
}

int WiFiClient::connect(IPAddress ip, uint16_t port) {
    (void)ip;
    (void)port;
    _connected = false;  // Mock: always fails to connect
    return 0;
}

int WiFiClient::connect(const char *host, uint16_t port) {
    (void)host;
    (void)port;
    _connected = false;
    return 0;
}

int WiFiClient::connect(const String& host, uint16_t port) {
    return connect(host.c_str(), port);
}

size_t WiFiClient::write(uint8_t b) {
    (void)b;
    return _connected ? 1 : 0;
}

size_t WiFiClient::write(const uint8_t *buf, size_t size) {
    (void)buf;
    return _connected ? size : 0;
}

int WiFiClient::available() {
    return 0;
}

int WiFiClient::read() {
    return -1;
}

int WiFiClient::read(uint8_t *buf, size_t size) {
    (void)buf;
    (void)size;
    return 0;
}

int WiFiClient::peek() {
    return -1;
}

void WiFiClient::flush() {
}

void WiFiClient::stop() {
    _connected = false;
}

uint8_t WiFiClient::connected() {
    return _connected ? 1 : 0;
}

WiFiClient::operator bool() {
    return _connected;
}

IPAddress WiFiClient::remoteIP() {
    return IPAddress(0, 0, 0, 0);
}

uint16_t WiFiClient::remotePort() {
    return 0;
}

IPAddress WiFiClient::localIP() {
    return WiFi.localIP();
}

uint16_t WiFiClient::localPort() {
    return 0;
}

bool WiFiClient::getNoDelay() {
    return true;
}

bool WiFiClient::setNoDelay(bool nodelay) {
    (void)nodelay;
    return true;
}

//-----------------------------------------------------------------------------
// WiFiServer Implementation
//-----------------------------------------------------------------------------

WiFiServer::WiFiServer(uint16_t port)
    : _port(port)
    , _listening(false)
{
}

void WiFiServer::begin() {
    _listening = true;
}

void WiFiServer::begin(uint16_t port) {
    _port = port;
    begin();
}

WiFiClient WiFiServer::available() {
    // Mock: never returns a client
    return WiFiClient();
}

WiFiClient WiFiServer::accept() {
    return available();
}

size_t WiFiServer::write(uint8_t b) {
    (void)b;
    return _listening ? 1 : 0;
}

size_t WiFiServer::write(const uint8_t *buf, size_t size) {
    (void)buf;
    return _listening ? size : 0;
}

void WiFiServer::stop() {
    _listening = false;
}

void WiFiServer::close() {
    stop();
}

void WiFiServer::end() {
    stop();
}

WiFiServer::operator bool() {
    return _listening;
}

//-----------------------------------------------------------------------------
// WiFiUDP Implementation
//-----------------------------------------------------------------------------

WiFiUDP::WiFiUDP()
    : _port(0)
{
}

WiFiUDP::~WiFiUDP() {
}

uint8_t WiFiUDP::begin(uint16_t port) {
    _port = port;
    return 1;
}

void WiFiUDP::stop() {
    _port = 0;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port) {
    (void)ip;
    (void)port;
    return 1;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port) {
    (void)host;
    (void)port;
    return 1;
}

int WiFiUDP::endPacket() {
    return 1;
}

size_t WiFiUDP::write(uint8_t b) {
    (void)b;
    return 1;
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size) {
    (void)buffer;
    return size;
}

int WiFiUDP::parsePacket() {
    return 0;
}

int WiFiUDP::available() {
    return 0;
}

int WiFiUDP::read() {
    return -1;
}

int WiFiUDP::read(unsigned char* buffer, size_t len) {
    (void)buffer;
    (void)len;
    return 0;
}

int WiFiUDP::read(char* buffer, size_t len) {
    (void)buffer;
    (void)len;
    return 0;
}

int WiFiUDP::peek() {
    return -1;
}

void WiFiUDP::flush() {
}

IPAddress WiFiUDP::remoteIP() {
    return IPAddress(0, 0, 0, 0);
}

uint16_t WiFiUDP::remotePort() {
    return 0;
}

//-----------------------------------------------------------------------------
// ESP32 WiFiMulti Implementation
//-----------------------------------------------------------------------------

#if defined(EPOXY_CORE_ESP32) || defined(EPOXY_DUINO)

bool WiFiMulti::addAP(const char* ssid, const char *passphrase) {
    (void)ssid;
    (void)passphrase;
    return true;
}

uint8_t WiFiMulti::run(uint32_t connectTimeout) {
    (void)connectTimeout;
    return WL_CONNECTED;
}

#endif // EPOXY_CORE_ESP32
