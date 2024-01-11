#include <Wifi.h>
#include <configs.h>

void scan_networks() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  int n = WiFi.scanNetworks();
  while (n < 1) {
    delay(500);
    n = WiFi.scanNetworks();
  }

  Serial.printf("Found %d networks\n", n);

  // Print details for each network
  for (int i = 0; i < n; ++i) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(" dBm, ");
    Serial.print(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open"
                                                          : "Encrypted");
    Serial.print(")");
    Serial.print("Channel: ");
    Serial.println(WiFi.channel(i));
    delay(10);
  }
}

String stringifyWiFiStatus(wl_status_t status) {
  switch (status) {
  case WL_IDLE_STATUS:
    return "Idle status";
  case WL_NO_SSID_AVAIL:
    return "Network SSID unavailable";
  case WL_SCAN_COMPLETED:
    return "Scan complete";
  case WL_CONNECTED:
    return "Connected";
  case WL_CONNECT_FAILED:
    return "Connection failed";
  case WL_CONNECTION_LOST:
    return "Connection lost";
  case WL_DISCONNECTED:
    return "Disconnected";
  default:
    return "Unknown status";
  }
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Wtf");

  while (WL_CONNECTED != WiFi.status()) {
    Serial.printf("Wifi Status: %s\n",
                  stringifyWiFiStatus(WiFi.status()).c_str());

    // loading_animation.render(0, 0, &display, true);
  }

  Serial.println("... connected!");
}
