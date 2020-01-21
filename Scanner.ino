#include "ESP8266WiFi.h"

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA); // Set device to station mode.
  WiFi.disconnect(); // Disconnect from any AP.
  delay(100);

  Serial.println("Setup done");
  Serial.println("Scan interval: 10s");
}

void loop() {
  
  int n = WiFi.scanNetworks(); // Return number of found networks.
  
  Serial.println("# RSSI ENC SSID");
  for (int i = 0; i < n; ++i) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.RSSI(i)); // Return RSSI value if network i.
    Serial.print((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " No  " : " Yes "); // Is i encrypted?
    Serial.print("'");
    Serial.print(WiFi.SSID(i)); // Return SSID of network i.
    Serial.println("'");
    delay(10);
  }
  
  Serial.println("");
  delay(10000);
}
