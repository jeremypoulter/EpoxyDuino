/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 *
 * Avahi-based ESPmDNS implementation for EpoxyDuino native builds.
 * Provides real mDNS service discovery using the Avahi library.
 */

#ifndef EPOXY_MDNS_H
#define EPOXY_MDNS_H

#include <Arduino.h>
#include <IPAddress.h>
#include <vector>
#include <map>
#include <cstring>
#include "mdns.h"

// IPv6 address placeholder for compatibility
class IPv6Address {
public:
  IPv6Address() {}
  IPv6Address(const IPv6Address& other) { (void)other; }
  bool isSet() const { return false; }
  String toString() const { return String("::"); }
};

// ESP interface type placeholder for compatibility
enum esp_interface_t {
  ESP_IF_WIFI_STA = 0,
  ESP_IF_WIFI_AP = 1
};

// Forward declarations for Avahi structures
struct AvahiClient;
struct AvahiServiceBrowser;
struct AvahiServiceResolver;
struct AvahiEntryGroup;
struct AvahiSimplePoll;

/**
 * @brief mDNS service record
 * 
 * Represents a discovered service with hostname, IP, port, and TXT records.
 */
class MDNSService {
public:
  String hostname;
  IPAddress ip;
  uint16_t port;
  std::map<String, String> txt_records;

  MDNSService() : port(0) {}
  
  MDNSService(const String& host, const IPAddress& addr, uint16_t p)
    : hostname(host), ip(addr), port(p) {}
};

/**
 * @brief Advertised service record
 * 
 * Represents a service being advertised by this responder.
 */
class AdvertisedService {
public:
  String name;
  String type;
  String proto;
  uint16_t port;
  std::map<String, String> txt_records;

  AdvertisedService() : port(0) {}
  
  AdvertisedService(const String& n, const String& t, const String& p, uint16_t port_num)
    : name(n), type(t), proto(p), port(port_num) {}
};

/**
 * @brief MDNSResponder class compatible with ESP32 ESPmDNS
 * 
 * Provides mDNS service discovery using Avahi library for native builds.
 * Maintains API compatibility with ESP32's ESPmDNS while using real system mDNS.
 */
class MDNSResponder {
public:
  MDNSResponder();
  ~MDNSResponder();

  // Basic mDNS responder functions
  bool begin(const String& hostName);
  bool begin(const char* hostName) {
    return begin(String(hostName));
  }
  void end();

  void setInstanceName(String name);
  void setInstanceName(const char* name) {
    setInstanceName(String(name));
  }
  void setInstanceName(char* name) {
    setInstanceName(String(name));
  }

  // Service advertisement (responder mode)
  bool addService(char* service, char* proto, uint16_t port);
  bool addService(const char* service, const char* proto, uint16_t port) {
    return addService((char*)service, (char*)proto, port);
  }
  bool addService(String service, String proto, uint16_t port) {
    return addService(service.c_str(), proto.c_str(), port);
  }

  bool addServiceTxt(char* name, char* proto, char* key, char* value);
  void addServiceTxt(const char* name, const char* proto, const char* key, const char* value) {
    addServiceTxt((char*)name, (char*)proto, (char*)key, (char*)value);
  }
  void addServiceTxt(String name, String proto, String key, String value) {
    addServiceTxt(name.c_str(), proto.c_str(), key.c_str(), value.c_str());
  }

  bool removeService(char* service, char* proto, uint16_t port);
  bool removeService(const char* service, const char* proto, uint16_t port) {
    return removeService((char*)service, (char*)proto, port);
  }
  bool removeService(String service, String proto, uint16_t port) {
    return removeService(service.c_str(), proto.c_str(), port);
  }

  void enableArduino(uint16_t port = 3232, bool auth = false);
  void disableArduino();

  void enableWorkstation(esp_interface_t interface = ESP_IF_WIFI_STA);
  void disableWorkstation();

  // Service discovery (query mode)
  IPAddress queryHost(char* host, uint32_t timeout = 2000);
  IPAddress queryHost(const char* host, uint32_t timeout = 2000) {
    return queryHost((char*)host, timeout);
  }
  IPAddress queryHost(String host, uint32_t timeout = 2000) {
    return queryHost(host.c_str(), timeout);
  }

  int queryService(char* service, char* proto);
  int queryService(const char* service, const char* proto) {
    return queryService((char*)service, (char*)proto);
  }
  int queryService(String service, String proto) {
    return queryService(service.c_str(), proto.c_str());
  }

  // Result accessors (after queryService)
  String hostname(int idx);
  IPAddress IP(int idx);
  IPv6Address IPv6(int idx);
  uint16_t port(int idx);
  int numTxt(int idx);
  bool hasTxt(int idx, const char* key);
  String txt(int idx, const char* key);
  String txt(int idx, int txtIdx);
  String txtKey(int idx, int txtIdx);
  
  // For internal use by callbacks
  std::vector<MDNSService> _queryResults;
  
  // Async query support
  mdns_search_once_t* beginAsyncQuery(const char* service, const char* proto, uint32_t timeout_ms);
  bool getAsyncQueryResults(mdns_search_once_t* search, mdns_result_t** results, uint32_t timeout_ms);
  void deleteAsyncQuery(mdns_search_once_t* search);
  
private:
  String _hostname;
  String _instanceName;
  String _serviceName;
  
  // Mock service registry: key = "service.proto", value = list of services
  std::map<String, std::vector<MDNSService>> _mockServices;
  
  // Advertised services
  std::vector<AdvertisedService> _advertisedServices;
  
  // Avahi client and browser
  AvahiClient* _avahi_client;
  AvahiServiceBrowser* _avahi_browser;
  AvahiEntryGroup* _avahi_entry_group;
  AvahiSimplePoll* _avahi_simple_poll;
  String _current_service_type;
  
  // Async query state tracking
  std::map<mdns_search_once_t*, std::vector<MDNSService>> _activeAsyncQueries;
  std::map<mdns_search_once_t*, uint32_t> _queryStartTimes;
  std::map<mdns_search_once_t*, uint32_t> _queryTimeouts;
  
  String _makeServiceKey(const String& service, const String& proto);
  MDNSService* _getResult(int idx);
  
  // Avahi helper methods
  bool _initAvahi();
  void _cleanupAvahi();
  void _browseService(const char* service, const char* proto, uint32_t timeout_ms);
  bool _publishService(const char* name, const char* service, const char* proto, uint16_t port);
  void _unpublishService(const char* service, const char* proto, uint16_t port);
  bool publishService(const char* name, const char* service, const char* proto, uint16_t port);
  bool unpublishService(const char* service, const char* proto, uint16_t port);
  
  // Mock service management
  void addMockService(const String& service, const String& proto, 
                      const String& hostname, const IPAddress& ip, 
                      uint16_t port);
  void addMockServiceTxt(const String& service, const String& proto,
                         const String& hostname, const String& key, 
                         const String& value);
  void clearMockServices();
};

extern MDNSResponder MDNS;
#endif // EPOXY_MDNS_H