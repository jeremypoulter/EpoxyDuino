/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 *
 * Avahi-based ESPmDNS implementation for EpoxyDuino native builds.
 */

#include "ESPmDNS.h"
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/address.h>
#include <cstring>
#include <iostream>
#include <ctime>

// Global MDNS instance
MDNSResponder MDNS;

// Static callback context for Avahi integration
struct AvahiBrowseContext {
  MDNSResponder* mdns;
  AvahiSimplePoll* simple_poll;
};

// Forward declarations for callbacks
static void browse_callback(
    AvahiServiceBrowser* b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AvahiLookupResultFlags flags,
    void* userdata);

static void resolve_callback(
    AvahiServiceResolver* r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char* name,
    const char* type,
    const char* domain,
    const char* host_name,
    const AvahiAddress* address,
    uint16_t port,
    AvahiStringList* txt,
    AvahiLookupResultFlags flags,
    void* userdata);

// Avahi browser callback
static void browse_callback(
    AvahiServiceBrowser* b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AvahiLookupResultFlags flags,
    void* userdata) {
  
  (void)flags;
  
  AvahiBrowseContext* ctx = static_cast<AvahiBrowseContext*>(userdata);
  if (!ctx || !ctx->mdns) return;

  switch (event) {
    case AVAHI_BROWSER_NEW: {
      // Service discovered - resolve it to get full details
      AvahiServiceResolver* resolver = avahi_service_resolver_new(
          avahi_service_browser_get_client(b),
          interface,
          protocol,
          name,
          type,
          domain,
          AVAHI_PROTO_UNSPEC,
          (AvahiLookupFlags)0,
          resolve_callback,
          userdata);
      
      if (!resolver) {
        // Could not create resolver, but don't fail completely
      }
      break;
    }
    case AVAHI_BROWSER_REMOVE:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_FAILURE:
      break;
  }
}

// Avahi resolver callback
static void resolve_callback(
    AvahiServiceResolver* r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char* name,
    const char* type,
    const char* domain,
    const char* host_name,
    const AvahiAddress* address,
    uint16_t port,
    AvahiStringList* txt,
    AvahiLookupResultFlags flags,
    void* userdata) {
  
  (void)interface;
  (void)protocol;
  (void)flags;
  (void)name;
  (void)type;
  (void)domain;
  
  AvahiBrowseContext* ctx = static_cast<AvahiBrowseContext*>(userdata);
  if (!ctx || !ctx->mdns) {
    if (r) avahi_service_resolver_free(r);
    return;
  }

  switch (event) {
    case AVAHI_RESOLVER_FOUND: {
      // Convert Avahi address to IPAddress
      char addr_str[AVAHI_ADDRESS_STR_MAX];
      avahi_address_snprint(addr_str, sizeof(addr_str), address);
      
      // Parse IP address
      uint8_t a, b, c, d;
      if (sscanf(addr_str, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) == 4) {
        IPAddress ip(a, b, c, d);
        String hostname = String(host_name);
        
        // Add to query results
        MDNSService service(hostname, ip, port);
        
        // Process TXT records
        AvahiStringList* tl = txt;
        while (tl) {
          char* key_value = avahi_string_list_to_string(tl);
          if (key_value) {
            char* eq = strchr(key_value, '=');
            if (eq) {
              *eq = '\0';
              String key = String(key_value);
              String value = String(eq + 1);
              service.txt_records[key] = value;
            }
            avahi_free(key_value);
          }
          tl = avahi_string_list_get_next(tl);
        }
        
        ctx->mdns->_queryResults.push_back(service);
      }
      break;
    }
    case AVAHI_RESOLVER_FAILURE:
      break;
  }

  if (r) avahi_service_resolver_free(r);
}

// Avahi client callback
static void client_callback(
    AvahiClient* c,
    AvahiClientState state,
    void* userdata) {
  
  (void)userdata;
  
  switch (state) {
    case AVAHI_CLIENT_S_RUNNING:
    case AVAHI_CLIENT_S_COLLISION:
    case AVAHI_CLIENT_S_REGISTERING:
      break;
    case AVAHI_CLIENT_FAILURE:
      if (c) {
        avahi_client_free(c);
      }
      break;
    case AVAHI_CLIENT_CONNECTING:
      break;
  }
}

//-----------------------------------------------------------------------------
// MDNSResponder Implementation
//-----------------------------------------------------------------------------

MDNSResponder::MDNSResponder() 
  : _hostname("")
  , _instanceName("")
  , _serviceName("")
  , _avahi_client(nullptr)
  , _avahi_browser(nullptr)
  , _avahi_entry_group(nullptr)
  , _avahi_simple_poll(nullptr) {
}

MDNSResponder::~MDNSResponder() {
  end();
}

bool MDNSResponder::begin(const String& hostName) {
  _hostname = hostName;
  if (_instanceName.length() == 0) {
    _instanceName = hostName;
  }
  
  return _initAvahi();
}

void MDNSResponder::end() {
  _unpublishService("_http", "_tcp", 0);  // Cleanup any published services
  _cleanupAvahi();
  _hostname = "";
  _advertisedServices.clear();
  _queryResults.clear();
}

void MDNSResponder::setInstanceName(String name) {
  _instanceName = name;
}

bool MDNSResponder::_initAvahi() {
  if (_avahi_client) {
    return true;  // Already initialized
  }

  int error = 0;

  // Create a simple poll object (store as member)
  _avahi_simple_poll = avahi_simple_poll_new();
  if (!_avahi_simple_poll) {
    std::cerr << "Failed to create Avahi simple poll" << std::endl;
    return false;
  }

  // Create a new Avahi client
  _avahi_client = avahi_client_new(
      avahi_simple_poll_get(_avahi_simple_poll),
      (AvahiClientFlags)0,
      client_callback,
      nullptr,
      &error);

  if (!_avahi_client) {
    std::cerr << "Failed to create Avahi client: " << avahi_strerror(error) << std::endl;
    avahi_simple_poll_free(_avahi_simple_poll);
    _avahi_simple_poll = nullptr;
    return false;
  }

  // Wait briefly for client to be ready
  for (int i = 0; i < 50 && avahi_client_get_state(_avahi_client) != AVAHI_CLIENT_S_RUNNING; i++) {
    avahi_simple_poll_iterate(_avahi_simple_poll, 10);
  }

  return true;
}

void MDNSResponder::_cleanupAvahi() {
  if (_avahi_entry_group) {
    avahi_entry_group_free(_avahi_entry_group);
    _avahi_entry_group = nullptr;
  }

  if (_avahi_browser) {
    avahi_service_browser_free(_avahi_browser);
    _avahi_browser = nullptr;
  }

  if (_avahi_client) {
    avahi_client_free(_avahi_client);
    _avahi_client = nullptr;
  }

  if (_avahi_simple_poll) {
    avahi_simple_poll_free(_avahi_simple_poll);
    _avahi_simple_poll = nullptr;
  }
}

//-----------------------------------------------------------------------------
// Service Advertisement (Responder Mode)
//-----------------------------------------------------------------------------

bool MDNSResponder::addService(char* service, char* proto, uint16_t port) {
  if (!service || !proto || port == 0) return false;

  // Store service to be advertised
  AdvertisedService adv_svc(String(_instanceName), String(service), String(proto), port);
  _advertisedServices.push_back(adv_svc);

  // Automatically publish like ESP32 does
  return publishService(_instanceName.c_str(), service, proto, port);
}

bool MDNSResponder::addServiceTxt(char* name, char* proto, char* key, char* value) {
  if (!name || !proto || !key || !value) return false;

  String serviceName(name);
  String serviceType(proto);
  
  // Find matching service and add TXT record
  for (auto& svc : _advertisedServices) {
    if (svc.type == serviceName && svc.proto == serviceType) {
      svc.txt_records[String(key)] = String(value);
      
      // Republish the service to include the new TXT record
      publishService(svc.name.c_str(), svc.type.c_str(), svc.proto.c_str(), svc.port);
      
      return true;
    }
  }

  return false;
}

bool MDNSResponder::removeService(char* service, char* proto, uint16_t port) {
  if (!service || !proto) return false;

  String serviceType(service);
  String protoType(proto);

  // Find and remove matching service
  for (auto it = _advertisedServices.begin(); it != _advertisedServices.end(); ++it) {
    if (it->type == serviceType && it->proto == protoType && it->port == port) {
      _advertisedServices.erase(it);
      return true;
    }
  }

  return false;
}

bool MDNSResponder::publishService(const char* name, const char* service, 
                                   const char* proto, uint16_t port) {
  if (!name || !service || !proto || port == 0) return false;

  return _publishService(name, service, proto, port);
}

bool MDNSResponder::unpublishService(const char* service, const char* proto, 
                                     uint16_t port) {
  if (!service || !proto) return false;

  _unpublishService(service, proto, port);
  return true;
}

bool MDNSResponder::_publishService(const char* name, const char* service, 
                                    const char* proto, uint16_t port) {
  if (!_avahi_client) {
    if (!_initAvahi()) {
      return false;
    }
  }

  // Check if client is running - retry up to 200ms if not ready
  AvahiClientState state = avahi_client_get_state(_avahi_client);
  int retries = 20;
  while (state != AVAHI_CLIENT_S_RUNNING && retries > 0) {
    if (_avahi_simple_poll) {
      avahi_simple_poll_iterate(_avahi_simple_poll, 10);
    }
    state = avahi_client_get_state(_avahi_client);
    retries--;
  }
  
  if (state != AVAHI_CLIENT_S_RUNNING) {
    std::cerr << "Avahi client not running (state=" << state << "), cannot publish service" << std::endl;
    return false;
  }

  // Reset entry group if it exists (to clear any previous state)
  if (_avahi_entry_group) {
    avahi_entry_group_reset(_avahi_entry_group);
  }

  // Create entry group if needed
  if (!_avahi_entry_group) {
    _avahi_entry_group = avahi_entry_group_new(_avahi_client, nullptr, nullptr);
    if (!_avahi_entry_group) {
      std::cerr << "Failed to create Avahi entry group" << std::endl;
      return false;
    }
  }

  // Build service type string
  String service_type = "_";
  service_type += service;
  service_type += "._";
  service_type += proto;

  // Build TXT records using AvahiStringList
  AvahiStringList* txt_list = nullptr;
  for (auto& svc : _advertisedServices) {
    if (svc.type == String(service) && svc.proto == String(proto) && svc.port == port) {
      for (auto& txt : svc.txt_records) {
        String txt_entry = txt.first + "=" + txt.second;
        txt_list = avahi_string_list_add(txt_list, txt_entry.c_str());
      }
      break;
    }
  }

  // Add the service entry with TXT records
  int ret = avahi_entry_group_add_service_strlst(
      _avahi_entry_group,
      AVAHI_IF_UNSPEC,
      AVAHI_PROTO_UNSPEC,
      (AvahiPublishFlags)0,
      name,
      service_type.c_str(),
      "local",
      nullptr,
      port,
      txt_list);

  // Free the TXT list
  if (txt_list) {
    avahi_string_list_free(txt_list);
  }

  if (ret < 0) {
    std::cerr << "Failed to add service: " << avahi_strerror(ret) << std::endl;
    // Reset on failure
    if (_avahi_entry_group) {
      avahi_entry_group_free(_avahi_entry_group);
      _avahi_entry_group = nullptr;
    }
    return false;
  }

  // Commit the entry group
  ret = avahi_entry_group_commit(_avahi_entry_group);
  if (ret < 0) {
    std::cerr << "Failed to commit entry group: " << avahi_strerror(ret) << std::endl;
    return false;
  }

  return true;
}

void MDNSResponder::_unpublishService(const char* service, const char* proto, 
                                      uint16_t port) {
  (void)service;
  (void)proto;
  (void)port;

  // Reset entry group to unpublish services
  if (_avahi_entry_group) {
    avahi_entry_group_reset(_avahi_entry_group);
  }
}

void MDNSResponder::enableArduino(uint16_t port, bool auth) {
  (void)auth;
  // Publish Arduino OTA update service
  publishService(_hostname.c_str(), "http", "tcp", port);
  addServiceTxt("http", "tcp", "service", "arduino");
}

void MDNSResponder::disableArduino() {
  // Unpublish Arduino OTA update service
  unpublishService("http", "tcp", 3232);
}

//-----------------------------------------------------------------------------
// Service Discovery (Query Mode)
//-----------------------------------------------------------------------------

IPAddress MDNSResponder::queryHost(char* host, uint32_t timeout) {
  (void)timeout;
  
  // Check mock services first
  for (auto& servicePair : _mockServices) {
    for (auto& service : servicePair.second) {
      if (service.hostname == String(host)) {
        return service.ip;
      }
    }
  }
  
  // Check Avahi results
  for (auto& service : _queryResults) {
    if (service.hostname == String(host)) {
      return service.ip;
    }
  }
  
  return IPAddress(0, 0, 0, 0);
}

int MDNSResponder::queryService(char* service, char* proto) {
  _queryResults.clear();
  
  // Add mock services matching the query
  String key = _makeServiceKey(String(service), String(proto));
  auto it = _mockServices.find(key);
  if (it != _mockServices.end()) {
    _queryResults = it->second;
  }
  
  // Perform Avahi browse if available
  if (_avahi_client) {
    _browseService(service, proto, 2000);  // 2 second timeout
  }
  
  return _queryResults.size();
}

void MDNSResponder::_browseService(const char* service, const char* proto, 
                                    uint32_t timeout_ms) {
  if (!_avahi_client) return;

  // Build service type string
  String service_type = "_";
  service_type += service;
  service_type += "._";
  service_type += proto;

  // Create a new Avahi simple poll for this query
  AvahiSimplePoll* simple_poll = avahi_simple_poll_new();
  if (!simple_poll) return;

  AvahiBrowseContext ctx;
  ctx.mdns = this;
  ctx.simple_poll = simple_poll;

  // Create service browser
  AvahiClient* tmp_client = avahi_client_new(
      avahi_simple_poll_get(simple_poll),
      (AvahiClientFlags)0,
      client_callback,
      nullptr,
      nullptr);

  if (!tmp_client) {
    avahi_simple_poll_free(simple_poll);
    return;
  }

  AvahiServiceBrowser* browser = avahi_service_browser_new(
      tmp_client,
      AVAHI_IF_UNSPEC,
      AVAHI_PROTO_UNSPEC,
      service_type.c_str(),
      "local",
      (AvahiLookupFlags)0,
      browse_callback,
      &ctx);

  if (!browser) {
    avahi_client_free(tmp_client);
    avahi_simple_poll_free(simple_poll);
    return;
  }

  // Process events for the specified timeout
  struct timespec start, now;
  clock_gettime(CLOCK_MONOTONIC, &start);

  while (true) {
    clock_gettime(CLOCK_MONOTONIC, &now);
    long elapsed = (now.tv_sec - start.tv_sec) * 1000 + 
                   (now.tv_nsec - start.tv_nsec) / 1000000;
    
    if (elapsed > (long)timeout_ms) break;

    int timeout = timeout_ms - elapsed;
    avahi_simple_poll_iterate(simple_poll, timeout);
  }

  // Cleanup
  if (browser) {
    avahi_service_browser_free(browser);
  }
  if (tmp_client) {
    avahi_client_free(tmp_client);
  }
  if (simple_poll) {
    avahi_simple_poll_free(simple_poll);
  }
}

void MDNSResponder::processAvahiEvents(int timeout_ms) {
  // Process pending Avahi events for the given timeout
  // This is a no-op for now - real implementation would use main event loop
  (void)timeout_ms;
}

//-----------------------------------------------------------------------------
// Result Accessors
//-----------------------------------------------------------------------------

String MDNSResponder::hostname(int idx) {
  MDNSService* service = _getResult(idx);
  return service ? service->hostname : "";
}

IPAddress MDNSResponder::IP(int idx) {
  MDNSService* service = _getResult(idx);
  return service ? service->ip : IPAddress(0, 0, 0, 0);
}

uint16_t MDNSResponder::port(int idx) {
  MDNSService* service = _getResult(idx);
  return service ? service->port : 0;
}

int MDNSResponder::numTxt(int idx) {
  MDNSService* service = _getResult(idx);
  return service ? service->txt_records.size() : 0;
}

bool MDNSResponder::hasTxt(int idx, const char* key) {
  MDNSService* service = _getResult(idx);
  if (!service) return false;
  
  return service->txt_records.find(String(key)) != service->txt_records.end();
}

String MDNSResponder::txt(int idx, const char* key) {
  MDNSService* service = _getResult(idx);
  if (!service) return "";
  
  auto it = service->txt_records.find(String(key));
  return (it != service->txt_records.end()) ? it->second : "";
}

String MDNSResponder::txt(int idx, int txtIdx) {
  MDNSService* service = _getResult(idx);
  if (!service || txtIdx < 0 || txtIdx >= (int)service->txt_records.size()) {
    return "";
  }
  
  auto it = service->txt_records.begin();
  std::advance(it, txtIdx);
  return it->second;
}

String MDNSResponder::txtKey(int idx, int txtIdx) {
  MDNSService* service = _getResult(idx);
  if (!service || txtIdx < 0 || txtIdx >= (int)service->txt_records.size()) {
    return "";
  }
  
  auto it = service->txt_records.begin();
  std::advance(it, txtIdx);
  return it->first;
}

//-----------------------------------------------------------------------------
// Mock Service Registry Management
//-----------------------------------------------------------------------------

void MDNSResponder::addMockService(const String& service, const String& proto,
                                   const String& hostname, const IPAddress& ip,
                                   uint16_t port) {
  String key = _makeServiceKey(service, proto);
  
  MDNSService newService(hostname, ip, port);
  _mockServices[key].push_back(newService);
}

void MDNSResponder::addMockServiceTxt(const String& service, const String& proto,
                                      const String& hostname, const String& key,
                                      const String& value) {
  String serviceKey = _makeServiceKey(service, proto);
  
  auto it = _mockServices.find(serviceKey);
  if (it != _mockServices.end()) {
    // Find the service with matching hostname
    for (auto& svc : it->second) {
      if (svc.hostname == hostname) {
        svc.txt_records[key] = value;
        break;
      }
    }
  }
}

void MDNSResponder::clearMockServices() {
  _mockServices.clear();
  _queryResults.clear();
}

//-----------------------------------------------------------------------------
// Private Helper Methods
//-----------------------------------------------------------------------------

String MDNSResponder::_makeServiceKey(const String& service, const String& proto) {
  // Create key like "_openevse._tcp"
  String key = service;
  if (!key.startsWith("_")) {
    key = "_" + key;
  }
  
  String protoStr = proto;
  if (!protoStr.startsWith("_")) {
    protoStr = "_" + protoStr;
  }
  
  return key + "." + protoStr;
}

MDNSService* MDNSResponder::_getResult(int idx) {
  if (idx < 0 || idx >= (int)_queryResults.size()) {
    return nullptr;
  }
  return &_queryResults[idx];
}
