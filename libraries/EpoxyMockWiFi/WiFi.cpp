/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 * Copyright (c) 2025 Brian T. Park
 */

#include "WiFi.h"

// Host-IP discovery uses POSIX getifaddrs()/freeifaddrs().
#if defined(EPOXY_DUINO)
#  include <ifaddrs.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#endif

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
    , _staticIPConfigured(false)
    , _ssid("")
    , _psk("")
    , _hostname("epoxy-duino")
    , _channel(1)
    , _autoConnect(true)
    , _persistent(true)
    , _apEnabled(false)
    , _apSsid("")
    , _apPsk("")
    , _apChannel(1)
    , _apHidden(false)
    , _apMaxConn(4)
    , _apLocalIP(192, 168, 4, 1)
    , _apGatewayIP(192, 168, 4, 1)
    , _apSubnetMask(255, 255, 255, 0)
    , _scanInProgress(false)
    , _scanHasResult(false)
    , _scanReadyAt(0)
    , _scanResultCount(0)
    , _flappyCounter(0)
    , _eventHandler(nullptr)
    , _eventHandlerInfo(nullptr)
{
    // Initialize mock MAC address
    _macAddress[0] = 0xDE;
    _macAddress[1] = 0xAD;
    _macAddress[2] = 0xBE;
    _macAddress[3] = 0xEF;
    _macAddress[4] = 0xFE;
    _macAddress[5] = 0xED;

    _initDefaultScanResults();
}

void WiFiClass::_emitEvent(WiFiEvent_t event, const arduino_event_info_t& info) {
    if (_eventHandler) {
        _eventHandler(event);
    }

    if (_eventHandlerInfo) {
        _eventHandlerInfo(event, info);
    }

    for (const auto& cb : _eventCallbacks) {
        if (cb.second && (cb.first == ARDUINO_EVENT_MAX || cb.first == event)) {
            cb.second(event);
        }
    }
}

//-----------------------------------------------------------------------------
// Private helpers
//-----------------------------------------------------------------------------

void WiFiClass::_initDefaultScanResults() {
    _scanResults.clear();

    // EPX_OK – good signal, WPA2
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_OK;
        e.rssi = -45;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 6;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x01;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_BADPASS – medium signal, WPA2
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_BADPASS;
        e.rssi = -62;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 11;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x02;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_TIMEOUT – weak signal, WPA2
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_TIMEOUT;
        e.rssi = -78;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 1;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x03;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_NOIP – good signal, open network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_NOIP;
        e.rssi = -55;
        e.encryptionType = WIFI_AUTH_OPEN;
        e.channel = 3;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x04;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_FLAPPY – medium signal, WPA2
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_FLAPPY;
        e.rssi = -70;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 9;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x05;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_HIDDEN – hidden network (omitted from default scan, connectable directly)
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_HIDDEN;
        e.rssi = -60;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 6;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x06;
        e.isHidden = true;
        _scanResults.push_back(e);
    }
    // EPX_OPEN – open auth network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_OPEN;
        e.rssi = -52;
        e.encryptionType = WIFI_AUTH_OPEN;
        e.channel = 4;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x07;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_WEP – WEP auth network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_WEP;
        e.rssi = -67;
        e.encryptionType = WIFI_AUTH_WEP;
        e.channel = 2;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x08;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_WPA – WPA-PSK auth network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_WPA;
        e.rssi = -59;
        e.encryptionType = WIFI_AUTH_WPA_PSK;
        e.channel = 8;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x09;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_WPA2 – WPA2-PSK auth network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_WPA2;
        e.rssi = -50;
        e.encryptionType = WIFI_AUTH_WPA2_PSK;
        e.channel = 6;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x0A;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
    // EPX_WPA3 – WPA3-PSK auth network
    {
        MockScanEntry e;
        e.ssid = EPX_SSID_WPA3;
        e.rssi = -61;
        e.encryptionType = WIFI_AUTH_WPA3_PSK;
        e.channel = 10;
        e.bssid[0] = 0xAA; e.bssid[1] = 0xBB; e.bssid[2] = 0xCC;
        e.bssid[3] = 0x00; e.bssid[4] = 0x00; e.bssid[5] = 0x0B;
        e.isHidden = false;
        _scanResults.push_back(e);
    }
}

IPAddress WiFiClass::_getHostIPv4() {
#if defined(EPOXY_DUINO)
    struct ifaddrs* iflist = nullptr;
    if (getifaddrs(&iflist) != 0 || iflist == nullptr) {
        return IPAddress(127, 0, 0, 1);
    }

    IPAddress result(127, 0, 0, 1);
    for (struct ifaddrs* ifa = iflist; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        const struct sockaddr_in* sa =
            reinterpret_cast<const struct sockaddr_in*>(ifa->ifa_addr);
        uint32_t addr = ntohl(sa->sin_addr.s_addr);
        // Skip loopback (127.x.x.x)
        if ((addr >> 24) == 127) continue;
        result = IPAddress(
            (addr >> 24) & 0xFF,
            (addr >> 16) & 0xFF,
            (addr >>  8) & 0xFF,
            (addr      ) & 0xFF);
        break;
    }
    freeifaddrs(iflist);
    return result;
#else
    return IPAddress(127, 0, 0, 1);
#endif
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
    if (_apEnabled) {
        _mode = WIFI_AP_STA;
    } else {
        _mode = WIFI_STA;
    }

    // Determine the IP to use on a successful connection.
    // A static IP set via config() takes priority over host discovery.
    IPAddress successIP = _staticIPConfigured ? _localIP : _getHostIPv4();

    // Deterministic outcomes based on well-known mock SSIDs.
    if (_ssid == EPX_SSID_OK) {
        _status = WL_CONNECTED;
        _localIP = successIP;
        if (!_staticIPConfigured) {
            _gatewayIP = IPAddress(0, 0, 0, 0);
            _dnsIP = IPAddress(0, 0, 0, 0);
        }
    } else if (_ssid == EPX_SSID_BADPASS) {
        _status = WL_CONNECT_FAILED;
        if (!_staticIPConfigured) {
            _localIP = IPAddress(0, 0, 0, 0);
        }
    } else if (_ssid == EPX_SSID_TIMEOUT) {
        _status = WL_CONNECTION_LOST;
        if (!_staticIPConfigured) {
            _localIP = IPAddress(0, 0, 0, 0);
        }
    } else if (_ssid == EPX_SSID_NOIP) {
        // L2 connected but no usable DHCP address
        _status = WL_CONNECTED;
        _localIP = IPAddress(0, 0, 0, 0);
        _gatewayIP = IPAddress(0, 0, 0, 0);
        _dnsIP = IPAddress(0, 0, 0, 0);
    } else if (_ssid == EPX_SSID_FLAPPY) {
        _flappyCounter++;
        if (_flappyCounter % 2 == 1) {
            // Odd attempt: success
            _status = WL_CONNECTED;
            _localIP = successIP;
        } else {
            // Even attempt: failure
            _status = WL_CONNECT_FAILED;
            if (!_staticIPConfigured) {
                _localIP = IPAddress(0, 0, 0, 0);
            }
        }
    } else if (_ssid == EPX_SSID_HIDDEN) {
        // Hidden network – connectable by exact SSID
        _status = WL_CONNECTED;
        _localIP = successIP;
        if (!_staticIPConfigured) {
            _gatewayIP = IPAddress(0, 0, 0, 0);
            _dnsIP = IPAddress(0, 0, 0, 0);
        }
    } else if (_ssid == EPX_SSID_OPEN ||
               _ssid == EPX_SSID_WEP ||
               _ssid == EPX_SSID_WPA ||
               _ssid == EPX_SSID_WPA2 ||
               _ssid == EPX_SSID_WPA3) {
        // Auth-mode specific deterministic success SSIDs.
        _status = WL_CONNECTED;
        _localIP = successIP;
        if (!_staticIPConfigured) {
            _gatewayIP = IPAddress(0, 0, 0, 0);
            _dnsIP = IPAddress(0, 0, 0, 0);
        }
    } else {
        // Unknown SSID: no network available
        _status = WL_NO_SSID_AVAIL;
        if (!_staticIPConfigured) {
            _localIP = IPAddress(0, 0, 0, 0);
        }
    }

    if (_status == WL_CONNECTED) {
        arduino_event_info_t connectedInfo = {};
        size_t ssidLen = _ssid.length();
        if (ssidLen > 32) {
            ssidLen = 32;
        }
        memcpy(connectedInfo.wifi_sta_connected.ssid, _ssid.c_str(), ssidLen);

        uint8_t* bssid = BSSID();
        if (bssid) {
            memcpy(connectedInfo.wifi_sta_connected.bssid, bssid, 6);
        }
        connectedInfo.wifi_sta_connected.channel = static_cast<uint8_t>(_channel);
        _emitEvent(ARDUINO_EVENT_WIFI_STA_CONNECTED, connectedInfo);

        // Mirror ESP32 flow: once connected and IP is available, emit GOT_IP.
        if (_localIP != IPAddress(0, 0, 0, 0)) {
            arduino_event_info_t gotIpInfo = {};
            gotIpInfo.got_ip.ip_info.ip.addr = static_cast<uint32_t>(_localIP);
            gotIpInfo.got_ip.ip_info.netmask.addr = static_cast<uint32_t>(_subnetMask);
            gotIpInfo.got_ip.ip_info.gw.addr = static_cast<uint32_t>(_gatewayIP);
            _emitEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP, gotIpInfo);
        }
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
    _staticIPConfigured = true;
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
    _apSsid = ssid;
    _apPsk = passphrase ? passphrase : "";
    _apChannel = channel;
    _apHidden = (ssid_hidden != 0);
    _apMaxConn = max_connection;
    _apEnabled = true;
    _mode = (_mode == WIFI_STA) ? WIFI_AP_STA : WIFI_AP;
    return true;
}

String WiFiClass::softAPSSID() const {
    return _apSsid;
}

bool WiFiClass::softAPsetHostname(const char* hostname) {
    return setHostname(hostname);
}

bool WiFiClass::softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    _apLocalIP = local_ip;
    _apGatewayIP = gateway;
    _apSubnetMask = subnet;
    return true;
}

IPAddress WiFiClass::softAPIP() {
    return _apLocalIP;
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
    _apEnabled = false;
    if (wifioff) {
        _mode = (_mode == WIFI_AP_STA) ? WIFI_STA : WIFI_OFF;
    } else {
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
    (void)passive;
    // Treat 0 as "unspecified" and complete on the next scanComplete() call.
    const uint32_t scanDelayMs = (max_ms_per_chan == 0) ? 1 : max_ms_per_chan;

    if (_scanInProgress) {
        return WIFI_SCAN_RUNNING;
    }

    int8_t count = 0;
    for (const auto& e : _scanResults) {
        if (!e.isHidden || show_hidden) {
            count++;
        }
    }

    _scanResultCount = count;
    _scanHasResult = false;

    if (async) {
        (void)scanDelayMs;
        _scanInProgress = false;
        _scanHasResult = true;
        _scanReadyAt = 0;

        arduino_event_info_t info = {};
        info.wifi_scan_done.number = static_cast<uint32_t>(_scanResultCount);
        _emitEvent(ARDUINO_EVENT_WIFI_SCAN_DONE, info);

        return WIFI_SCAN_RUNNING;
    }

    _scanInProgress = false;
    _scanHasResult = true;
    _scanReadyAt = 0;
    return _scanResultCount;
}

int8_t WiFiClass::scanComplete() {
    if (_scanInProgress) {
        if ((long)(millis() - _scanReadyAt) >= 0) {
            _scanInProgress = false;
            _scanHasResult = true;
            return _scanResultCount;
        }
        return WIFI_SCAN_RUNNING;
    }

    if (_scanHasResult) {
        return _scanResultCount;
    }

    return WIFI_SCAN_FAILED;
}

void WiFiClass::scanDelete() {
    // Restore default scan results instead of clearing entirely so that
    // subsequent scans still work as expected after a delete.
    _scanInProgress = false;
    _scanHasResult = false;
    _scanReadyAt = 0;
    _scanResultCount = 0;
    _initDefaultScanResults();
}

String WiFiClass::SSID(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        return _scanResults[networkItem].ssid;
    }
    return String("");
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        return _scanResults[networkItem].encryptionType;
    }
    return ENC_TYPE_NONE;
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        return _scanResults[networkItem].rssi;
    }
    return 0;
}

uint8_t* WiFiClass::BSSID(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        // Return pointer into the entry – valid for the lifetime of the object
        return const_cast<uint8_t*>(_scanResults[networkItem].bssid);
    }
    static uint8_t zeroBssid[6] = {0};
    return zeroBssid;
}

int32_t WiFiClass::channel(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        return _scanResults[networkItem].channel;
    }
    return 0;
}

bool WiFiClass::isHidden(uint8_t networkItem) {
    if (networkItem < _scanResults.size()) {
        return _scanResults[networkItem].isHidden;
    }
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
    _eventHandler = handler;
}

void WiFiClass::onEvent(WiFiEventCb cbEvent, WiFiEvent_t event) {
    _eventCallbacks.push_back(std::make_pair(event, cbEvent));
}

void WiFiClass::onEvent(WiFiEventHandlerInfo handler) {
    _eventHandlerInfo = handler;
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
    _staticIPConfigured = false;
    _ssid = "";
    _psk = "";
    _hostname = "epoxy-duino";
    _channel = 1;
    _autoConnect = true;
    _persistent = true;
    _apEnabled = false;
    _apSsid = "";
    _apPsk = "";
    _apChannel = 1;
    _apHidden = false;
    _apMaxConn = 4;
    _apLocalIP = IPAddress(192, 168, 4, 1);
    _apGatewayIP = IPAddress(192, 168, 4, 1);
    _apSubnetMask = IPAddress(255, 255, 255, 0);
    _flappyCounter = 0;
    _initDefaultScanResults();
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
    dest.print("STA SSID: ");
    dest.println(_ssid);
    dest.print("STA IP: ");
    dest.println(_localIP);
    dest.print("AP Enabled: ");
    dest.println(_apEnabled);
    dest.print("AP SSID: ");
    dest.println(_apSsid);
    dest.print("AP IP: ");
    dest.println(_apLocalIP);
}

void WiFiClass::waitForConnectResult() {
    // Mock: do nothing, already "connected"
}

bool WiFiClass::getNetworkInfo(uint8_t i, String &ssid, uint8_t &encType, int32_t &rssi, uint8_t* &bssid, int32_t &channel, bool &isHidden) {
    if (i >= _scanResults.size()) {
        return false;
    }
    const MockScanEntry& e = _scanResults[i];
    ssid = e.ssid;
    encType = e.encryptionType;
    rssi = e.rssi;
    bssid = const_cast<uint8_t*>(e.bssid);
    channel = e.channel;
    isHidden = e.isHidden;
    return true;
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
