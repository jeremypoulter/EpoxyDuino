/*
 * WiFiMockTest.ino
 *
 * Deterministic unit-style tests for EpoxyMockWiFi.
 * Covers:
 *   - scan list content and metadata
 *   - per-SSID connect outcomes (EPX_OK, EPX_BADPASS, EPX_TIMEOUT,
 *     EPX_NOIP, EPX_FLAPPY, EPX_HIDDEN)
 *   - AP lifecycle and query API
 *   - localIP() host-IP behaviour on successful STA connection
 *
 * Exit code 0 = all passed.  Non-zero = at least one failure.
 */

#include <Arduino.h>
#include <WiFi.h>

// ---------------------------------------------------------------------------
// Minimal assertion helpers
// ---------------------------------------------------------------------------

static int gPassed = 0;
static int gFailed = 0;

static void _check(bool condition, const char* expr, const char* file, int line) {
    if (condition) {
        gPassed++;
    } else {
        gFailed++;
        Serial.print("FAIL ");
        Serial.print(file);
        Serial.print(":");
        Serial.print(line);
        Serial.print("  ");
        Serial.println(expr);
    }
}

#define CHECK(expr) _check((expr), #expr, __FILE__, __LINE__)

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

void testScanList();
void testConnectOutcomes();
void testFlappySSID();
void testHiddenSSID();
void testAPLifecycle();
void testHostIPOnSuccess();
void testStaticIPHonoured();
void testGetNetworkInfo();

// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);

#if defined(EPOXY_DUINO)
    Serial.setLineModeUnix();
#endif

    delay(100);

    Serial.println("\nWiFiMockTest");
    Serial.println("============");

    testScanList();
    testConnectOutcomes();
    testFlappySSID();
    testHiddenSSID();
    testAPLifecycle();
    testHostIPOnSuccess();
    testStaticIPHonoured();
    testGetNetworkInfo();

    // Summary
    Serial.println();
    Serial.print("Tests passed: ");
    Serial.println(gPassed);
    Serial.print("Tests failed: ");
    Serial.println(gFailed);

    if (gFailed == 0) {
        Serial.println("RESULT: PASS");
    } else {
        Serial.println("RESULT: FAIL");
    }

#if defined(EPOXY_DUINO)
    exit(gFailed == 0 ? 0 : 1);
#endif
}

void loop() {}

// ---------------------------------------------------------------------------
// testScanList – scanNetworks() returns known entries with correct metadata
// ---------------------------------------------------------------------------

void testScanList() {
    Serial.println("\n[testScanList]");
    WiFi.mockReset();

    // Default scan (hidden excluded)
    int8_t n = WiFi.scanNetworks();
    // 5 visible + 1 hidden; scanNetworks() without show_hidden excludes hidden
    CHECK(n == 5);

    // Scan including hidden networks
    int8_t nAll = WiFi.scanNetworks(false, true);
    CHECK(nAll == 6);

    // Verify EPX_OK entry (index 0)
    CHECK(WiFi.SSID(0) == EPX_SSID_OK);
    CHECK(WiFi.RSSI(0) < -40);          // -45 dBm
    CHECK(WiFi.encryptionType(0) == WIFI_AUTH_WPA2_PSK);
    CHECK(WiFi.channel(0) == 6);
    CHECK(WiFi.isHidden(0) == false);
    uint8_t* bssid0 = WiFi.BSSID(0);
    CHECK(bssid0 != nullptr);
    CHECK(bssid0[5] == 0x01);

    // Verify EPX_HIDDEN entry (index 5, last)
    CHECK(WiFi.SSID(5) == EPX_SSID_HIDDEN);
    CHECK(WiFi.isHidden(5) == true);

    // Out-of-range access returns safe defaults
    CHECK(WiFi.SSID(99) == "");
    CHECK(WiFi.RSSI(99) == 0);
}

// ---------------------------------------------------------------------------
// testConnectOutcomes – deterministic status per SSID
// ---------------------------------------------------------------------------

void testConnectOutcomes() {
    Serial.println("\n[testConnectOutcomes]");

    // EPX_OK → WL_CONNECTED
    WiFi.mockReset();
    wl_status_t s = WiFi.begin(EPX_SSID_OK, "anypass");
    CHECK(s == WL_CONNECTED);
    CHECK(WiFi.isConnected());
    CHECK(WiFi.SSID() == EPX_SSID_OK);

    // EPX_BADPASS → WL_CONNECT_FAILED
    WiFi.mockReset();
    s = WiFi.begin(EPX_SSID_BADPASS, "wrongpass");
    CHECK(s == WL_CONNECT_FAILED);
    CHECK(!WiFi.isConnected());

    // EPX_TIMEOUT → WL_CONNECTION_LOST
    WiFi.mockReset();
    s = WiFi.begin(EPX_SSID_TIMEOUT, "anypass");
    CHECK(s == WL_CONNECTION_LOST);
    CHECK(!WiFi.isConnected());

    // EPX_NOIP → WL_CONNECTED but localIP == 0.0.0.0
    WiFi.mockReset();
    s = WiFi.begin(EPX_SSID_NOIP);
    CHECK(s == WL_CONNECTED);
    CHECK(WiFi.isConnected());
    CHECK(WiFi.localIP() == IPAddress(0, 0, 0, 0));

    // Unknown SSID → WL_NO_SSID_AVAIL
    WiFi.mockReset();
    s = WiFi.begin("UNKNOWN_NETWORK", "anypass");
    CHECK(s == WL_NO_SSID_AVAIL);
    CHECK(!WiFi.isConnected());
}

// ---------------------------------------------------------------------------
// testFlappySSID – alternates success/failure on each call
// ---------------------------------------------------------------------------

void testFlappySSID() {
    Serial.println("\n[testFlappySSID]");
    WiFi.mockReset();

    // Attempt 1: success (odd)
    wl_status_t s1 = WiFi.begin(EPX_SSID_FLAPPY, "anypass");
    CHECK(s1 == WL_CONNECTED);
    CHECK(WiFi.isConnected());

    // Attempt 2: failure (even)
    wl_status_t s2 = WiFi.begin(EPX_SSID_FLAPPY, "anypass");
    CHECK(s2 == WL_CONNECT_FAILED);
    CHECK(!WiFi.isConnected());

    // Attempt 3: success again (odd)
    wl_status_t s3 = WiFi.begin(EPX_SSID_FLAPPY, "anypass");
    CHECK(s3 == WL_CONNECTED);
    CHECK(WiFi.isConnected());

    // mockReset() resets the counter; next call should be attempt 1 (success)
    WiFi.mockReset();
    wl_status_t s4 = WiFi.begin(EPX_SSID_FLAPPY, "anypass");
    CHECK(s4 == WL_CONNECTED);
}

// ---------------------------------------------------------------------------
// testHiddenSSID – connectable by exact SSID but hidden in default scan
// ---------------------------------------------------------------------------

void testHiddenSSID() {
    Serial.println("\n[testHiddenSSID]");
    WiFi.mockReset();

    // Invisible in default scan
    int8_t n = WiFi.scanNetworks(false, false);
    bool foundHidden = false;
    for (int8_t i = 0; i < n; i++) {
        if (WiFi.SSID(i) == EPX_SSID_HIDDEN) {
            foundHidden = true;
        }
    }
    CHECK(!foundHidden);

    // Visible when show_hidden = true
    int8_t nAll = WiFi.scanNetworks(false, true);
    bool foundHiddenAll = false;
    for (int8_t i = 0; i < nAll; i++) {
        if (WiFi.SSID(i) == EPX_SSID_HIDDEN) {
            foundHiddenAll = true;
        }
    }
    CHECK(foundHiddenAll);

    // Connectable by direct begin()
    WiFi.mockReset();
    wl_status_t s = WiFi.begin(EPX_SSID_HIDDEN, "anypass");
    CHECK(s == WL_CONNECTED);
    CHECK(WiFi.isConnected());
}

// ---------------------------------------------------------------------------
// testAPLifecycle – softAP(), softAPSSID(), softAPIP(), softAPdisconnect()
// ---------------------------------------------------------------------------

void testAPLifecycle() {
    Serial.println("\n[testAPLifecycle]");
    WiFi.mockReset();

    // Start AP
    bool ok = WiFi.softAP("MyAP", "secret123", 11, 0, 4);
    CHECK(ok);
    CHECK(WiFi.getMode() == WIFI_AP);
    CHECK(WiFi.softAPSSID() == "MyAP");
    CHECK(WiFi.softAPIP() == IPAddress(192, 168, 4, 1));
    CHECK(WiFi.softAPgetStationNum() == 0);

    // Custom AP IP via softAPConfig
    WiFi.softAPConfig(
        IPAddress(10, 10, 0, 1),
        IPAddress(10, 10, 0, 1),
        IPAddress(255, 255, 255, 0));
    CHECK(WiFi.softAPIP() == IPAddress(10, 10, 0, 1));

    // AP+STA combined mode
    WiFi.begin(EPX_SSID_OK, "anypass");
    CHECK(WiFi.getMode() == WIFI_AP_STA);

    // Disconnect AP
    WiFi.softAPdisconnect(false);
    CHECK(WiFi.getMode() == WIFI_STA);

    // Hidden AP
    WiFi.mockReset();
    WiFi.softAP("HiddenAP", "pass", 6, 1);
    // isHidden flag stored in AP state; we verify the AP is enabled
    CHECK(WiFi.getMode() == WIFI_AP);
    CHECK(WiFi.softAPSSID() == "HiddenAP");
}

// ---------------------------------------------------------------------------
// testHostIPOnSuccess – EPX_OK localIP is non-zero on this host
// ---------------------------------------------------------------------------

void testHostIPOnSuccess() {
    Serial.println("\n[testHostIPOnSuccess]");
    WiFi.mockReset();

    WiFi.begin(EPX_SSID_OK, "anypass");
    IPAddress ip = WiFi.localIP();

    // Must be a non-zero address (either host IP or loopback fallback)
    CHECK(ip != IPAddress(0, 0, 0, 0));

    Serial.print("  Host IP reported: ");
    Serial.println(ip);
}

// ---------------------------------------------------------------------------
// testStaticIPHonoured – config() before begin() keeps the static IP
// ---------------------------------------------------------------------------

void testStaticIPHonoured() {
    Serial.println("\n[testStaticIPHonoured]");
    WiFi.mockReset();

    IPAddress staticIP(172, 16, 0, 42);
    WiFi.config(staticIP, IPAddress(172, 16, 0, 1), IPAddress(255, 255, 0, 0));
    WiFi.begin(EPX_SSID_OK, "anypass");

    CHECK(WiFi.status() == WL_CONNECTED);
    CHECK(WiFi.localIP() == staticIP);
}

// ---------------------------------------------------------------------------
// testGetNetworkInfo – getNetworkInfo() matches scan accessors
// ---------------------------------------------------------------------------

void testGetNetworkInfo() {
    Serial.println("\n[testGetNetworkInfo]");
    WiFi.mockReset();

    String ssid;
    uint8_t encType;
    int32_t rssi;
    uint8_t* bssid = nullptr;
    int32_t ch;
    bool hidden;

    bool ok = WiFi.getNetworkInfo(0, ssid, encType, rssi, bssid, ch, hidden);
    CHECK(ok);
    CHECK(ssid == EPX_SSID_OK);
    CHECK(encType == WIFI_AUTH_WPA2_PSK);
    CHECK(rssi < 0);
    CHECK(bssid != nullptr);
    CHECK(ch == 6);
    CHECK(hidden == false);

    // Out-of-range returns false
    bool bad = WiFi.getNetworkInfo(99, ssid, encType, rssi, bssid, ch, hidden);
    CHECK(!bad);
}
