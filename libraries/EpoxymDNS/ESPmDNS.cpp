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
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>

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
      
      // Preserve a discovered service even when it only resolves to a hostname
      // (or IPv6) so higher layers can still track peers by instance/host.
      MDNSService service;
      service.instance_name = name ? String(name) : String("");
      service.hostname = host_name ? String(host_name) : String("");
      service.port = port;

      // Parse IPv4 address when available.
      uint8_t a, b, c, d;
      if (sscanf(addr_str, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) == 4) {
        service.ip = IPAddress(a, b, c, d);
      }

      // Process TXT records
      AvahiStringList* tl = txt;
      while (tl) {
        char* key = nullptr;
        char* value = nullptr;
        size_t value_size = 0;
        if (avahi_string_list_get_pair(tl, &key, &value, &value_size) == 0 && key) {
          String keyStr = String(key);
          String valueStr = value ? String(value) : String("");
          service.txt_records[keyStr] = valueStr;
        }

        if (key) {
          avahi_free(key);
        }
        if (value) {
          avahi_free(value);
        }
        tl = avahi_string_list_get_next(tl);
      }

      ctx->mdns->_queryResults.push_back(service);
      break;
    }
    case AVAHI_RESOLVER_FAILURE:
      std::cerr << "Avahi resolver failure for '" << (name ? name : "?") << "': "
                << avahi_strerror(avahi_client_errno(
                     avahi_service_resolver_get_client(r))) << std::endl;
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
  return _publishService(_instanceName.c_str(), service, proto, port);
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
      _publishService(svc.name.c_str(), svc.type.c_str(), svc.proto.c_str(), svc.port);
      
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

  // Use Avahi's daemon-managed host target for native services. Instance names
  // still carry per-device identity (e.g. openevse-ev0), while this avoids SRV
  // targets that may not have stable A/AAAA records on multi-interface hosts.
  const char* avahi_host = nullptr;

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
      avahi_host,
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
  addService("http", "tcp", port);
  addServiceTxt("http", "tcp", "service", "arduino");
}

void MDNSResponder::disableArduino() {
  // Unpublish Arduino OTA update service
  removeService("http", "tcp", 3232);
}

void MDNSResponder::enableWorkstation(esp_interface_t interface) {
  // Workstation mode not supported on native builds
  // This is a no-op for compatibility with ESP32 ESPmDNS
  (void)interface;
}

void MDNSResponder::disableWorkstation() {
  // Workstation mode not supported on native builds
  // This is a no-op for compatibility with ESP32 ESPmDNS
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

  // Build service type string (service and proto should already have underscores)
  String service_type = service;
  service_type += ".";
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

IPv6Address MDNSResponder::IPv6(int idx) {
  // IPv6 not supported in this Avahi-based implementation
  // Return empty IPv6 address for compatibility
  (void)idx;
  return IPv6Address();
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
//-----------------------------------------------------------------------------
// Async Query Implementation
//-----------------------------------------------------------------------------

mdns_search_once_t* MDNSResponder::beginAsyncQuery(const char* service, const char* proto, uint32_t timeout_ms) {
  if (!service || !proto) {
    return nullptr;
  }
  
  // Create a new search handle using unique pointer value
  void* handle = (void*)new uintptr_t(reinterpret_cast<uintptr_t>(this) + _activeAsyncQueries.size());
  if (!handle) {
    return nullptr;
  }
  
  // Initialize query state
  _activeAsyncQueries[(mdns_search_once_t*)handle] = std::vector<MDNSService>();
  _queryStartTimes[(mdns_search_once_t*)handle] = millis();
  _queryTimeouts[(mdns_search_once_t*)handle] = timeout_ms;

  // Clear previous browse results so this query consumes only fresh resolver callbacks.
  _queryResults.clear();
  
  // Start the browse operation immediately
  _browseService(service, proto, timeout_ms);
  
  // Copy results to async query storage
  if (!_queryResults.empty()) {
    _activeAsyncQueries[(mdns_search_once_t*)handle] = _queryResults;
  }
  
  return (mdns_search_once_t*)handle;
}

bool MDNSResponder::getAsyncQueryResults(mdns_search_once_t* search, mdns_result_t** results, uint32_t timeout_ms) {
  if (!search || !results) {
    return false;
  }
  
  // Check if query exists
  auto it = _activeAsyncQueries.find(search);
  if (it == _activeAsyncQueries.end()) {
    return false;
  }
  
  // Check if query has timed out
  uint32_t elapsed = millis() - _queryStartTimes[search];
  uint32_t queryTimeout = _queryTimeouts[search];
  bool isComplete = elapsed >= queryTimeout;
  
  // If query not complete and no timeout specified, wait
  if (!isComplete && timeout_ms > 0) {
    delay(timeout_ms);
    elapsed = millis() - _queryStartTimes[search];
    isComplete = elapsed >= queryTimeout;
  }
  
  if (isComplete) {
    // Convert stored MDNSService results to mdns_result_t linked list
    std::vector<MDNSService>& services = _queryResults;
    
    if (services.empty()) {
      *results = nullptr;
      return true;
    }
    
    // Build result linked list
    mdns_result_t* first = nullptr;
    mdns_result_t* current = nullptr;
    
    for (const auto& svc : services) {
      mdns_result_t* result = (mdns_result_t*)malloc(sizeof(mdns_result_t));
      if (!result) break;
      
      memset(result, 0, sizeof(mdns_result_t));
      
      // Copy hostname
      result->hostname = (char*)malloc(svc.hostname.length() + 1);
      if (result->hostname) {
        strcpy(result->hostname, svc.hostname.c_str());
      }

      // Copy instance name when available.
      if (svc.instance_name.length() > 0) {
        result->instance_name = (char*)malloc(svc.instance_name.length() + 1);
        if (result->instance_name) {
          strcpy(result->instance_name, svc.instance_name.c_str());
        }
      }
      
      // Copy IP address
      mdns_ip_addr_t* addr = (mdns_ip_addr_t*)malloc(sizeof(mdns_ip_addr_t));
      if (addr) {
        addr->addr.u_addr.ip4.addr = svc.ip;
        addr->addr.type = 0;  // IPv4
        addr->next = nullptr;
        result->addr = addr;
      }
      
      result->port = svc.port;
      result->ttl = 4500;
      result->tcpip_if = MDNS_IF_STA;
      result->ip_protocol = MDNS_IP_PROTOCOL_V4;
      
      // Copy TXT records
      if (!svc.txt_records.empty()) {
        result->txt_count = svc.txt_records.size();
        result->txt = (mdns_txt_item_t*)malloc(result->txt_count * sizeof(mdns_txt_item_t));
        result->txt_value_len = (uint8_t*)malloc(result->txt_count);
        
        int idx = 0;
        for (const auto& txt_pair : svc.txt_records) {
          char* key_buf = (char*)malloc(txt_pair.first.length() + 1);
          if (key_buf) {
            strcpy(key_buf, txt_pair.first.c_str());
            result->txt[idx].key = const_cast<const char*>(key_buf);
          }
          char* val_buf = (char*)malloc(txt_pair.second.length() + 1);
          if (val_buf) {
            strcpy(val_buf, txt_pair.second.c_str());
            result->txt[idx].value = const_cast<const char*>(val_buf);
          }
          result->txt_value_len[idx] = txt_pair.second.length();
          idx++;
        }
      }
      
      // Link to chain
      if (!first) {
        first = result;
        current = result;
      } else {
        current->next = result;
        current = result;
      }
    }
    
    *results = first;
  }
  
  return isComplete;
}

void MDNSResponder::deleteAsyncQuery(mdns_search_once_t* search) {
  if (!search) {
    return;
  }
  
  // Remove from tracking maps
  _activeAsyncQueries.erase(search);
  _queryStartTimes.erase(search);
  _queryTimeouts.erase(search);
  
  // Free the opaque handle
  delete (uintptr_t*)search;
}

//-----------------------------------------------------------------------------
// C API Wrappers for Async mDNS Functions
//-----------------------------------------------------------------------------

extern "C" {

esp_err_t mdns_init(void) {
  return ESP_OK;
}

void mdns_free(void) {
}

esp_err_t mdns_hostname_set(const char * hostname) {
  if (!hostname) {
    return ESP_ERR_INVALID_ARG;
  }
  return MDNS.begin(hostname) ? ESP_OK : ESP_ERR_INVALID_ARG;
}

mdns_search_once_t * mdns_query_async_new(
    const char * name,
    const char * service_type,
    const char * proto,
    uint16_t type,
    uint32_t timeout_ms,
    size_t max_results,
    mdns_query_notify_t notifier) {
  
  (void)name;        // Unused for now - only PTR queries supported
  (void)type;        // Query type (MDNS_TYPE_*) - for future use
  (void)max_results; // Unused for now
  (void)notifier;    // Callback support can be added later
  
  if (!service_type || !proto) {
    return nullptr;
  }
  
  // Build full service type if needed
  String fullServiceType = service_type;
  if (!fullServiceType.startsWith("_")) {
    fullServiceType = "_" + fullServiceType;
  }
  
  String fullProto = proto;
  if (!fullProto.startsWith("_")) {
    fullProto = "_" + fullProto;
  }
  
  return MDNS.beginAsyncQuery(fullServiceType.c_str(), fullProto.c_str(), timeout_ms);
}

bool mdns_query_async_get_results(
    mdns_search_once_t * search,
    uint32_t timeout,
    mdns_result_t ** results) {
  
  if (!search || !results) {
    return false;
  }
  
  return MDNS.getAsyncQueryResults(search, results, timeout);
}

esp_err_t mdns_query_async_delete(mdns_search_once_t * search) {
  if (!search) {
    return ESP_ERR_INVALID_ARG;
  }
  
  MDNS.deleteAsyncQuery(search);
  return ESP_OK;
}

void mdns_query_results_free(mdns_result_t * results) {
  if (!results) {
    return;
  }
  
  mdns_result_t* current = results;
  while (current) {
    mdns_result_t* next = current->next;
    
    // Free TXT records
    if (current->txt) {
      for (size_t i = 0; i < current->txt_count; i++) {
        if (current->txt[i].key) {
          free(const_cast<char*>(current->txt[i].key));
        }
        if (current->txt[i].value) {
          free(const_cast<char*>(current->txt[i].value));
        }
      }
      free(current->txt);
    }
    if (current->txt_value_len) {
      free(current->txt_value_len);
    }
    
    // Free IP addresses
    mdns_ip_addr_t* addr = current->addr;
    while (addr) {
      mdns_ip_addr_t* next_addr = addr->next;
      free(addr);
      addr = next_addr;
    }
    
    // Free strings
    if (current->hostname) {
      free(current->hostname);
    }
    if (current->instance_name) {
      free(current->instance_name);
    }
    if (current->service_type) {
      free(current->service_type);
    }
    if (current->proto) {
      free(current->proto);
    }
    
    free(current);
    current = next;
  }
}

esp_err_t mdns_service_add(
    const char *instance,
    const char *service,
    const char *proto,
    uint16_t port,
    mdns_txt_item_t txt[],
    size_t txt_count) {
  
  (void)instance;  // Instance name not used in current implementation
  
  if (!service || !proto) {
    return ESP_ERR_INVALID_ARG;
  }
  
  if (!MDNS.addService(const_cast<char*>(service), const_cast<char*>(proto), port)) {
    return ESP_ERR_INVALID_STATE;
  }
  
  // Add TXT records
  for (size_t i = 0; i < txt_count; i++) {
    if (txt[i].key && txt[i].value) {
      MDNS.addServiceTxt(const_cast<char*>(service), const_cast<char*>(proto), 
                        const_cast<char*>(txt[i].key), const_cast<char*>(txt[i].value));
    }
  }
  
  return ESP_OK;
}

esp_err_t mdns_service_remove(const char *service, const char *proto) {
  if (!service || !proto) {
    return ESP_ERR_INVALID_ARG;
  }
  
  if (!MDNS.removeService(const_cast<char*>(service), const_cast<char*>(proto), 0)) {
    return ESP_ERR_INVALID_STATE;
  }
  
  return ESP_OK;
}

esp_err_t mdns_service_txt_set(
    const char *service,
    const char *proto,
    const char *key,
    const char *value) {
  
  if (!service || !proto || !key || !value) {
    return ESP_ERR_INVALID_ARG;
  }
  
  MDNS.addServiceTxt(const_cast<char*>(service), const_cast<char*>(proto), 
                    const_cast<char*>(key), const_cast<char*>(value));
  return ESP_OK;
}

}  // extern "C"