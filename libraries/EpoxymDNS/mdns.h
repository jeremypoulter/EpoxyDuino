/*
 * MIT License
 * Copyright (c) 2025 Jeremy Poulter
 *
 * Async mDNS API definitions compatible with ESP32 mdns.h
 * Provides async query capabilities for mDNS service discovery.
 */

#ifndef MDNS_H_
#define MDNS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <cstdint>

/**
 * @brief ESP error type
 */
typedef int esp_err_t;

#define ESP_OK                      0
#define ESP_ERR_NO_MEM              -1
#define ESP_ERR_INVALID_ARG         -2
#define ESP_ERR_INVALID_STATE       -3

/**
 * @brief Asynchronous query handle (opaque)
 */
struct mdns_search_once_s {
  // Opaque handle - implementation details hidden
  void* _reserved;
};

typedef struct mdns_search_once_s mdns_search_once_t;

/**
 * @brief mDNS enum to specify the ip_protocol type
 */
typedef enum {
    MDNS_IP_PROTOCOL_V4 = 0,
    MDNS_IP_PROTOCOL_V6 = 1,
    MDNS_IP_PROTOCOL_MAX = 2
} mdns_ip_protocol_t;

/**
 * @brief mDNS query type constants
 */
#define MDNS_TYPE_A                 0x0001
#define MDNS_TYPE_PTR               0x000C
#define MDNS_TYPE_TXT               0x0010
#define MDNS_TYPE_AAAA              0x001C
#define MDNS_TYPE_SRV               0x0021
#define MDNS_TYPE_OPT               0x0029
#define MDNS_TYPE_NSEC              0x002F
#define MDNS_TYPE_ANY               0x00FF

/**
 * @brief mDNS basic text item structure
 */
typedef struct {
    const char * key;
    const char * value;
} mdns_txt_item_t;

/**
 * @brief IPv4 address type
 */
struct esp_ip4_addr {
    uint32_t addr;  /*!< IPv4 address */
};

typedef struct esp_ip4_addr esp_ip4_addr_t;

/**
 * @brief IPv6 address type (simplified)
 */
struct esp_ip6_addr {
    uint32_t addr[4];  /*!< IPv6 address */
};

typedef struct esp_ip6_addr esp_ip6_addr_t;

/**
 * @brief ESP IP address type
 */
typedef struct _ip_addr {
    union {
        esp_ip6_addr_t ip6;  /*!< IPv6 address type */
        esp_ip4_addr_t ip4;  /*!< IPv4 address type */
    } u_addr;                /*!< IP address union */
    uint8_t type;            /*!< ipaddress type */
} esp_ip_addr_t;

/**
 * @brief mDNS query linked list IP item
 */
typedef struct mdns_ip_addr_s {
    esp_ip_addr_t addr;         /* IP address */
    struct mdns_ip_addr_s * next;
} mdns_ip_addr_t;

/**
 * @brief mDNS interface types
 */
typedef enum {
    MDNS_IF_STA = 0,
    MDNS_IF_AP = 1,
    MDNS_IF_ETH = 2,
    MDNS_IF_MAX = 3
} mdns_if_t;

/**
 * @brief mDNS query result structure
 */
typedef struct mdns_result_s {
    struct mdns_result_s * next;
    
    mdns_if_t tcpip_if;
    uint32_t ttl;
    
    mdns_ip_protocol_t ip_protocol;
    char * instance_name;
    char * service_type;
    char * proto;
    char * hostname;
    uint16_t port;
    mdns_txt_item_t * txt;
    uint8_t *txt_value_len;
    size_t txt_count;
    mdns_ip_addr_t * addr;
} mdns_result_t;

/**
 * @brief mDNS query callback for async search
 * Called when search is complete or results are available
 */
typedef void (*mdns_query_notify_t)(mdns_search_once_t *search);

/**
 * @brief Initialize mDNS (stub for compatibility)
 * 
 * @return ESP_OK on success
 */
esp_err_t mdns_init(void);

/**
 * @brief Stop and free mDNS server (stub for compatibility)
 */
void mdns_free(void);

/**
 * @brief Set the hostname for mDNS server
 * 
 * @param hostname  Hostname to set
 * @return ESP_OK on success
 */
esp_err_t mdns_hostname_set(const char * hostname);

/**
 * @brief Initiate an asynchronous mDNS query
 * 
 * Starts an asynchronous query for mDNS services. Results can be retrieved
 * using mdns_query_async_get_results() and must be freed with mdns_query_async_delete().
 *
 * @param name           Service instance or host name (NULL for PTR queries)
 * @param service_type   Service type (e.g., "openevse" or "http")
 * @param proto          Protocol type (e.g., "tcp" or "udp")
 * @param type           Query type (MDNS_TYPE_*)
 * @param timeout_ms     Query timeout in milliseconds
 * @param max_results    Maximum number of results to collect
 * @param notifier       Optional callback function when query completes
 *
 * @return Opaque search handle on success, NULL on failure
 */
mdns_search_once_t * mdns_query_async_new(
    const char * name,
    const char * service_type,
    const char * proto,
    uint16_t type,
    uint32_t timeout_ms,
    size_t max_results,
    mdns_query_notify_t notifier);

/**
 * @brief Get results from an asynchronous query
 * 
 * Polls the status of an async query and retrieves results if available.
 * Results are returned as a linked list of mdns_result_t structures.
 * Must be freed with mdns_query_async_delete() after processing.
 *
 * @param search     Search handle from mdns_query_async_new()
 * @param timeout    Timeout in milliseconds for polling (0 for non-blocking)
 * @param results    Output parameter for results linked list
 *
 * @return True if query is complete, false if still pending
 */
bool mdns_query_async_get_results(
    mdns_search_once_t * search,
    uint32_t timeout,
    mdns_result_t ** results);

/**
 * @brief Free an asynchronous query and its results
 * 
 * Must be called after mdns_query_async_get_results() returns true
 * to clean up resources. Do not call before query completes.
 *
 * @param search  Search handle to free
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mdns_query_async_delete(mdns_search_once_t * search);

/**
 * @brief Free mDNS result linked list
 * 
 * Frees a result structure returned by mdns_query_async_get_results().
 * Only call this after you've extracted needed data from results.
 *
 * @param results  Results linked list to free
 */
void mdns_query_results_free(mdns_result_t * results);

/**
 * @brief Add a service for mDNS advertising
 * 
 * @param instance   Service instance name
 * @param service    Service type (without protocol)
 * @param proto      Protocol (_tcp or _udp)
 * @param port       Service port
 * @param txt        Optional TXT records array
 * @param txt_count  Number of TXT records
 *
 * @return ESP_OK on success
 */
esp_err_t mdns_service_add(
    const char *instance,
    const char *service,
    const char *proto,
    uint16_t port,
    mdns_txt_item_t txt[],
    size_t txt_count);

/**
 * @brief Remove a service from mDNS advertising
 * 
 * @param service    Service type (without protocol)
 * @param proto      Protocol (_tcp or _udp)
 *
 * @return ESP_OK on success
 */
esp_err_t mdns_service_remove(const char *service, const char *proto);

/**
 * @brief Add a TXT record to an advertised service
 * 
 * @param service    Service type (without protocol)
 * @param proto      Protocol (_tcp or _udp)
 * @param key        TXT record key
 * @param value      TXT record value
 *
 * @return ESP_OK on success
 */
esp_err_t mdns_service_txt_set(
    const char *service,
    const char *proto,
    const char *key,
    const char *value);

#ifdef __cplusplus
}
#endif

#endif /* MDNS_H_ */
