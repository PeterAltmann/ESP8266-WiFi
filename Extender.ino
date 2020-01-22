/* WiFi NAT Router part from https://github.com/martin-ger/lwip_nat_arduino
 * See also https://github.com/martin-ger/esp_wifi_repeater
 *
 * IP/MAC address retrival part from https://www.esp8266.com/viewtopic.php?f=32&t=5669&start=4
 *
 */

#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0

#ifndef STASSID
#define STASSID "RISE-GUEST"
#define STAPSK  "password"
#endif

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <dhcpserver.h>

#define NAPT 1000
#define NAPT_PORT 10

#if HAVE_NETDUMP

#include <NetDump.h>

extern "C" {
  #include<user_interface.h>
}

void dump(int netif_idx, const char* data, size_t len, int out, int success) {
  (void)success;
  Serial.print(out ? F("out ") : F(" in "));
  Serial.printf("%d ", netif_idx);

  // optional filter example: if (netDump_is_ARP(data))
  {
    netDump(Serial, data, len);
    //netDumpHex(Serial, data, len);
  }
}
#endif

/*
IPAddress local_IP(10, 0, 0, 10);
IPAddress gateway(10, 0, 0, 255);
IPAddress subnet(255, 255, 255, 0);
*/

void setup() {
  pinMode(D4, OUTPUT); // prepares the blue led for status reporting
  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

#if HAVE_NETDUMP
  phy_capture = dump;
#endif

  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID);//, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nSTA: %s (dns: %s / %s)\n",
                WiFi.localIP().toString().c_str(),
                WiFi.dnsIP(0).toString().c_str(),
                WiFi.dnsIP(1).toString().c_str());

  // give DNS servers to AP side
  dhcps_set_dns(0, WiFi.dnsIP(0));
  dhcps_set_dns(1, WiFi.dnsIP(1));

  //WiFi.softAPConfig(local_IP, gateway, subnet); 
  WiFi.softAP(STASSID "extender");//, STAPSK);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID);
    }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) {
    Serial.printf("NAPT initialization failed\n");
  }
}

#else

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT not supported in this configuration\n");
}

#endif

void loop() {
  digitalWrite(D4, HIGH); // turns off blue led (only on LoLin boards)

  // lists the devices connected
  client_status(); 
  delay(3500);

  //blink the blue led when awaiting connections
  if (WiFi.softAPgetStationNum() == 0) {
    digitalWrite(D4, LOW);
    delay(100);
    digitalWrite(D4, HIGH);
  }

  //blink the blue led for a longer period when connected to other device
  if (WiFi.softAPgetStationNum() != 0) {
    digitalWrite(D4, LOW);
    delay(1500);
    digitalWrite(D4, HIGH);
  }
}

void client_status() {
 
  struct station_info *stat_info;
  
  struct ipv4_addr *IPaddress;
  IPAddress address;
  int i=1;
  
  stat_info = wifi_softap_get_station_info();
  
  Serial.print(" Total Connected Clients are = ");
  Serial.println(WiFi.softAPgetStationNum());
  
    while (stat_info != NULL) {
    
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;
      
      Serial.print("Client ");
      Serial.print(i);
      Serial.print(" IP adress is: ");
      Serial.print((address));
      Serial.print(" with MAC adress is = ");
      
      Serial.print(stat_info->bssid[0],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[1],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[2],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[3],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[4],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[5],HEX);Serial.print(" ");
      
      stat_info = STAILQ_NEXT(stat_info, next);
      i++;
      Serial.println();
    }
    
  delay(500);
}
