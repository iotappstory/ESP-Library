/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/CaptivePortalAdvanced/
   Forked from Tzapu https://github.com/tzapu/WiFiManager
   Built by Ken Taylor https://github.com/kentaylor
   Modified by IOTAppStory.com
   Licensed under MIT license
 **************************************************************/

#include "WiFiManagerMod.h"
#include <algorithm>

WiFiManagerParameter::WiFiManagerParameter() {
  _id = NULL;
  _placeholder = NULL;
  _length = 0;
  _value = NULL;
  _labelPlacement = WFM_LABEL_BEFORE;

  _customHTML = NULL;
}

WiFiManagerParameter::WiFiManagerParameter(const char *custom) {
  _id = NULL;
  _placeholder = NULL;
  _length = 0;
  _value = NULL;
  _labelPlacement = WFM_LABEL_BEFORE;

  _customHTML = custom;
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length) {
  init(id, placeholder, defaultValue, length, "", WFM_LABEL_BEFORE);
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
  init(id, placeholder, defaultValue, length, custom, WFM_LABEL_BEFORE);
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement) {
  init(id, placeholder, defaultValue, length, custom, labelPlacement);
}

void WiFiManagerParameter::init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement) {
  _id = id;
  _placeholder = placeholder;
  _length = length;
  _labelPlacement = labelPlacement;
  _value = new char[length + 1];
  for (int i = 0; i < length; i++) {
    _value[i] = 0;
  }
  if (defaultValue != NULL) {
    strncpy(_value, defaultValue, length);
  }
	
  _customHTML = custom;
}

const char* WiFiManagerParameter::getValue() {
  return _value;
}
const char* WiFiManagerParameter::getID() {
  return _id;
}
const char* WiFiManagerParameter::getPlaceholder() {
  return _placeholder;
}
int WiFiManagerParameter::getValueLength() {
  return _length;
}
int WiFiManagerParameter::getLabelPlacement() {
  return _labelPlacement;
}
const char* WiFiManagerParameter::getCustomHTML() {
  return _customHTML;
}

WiFiManager::WiFiManager() {
	//Do a network scan before setting up an access point so as not to close WiFiNetwork while scanning.
	numberOfNetworks = scanWifiNetworks(networkIndicesptr);
}
WiFiManager::~WiFiManager() {
    free(networkIndices); //indices array no longer required so free memory
}


void WiFiManager::addParameter(WiFiManagerParameter *p) {
  _params[_paramsCount] = p;
  _paramsCount++;
  DEBUG_WM("Adding parameter");
  DEBUG_WM(p->getID());
}

void WiFiManager::setupConfigPortal() {
  stopConfigPortal = false; //Signal not to close config portal
  /*This library assumes autoconnect is set to 1. It usually is
  but just in case check the setting and turn on autoconnect if it is off.
  Some useful discussion at https://github.com/esp8266/Arduino/issues/1615*/
  if (WiFi.getAutoConnect()==0)WiFi.setAutoConnect(1);
  dnsServer.reset(new DNSServer());
  server.reset(new ESP8266WebServer(80));

  DEBUG_WM(F(""));
  _configPortalStart = millis();

  DEBUG_WM(F("Configuring access point... "));
  DEBUG_WM(_apName);
  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      // fail passphrase to short or long!
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

  //optional soft ip config
  if (_ap_static_ip) {
    DEBUG_WM(F("Custom AP IP/GW/Subnet"));
    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
  }

  if (_apPassword != NULL) {
    WiFi.softAP(_apName, _apPassword);//password option
  } else {
    WiFi.softAP(_apName);
  }

  delay(500); // Without delay I've seen the IP address blank
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server->on("/", std::bind(&WiFiManager::handleRoot, this));
  server->on("/wifi", std::bind(&WiFiManager::handleWifi, this));
  server->on("/wifisave", std::bind(&WiFiManager::handleWifiSave, this));
  
  server->on("/app", std::bind(&WiFiManager::handleApp, this));
  server->on("/appsave", std::bind(&WiFiManager::handleAppSave, this));
  
  server->on("/close", std::bind(&WiFiManager::handleServerClose, this));
  server->on("/i", std::bind(&WiFiManager::handleInfo, this));
  server->on("/r", std::bind(&WiFiManager::handleReset, this));
  server->on("/state", std::bind(&WiFiManager::handleState, this));
  server->on("/scan", std::bind(&WiFiManager::handleScan, this));
  
  #if IASCNF == 1
	  server->on("/ias", std::bind(&WiFiManager::handleIAScfg, this));
	  server->on("/addpage", std::bind(&WiFiManager::handleAddPage, this));
	  server->on("/add", std::bind(&WiFiManager::handleDevSave, this));
	  
	  server->on("/editPro", std::bind(&WiFiManager::handleEditProPage, this));
	  server->on("/newPro", std::bind(&WiFiManager::handleNewProPage, this));
	  server->on("/addToPro", std::bind(&WiFiManager::handleAddToProPage, this));
	  server->on("/savePro", std::bind(&WiFiManager::handleSavePro, this));
	  server->on("/saveATP", std::bind(&WiFiManager::handleSaveATP, this));
  #endif
  
  server->onNotFound (std::bind(&WiFiManager::handleNotFound, this));
  server->begin(); // Web server start
  DEBUG_WM(F("HTTP server started"));

}

boolean WiFiManager::autoConnect() {
  String ssid = "ESP" + String(ESP.getChipId());
  return autoConnect(ssid.c_str(), NULL);
}
/* This is not very useful as there has been an assumption that device has to be
told to connect but Wifi already does it's best to connect in background. Calling this
method will block until WiFi connects. Sketch can avoid
blocking call then use (WiFi.status()==WL_CONNECTED) test to see if connected yet.
See some discussion at https://github.com/tzapu/WiFiManager/issues/68
*/
boolean WiFiManager::autoConnect(char const *apName, char const *apPassword) {
  DEBUG_WM(F(""));
  DEBUG_WM(F("AutoConnect"));

  // read eeprom for ssid and pass
  //String ssid = getSSID();
  //String pass = getPassword();

  // device will attempt to connect by itself; wait 10 secs
  // to see if it succeeds and should it fail, fall back to AP
  WiFi.mode(WIFI_STA);
  unsigned long startedAt = millis();
    while(millis() - startedAt < 10000)
    {
        delay(100);
        if (WiFi.status()==WL_CONNECTED) {
			float waited = (millis()- startedAt);
			DEBUG_WM(F("After waiting "));
			DEBUG_WM(waited/1000);
			DEBUG_WM(F(" secs local ip: "));
			DEBUG_WM(WiFi.localIP());
            return true;
		}
    }

  return startConfigPortal(apName, apPassword);
}

boolean  WiFiManager::startConfigPortal() {
  String ssid = "ESP" + String(ESP.getChipId());
  return startConfigPortal(ssid.c_str(),NULL);
}

boolean  WiFiManager::startConfigPortal(char const *apName, char const *apPassword) {
  //setup AP
  int connRes = WiFi.waitForConnectResult();
  if (connRes == WL_CONNECTED){
	  WiFi.mode(WIFI_AP_STA); //Dual mode works fine if it is connected to WiFi
	  DEBUG_WM("SET AP STA");
  	}
  	else {
    WiFi.mode(WIFI_AP); // Dual mode becomes flaky if not connected to a WiFi network.
    // When ESP8266 station is trying to find a target AP, it will scan on every channel,
    // that means ESP8266 station is changing its channel to scan. This makes the channel of ESP8266 softAP keep changing too..
    // So the connection may break. From http://bbs.espressif.com/viewtopic.php?t=671#p2531
    DEBUG_WM("SET AP");
	}
  _apName = apName;
  _apPassword = apPassword;

  //notify we entered AP mode
  if ( _apcallback != NULL) {
    _apcallback(this);
  }

  connect = false;
  setupConfigPortal();
  bool TimedOut=true;
  while (_configPortalTimeout == 0 || millis() < _configPortalStart + _configPortalTimeout) {
    //DNS
    dnsServer->processNextRequest();
    //HTTP
    server->handleClient();


    if (connect) {
      connect = false;
      TimedOut=false;
      delay(2000);
      DEBUG_WM(F("Connecting to new AP"));

      // using user-provided  _ssid, _pass in place of system-stored ssid and pass
      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        DEBUG_WM(F("Failed to connect."));
        WiFi.mode(WIFI_AP); // Dual mode becomes flaky if not connected to a WiFi network.
		    // I think this might be because too much of the processor is being utilised
    //trying to connect to the network.
      } else {
        //notify that configuration has changed and any optional parameters should be saved
        if ( _savecallback != NULL) {
          //todo: check if any custom parameters actually exist, and check if they really changed maybe
          _savecallback();
        }
        //break;
      }

      if (_shouldBreakAfterConfig) {
        //flag set to exit after config after trying to connect
        //notify that configuration has changed and any optional parameters should be saved
        if ( _savecallback != NULL) {
          //todo: check if any custom parameters actually exist, and check if they really changed maybe
          _savecallback();
        }
        break;
      }
    }
    if (stopConfigPortal) {
	  stopConfigPortal = false;
	  break;
    }
    yield();
  }
  WiFi.mode(WIFI_STA);
  if (TimedOut & WiFi.status() != WL_CONNECTED) {
	WiFi.begin();
    int connRes = waitForConnectResult();
    DEBUG_WM ("Timed out connection result: ");
    DEBUG_WM ( getStatus(connRes));
    }
  server.reset();
  dnsServer.reset();
  return  WiFi.status() == WL_CONNECTED;
}

int WiFiManager::connectWifi(String ssid, String pass) {
  DEBUG_WM(F("Connecting wifi with new parameters..."));
  if (ssid != "") {
	resetSettings(); /*Disconnect from the network and wipe out old values
	if no values were entered into form. If you don't do this
    esp8266 will sometimes lock up when SSID or password is different to
	the already stored values and device is in the process of trying to connect
	to the network. Mostly it doesn't but occasionally it does.
	*/
  // check if we've got static_ip settings, if we do, use those.
  if (_sta_static_ip) {
	    DEBUG_WM(F("Custom STA IP/GW/Subnet"));
	    WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
	    DEBUG_WM(WiFi.localIP());
  }
	WiFi.mode(WIFI_AP_STA); //It will start in station mode if it was previously in AP mode.
    WiFi.begin(ssid.c_str(), pass.c_str());// Start Wifi with new values.
  } else if(!WiFi.SSID()) {
      DEBUG_WM(F("No saved credentials"));
  }

  int connRes = waitForConnectResult();
  DEBUG_WM ("Connection result: ");
  DEBUG_WM ( getStatus(connRes));
  //not connected, WPS enabled, no pass - first attempt
  if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
    startWPS();
    //should be connected at the end of WPS
    connRes = waitForConnectResult();
  }
  return connRes;
}

uint8_t WiFiManager::waitForConnectResult() {
  if (_connectTimeout == 0) {
	unsigned long startedAt = millis();
	DEBUG_WM (F("After waiting..."));
	int connRes = WiFi.waitForConnectResult();
	float waited = (millis()- startedAt);
	DEBUG_WM (waited/1000);
	DEBUG_WM (F("seconds"));
    return connRes;
  } else {
    DEBUG_WM (F("Waiting for connection result with time out"));
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        DEBUG_WM (F("Connection timed out"));
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}

void WiFiManager::startWPS() {
  DEBUG_WM(F("START WPS"));
  WiFi.beginWPSConfig();
  DEBUG_WM(F("END WPS"));
}
//Convenient for debugging but wasteful of program space.
//Remove if short of space
char* WiFiManager::getStatus(int status)
{
   switch (status)
   {
      case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
      case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
      case WL_CONNECTED: return "WL_CONNECTED";
	  case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
      case WL_DISCONNECTED: return "WL_DISCONNECTED";
      default: return "UNKNOWN";
   }
}

String WiFiManager::getConfigPortalSSID() {
  return _apName;
}

void WiFiManager::resetSettings() {
  DEBUG_WM(F("previous settings invalidated"));
  WiFi.disconnect(true);
  delay(200);
  return;
}
void WiFiManager::setTimeout(unsigned long seconds) {
  setConfigPortalTimeout(seconds);
}

void WiFiManager::setConfigPortalTimeout(unsigned long seconds) {
  _configPortalTimeout = seconds * 1000;
}

void WiFiManager::setConnectTimeout(unsigned long seconds) {
  _connectTimeout = seconds * 1000;
}

void WiFiManager::setDebugOutput(boolean debug) {
  _debug = debug;
}

void WiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _ap_static_ip = ip;
  _ap_static_gw = gw;
  _ap_static_sn = sn;
}

void WiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _sta_static_ip = ip;
  _sta_static_gw = gw;
  _sta_static_sn = sn;
}

void WiFiManager::setMinimumSignalQuality(int quality) {
  _minimumQuality = quality;
}

void WiFiManager::setBreakAfterConfig(boolean shouldBreak) {
  _shouldBreakAfterConfig = shouldBreak;
}

void WiFiManager::reportStatus(String &page){
  if (WiFi.SSID() != ""){
	  page += F("Configured to connect to access point ");
	  page += WiFi.SSID();
	  if (WiFi.status()==WL_CONNECTED){
		  page += F(" and <strong>currently connected</strong> on IP <a href=\"http://");
		  page += WiFi.localIP().toString();
		  page += F("/\">");
		  page += WiFi.localIP().toString();
		  page += F("</a>");
	   }
	  else {
		  page += F(" but <strong>not currently connected</strong> to network.");
	  }
    }
    else {
		page += F("No network currently configured.");
	}
}

/** Handle root or redirect to captive portal */
void WiFiManager::handleRoot() {
  DEBUG_WM(F("Handle root"));
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
      return;
  }
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += "<h2>";
  page += _apName;
  if (WiFi.SSID() != ""){
	  if (WiFi.status()==WL_CONNECTED){
		  page += " on ";
		  page += WiFi.SSID();
	  }
	  else{
		  page += " <s>on ";
		  page += WiFi.SSID();
		  page += "</s>";
	  }

  }
  page += "</h2>";
  page += FPSTR(HTTP_PORTAL_OPTIONS);
  //DEBUG_WM(F("_paramsCount"));
  //DEBUG_WM(_paramsCount);
  if (_paramsCount > 0){
	  page.replace("{a}", FPSTR(HTTP_PORTAL_APPCFG));
  }else{
	  page.replace("{a}", "");
  }
  if (WiFi.status()==WL_CONNECTED){
	  page.replace("{q}", FPSTR(HTTP_PORTAL_IASCFG));
  }else{
	  page.replace("{q}", "");
  }
  
  
  page += F("<div class=\"msg\">");
  reportStatus(page);
  page += F("</div>");
  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

}


#if IASCNF == 1
/** IOTAppStory config page handler */
void WiFiManager::handleIAScfg() {

	if(server->arg("d") != ""){
		// check the entered code
		hdlIasCfgPages("IOTAppStory.com config","ias-check.php","&ias_id="+server->arg("d"));
		
	}else if(devPass == "000000" || devPass == ""){
		// send "enter your code" page
		hdlIasCfgPages("IOTAppStory.com config","ias-id.php");
		
	}else{
		// check device status
		hdlIasCfgPages("IOTAppStory.com config","dev_check.php","&ias_id="+devPass);
	}
}

/** IOTAppStory add device page handler */
void WiFiManager::handleAddPage() {
	hdlIasCfgPages("Add Device","dev_new.php");
}


/** IOTAppStory edit project page handler */
void WiFiManager::handleEditProPage() {
	hdlIasCfgPages("Edit Project","proj_edit.php");
}


/** IOTAppStory new project page handler */
void WiFiManager::handleNewProPage() {
	hdlIasCfgPages("New Project","proj_edit.php","&new=1");
}


/** IOTAppStory add to existing project page handler */
void WiFiManager::handleAddToProPage() {
	hdlIasCfgPages("Add to a Project","proj_addto.php");
}


void WiFiManager::hdlIasCfgPages(String title, String file, String para){
	const char* host = "iotappstory.com";
	
	////Serial.print("connecting to ");
	////Serial.println(host);

	// Use WiFiClient class to create TCP connections
	WiFiClient client;
	
	if (!client.connect(host, 80)) {
		//Serial.println("connection failed");
		return;
	}

	// We now create a URI for the request
	String url = "/ota/cnf/" + file;
	url += "?chip_id=";
	url += ESP.getChipId();
	url += "&flash_chip_id=";
	url += ESP.getFlashChipId();
	url += "&flash_size=";
	url += ESP.getFlashChipRealSize();
	url += "&mac=";
	url += WiFi.macAddress();
	url += para;

	//Serial.print("Requesting URL: ");
	//Serial.println(url);

	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1 " + "\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
	unsigned long timeout = millis();
	while (client.available() == 0) {
		if (millis() - timeout > 5000) {
			Serial.println(">>> Client Timeout !");
			client.stop();
			return;
		}
	}
	String line = "";

	int st = 0;
	// Read all the lines of the reply from server and print them to Serial
	while(client.available()){
			// from : close
			if(st == 0){
				client.readStringUntil('{hstart}');
				st = 1;
			}else if(st == 1){
				line += client.readStringUntil('\r');
				st = 2;
			}else if(st == 2){
				if(client.readStringUntil('\r') == "1"){
					Serial.println(client.readStringUntil('\r'));
					devPass = server->arg("d");
				}
				st = 3;
			}
	}
	//Serial.println(line);
	

  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");

  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", title);
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  
  page += line;
  
  page += FPSTR(HTTP_END);
  


  server->send(200, "text/html", page);

  DEBUG_WM("Sent " + title);
}
#endif


/** Wifi config page handler */
void WiFiManager::handleWifi() {
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += F("<h2>Wifi Configuration</h2>");
  //Print list of WiFi networks that were found in earlier scan
    if (numberOfNetworks == 0) {
      page += F("WiFi scan found no networks. Restart configuration portal to scan again.");
    } else {
      //display networks in page
      for (int i = 0; i < numberOfNetworks; i++) {
        if(networkIndices[i] == -1) continue; // skip dups and those that are below the required quality

        DEBUG_WM(WiFi.SSID(networkIndices[i]));
        DEBUG_WM(WiFi.RSSI(networkIndices[i]));
        int quality = getRSSIasQuality(WiFi.RSSI(networkIndices[i]));

        String item = FPSTR(HTTP_ITEM);
        String rssiQ;
        rssiQ += quality;
        item.replace("{v}", WiFi.SSID(networkIndices[i]));
        item.replace("{r}", rssiQ);
        if (WiFi.encryptionType(networkIndices[i]) != ENC_TYPE_NONE) {
          item.replace("{i}", "l");
        } else {
            item.replace("{i}", "");
        }
        //DEBUG_WM(item);
        page += item;
        delay(0);
        }
    page += "<br/>";
    }

  page += FPSTR(HTTP_WFORM_START);
  //char parLength[2];


  if (_sta_static_ip) {

    String item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "ip");
    item.replace("{n}", "ip");
    item.replace("{p}", "Static IP");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_ip.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "gw");
    item.replace("{n}", "gw");
    item.replace("{p}", "Static Gateway");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_gw.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "sn");
    item.replace("{n}", "sn");
    item.replace("{p}", "Subnet");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_sn.toString());

    page += item;

    page += "<br/>";
  }

  page += FPSTR(HTTP_FORM_BTN);
  page.replace("{t}", "submit");
  page.replace("{b}", "Save");
  page += FPSTR(HTTP_FORM_END);

  
  page += FPSTR(HTTP_FORM_BACKBTN);

  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent config page"));
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WiFiManager::handleWifiSave() {
  DEBUG_WM(F("WiFi save"));

  //SAVE/connect here
  _ssid = server->arg("s").c_str();
  _pass = server->arg("p").c_str();



  if (server->arg("ip") != "") {
    DEBUG_WM(F("static ip"));
    DEBUG_WM(server->arg("ip"));
    //_sta_static_ip.fromString(server->arg("ip"));
    String ip = server->arg("ip");
    optionalIPFromString(&_sta_static_ip, ip.c_str());
  }
  if (server->arg("gw") != "") {
    DEBUG_WM(F("static gateway"));
    DEBUG_WM(server->arg("gw"));
    String gw = server->arg("gw");
    optionalIPFromString(&_sta_static_gw, gw.c_str());
  }
  if (server->arg("sn") != "") {
    DEBUG_WM(F("static netmask"));
    DEBUG_WM(server->arg("sn"));
    String sn = server->arg("sn");
    optionalIPFromString(&_sta_static_sn, sn.c_str());
  }

  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Credentials Saved");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_SAVED);
  page.replace("{v}", _apName);
  page.replace("{x}", _ssid);
  page += FPSTR(HTTP_FORM_BACKBTN);
  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent wifi save page"));

  connect = true; //signal ready to connect/reset
}



/** Wifi config page handler */
void WiFiManager::handleApp() {
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Config App");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += F("<h2>App Configuration</h2>");

  page += FPSTR(HTTP_AFORM_START);
  //char parLength[2];
  // add the extra parameters to the form
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
		break;
    }

	String pitem;
	switch (_params[i]->getLabelPlacement()) {
		case WFM_LABEL_BEFORE:
			pitem = FPSTR(HTTP_FORM_LABEL);
			pitem += FPSTR(HTTP_FORM_PARAM);
		break;
		case WFM_LABEL_AFTER:
			pitem = FPSTR(HTTP_FORM_PARAM);
			pitem += FPSTR(HTTP_FORM_LABEL);
		break;
		default:
			// WFM_NO_LABEL
			pitem = FPSTR(HTTP_FORM_PARAM);
		break;
	}

    if (_params[i]->getID() != NULL) {
		pitem.replace("{i}", _params[i]->getID());
		pitem.replace("{n}", _params[i]->getID());
		pitem.replace("{p}", _params[i]->getPlaceholder());
		//snprintf(parLength, 2, "%d", _params[i]->getValueLength());
		pitem.replace("{l}", String(_params[i]->getValueLength()-1));
		pitem.replace("{v}", _params[i]->getValue());
		pitem.replace("{c}", _params[i]->getCustomHTML());
    } else {
		pitem = _params[i]->getCustomHTML();
    }

    page += pitem;
  }
  if (_params[0] != NULL) {
    page += "<br/>";
  }



  page += FPSTR(HTTP_FORM_BTN);
  page.replace("{t}", "submit");
  page.replace("{b}", "Save");
  page += FPSTR(HTTP_FORM_END);

  
  page += FPSTR(HTTP_FORM_BACKBTN);

  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent config page"));
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WiFiManager::handleAppSave() {
  DEBUG_WM(F("App save"));



  //parameters
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }
    //read parameter
    String value = server->arg(_params[i]->getID()).c_str();
    //store it in array
    value.toCharArray(_params[i]->_value, _params[i]->_length);
    DEBUG_WM(F("Parameter"));
    DEBUG_WM(_params[i]->getID());
    DEBUG_WM(value);
  }


  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "App Config Saved");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_APPSAVED);


  
  page += FPSTR(HTTP_FORM_BACKBTN);
  
  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent app save page"));


}












/** Handle shut down the server page */
void WiFiManager::handleServerClose() {
    DEBUG_WM(F("Server Close"));
    server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server->sendHeader("Pragma", "no-cache");
    server->sendHeader("Expires", "-1");
    String page = FPSTR(HTTP_HEAD);
    page.replace("{v}", "Close Server");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += F("<div class=\"msg\">");
    page += F("My network is <strong>");
    page += WiFi.SSID();
    page += F("</strong><br>");
    page += F("My IP address is <strong>");
    page += WiFi.localIP().toString();
    page += F("</strong><br><br>");
    page += F("Configuration server closed...<br><br>");
    //page += F("Push button on device to restart configuration server!");
    page += FPSTR(HTTP_END);
    server->send(200, "text/html", page);
    stopConfigPortal = true; //signal ready to shutdown config portal
  DEBUG_WM(F("Sent server close page"));

}
/** Handle the info page */
void WiFiManager::handleInfo() {
  DEBUG_WM(F("Info"));
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += F("<h2>Information</h2>");			// removed "Wifi" from the title
  //page += F("Android app from <a href=\"https://play.google.com/store/apps/details?id=au.com.umranium.espconnect\">https://play.google.com/store/apps/details?id=au.com.umranium.espconnect</a> provides easier ESP WiFi configuration.<p/>");
  // removed this add because the app refered to won't work with the IAS loader
  reportStatus(page);
  
  page += FPSTR(HTTP_PORTAL_INFO);
  page.replace("{si}", WiFi.SSID());
  page.replace("{api}", WiFi.softAPIP().toString());
  page.replace("{apm}", WiFi.softAPmacAddress());
  page.replace("{cid}", String(ESP.getChipId()));
  page.replace("{fid}", String(ESP.getFlashChipId()));
  page.replace("{ifs}", String(ESP.getFlashChipSize()));
  page.replace("{rfs}", String(ESP.getFlashChipRealSize()));
  page.replace("{dip}", WiFi.localIP().toString());
  page.replace("{mac}", WiFi.macAddress());
  page += "<br/>";
  
  page += FPSTR(HTTP_FORM_BACKBTN);
  
  page += FPSTR(HTTP_END);

  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent info page"));
}
/** Handle the state page */
void WiFiManager::handleState() {
  DEBUG_WM(F("State - json"));
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = F("{\"Soft_AP_IP\":\"");
  page += WiFi.softAPIP().toString();
  page += F("\",\"Soft_AP_MAC\":\"");
  page += WiFi.softAPmacAddress();
  page += F("\",\"Station_IP\":\"");
  page += WiFi.localIP().toString();
  page += F("\",\"Station_MAC\":\"");
  page += WiFi.macAddress();
  page += F("\",");
  if (WiFi.psk()!=""){
  	  page += F("\"Password\":true,");
    }
  else {
  	  page += F("\"Password\":false,");
    }
  page += F("\"SSID\":\"");
  page += WiFi.SSID();
  page += F("\"}");
  server->send(200, "application/json", page);
  DEBUG_WM(F("Sent state page in json format"));
}

/** Handle the scan page */
void WiFiManager::handleScan() {
  DEBUG_WM(F("State - json"));
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");

  int n;
  int *indices;
  int **indicesptr = &indices;
  //Space for indices array allocated on heap in scanWifiNetworks
  //and should be freed when indices no longer required.
  n = scanWifiNetworks(indicesptr);
  DEBUG_WM(F("In handleScan, scanWifiNetworks done"));
  String page = F("{\"Access_Points\":[");
  //display networks in page
  for (int i = 0; i < n; i++) {
          if(indices[i] == -1) continue; // skip duplicates and those that are below the required quality
          if(i != 0) page += F(", ");
          DEBUG_WM(WiFi.SSID(indices[i]));
          DEBUG_WM(WiFi.RSSI(indices[i]));
          int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
          String item = FPSTR(JSON_ITEM);
          String rssiQ;
          rssiQ += quality;
          item.replace("{v}", WiFi.SSID(indices[i]));
          item.replace("{r}", rssiQ);
          if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
            item.replace("{i}", "true");
          } else {
            item.replace("{i}", "false");
          }
          //DEBUG_WM(item);
          page += item;
          delay(0);
  }
  free(indices); //indices array no longer required so free memory
  page += F("]}");
  server->send(200, "application/json", page);
  DEBUG_WM(F("Sent WiFi scan data ordered by signal strength in json format"));
}








#if IASCNF == 1
/* Handle the save device to IAS request */				// added for IAS
void WiFiManager::handleDevSave() {

	String devName = server->arg("n").c_str();
	devName.replace(" ", "%20");
	
	hdlIasCfgPages("Device added to IOTAppStory.com","dev_save.php","&ias_id="+devPass+"&name="+devName+"&dt="+server->arg("d").c_str());
}


/* Handle the add device to IAS request */				// added for IAS
void WiFiManager::handleSavePro() {

	String name = server->arg("n").c_str();
	name.replace(" ", "%20");
	
	hdlIasCfgPages("Project saved","proj_save.php","&ias_id="+devPass+"&id="+server->arg("id")+"&name="+name+"&app="+server->arg("a"));
}


/* Handle the add device to IAS request */				// added for IAS
void WiFiManager::handleSaveATP() {

	hdlIasCfgPages("Added to Project","proj_addto_save.php","&ias_id="+devPass+"&proj="+server->arg("p"));
}
#endif







/** Handle the reset page */
void WiFiManager::handleReset() {
  DEBUG_WM(F("Reset"));
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "WiFi Information");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += F("Module will reset in a few seconds.");
  page += FPSTR(HTTP_END);
  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent reset page"));
  delay(5000);
  WiFi.disconnect(true); // Wipe out WiFi credentials.
  ESP.reset();
  delay(2000);
}

void WiFiManager::handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
      return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += ( server->method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";

  for ( uint8_t i = 0; i < server->args(); i++ ) {
    message += " " + server->argName ( i ) + ": " + server->arg ( i ) + "\n";
  }
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->send ( 404, "text/plain", message );
}

/** Redirect to captive portal if we got a request for another domain. Return true in
that case so the page handler do not try to handle the request again. */
boolean WiFiManager::captivePortal() {
  if (!isIp(server->hostHeader()) && server->hostHeader() != (String(myHostname))) {
    DEBUG_WM(F("Request redirected to captive portal"));
    server->sendHeader("Location", ("http://") +String(myHostname), true);
    server->setContentLength(0);
    server->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
//    server->client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

//start up config portal callback
void WiFiManager::setAPCallback( void (*func)(WiFiManager* myWiFiManager) ) {
  _apcallback = func;
}

//start up save config callback
void WiFiManager::setSaveConfigCallback( void (*func)(void) ) {
  _savecallback = func;
}

//sets a custom element to add to head, like a new style tag
void WiFiManager::setCustomHeadElement(const char* element) {
  _customHeadElement = element;
}

//if this is true, remove duplicated Access Points - defaut true
void WiFiManager::setRemoveDuplicateAPs(boolean removeDuplicates) {
  _removeDuplicateAPs = removeDuplicates;
}

//Scan for WiFiNetworks in range and sort by signal strength
//space for indices array allocated on the heap and should be freed when no longer required
int WiFiManager::scanWifiNetworks(int **indicesptr) {
    int n = WiFi.scanNetworks();
    DEBUG_WM(F("Scan done"));
    if (n == 0) {
      DEBUG_WM(F("No networks found"));
      return(0);
    } else {
	  // Allocate space off the heap for indices array.
	  // This space should be freed when no longer required.
 	  int* indices = (int *)malloc(n*sizeof(int));
					if (indices == NULL){
						DEBUG_WM(F("ERROR: Out of memory"));
						return(0);
						}
	  *indicesptr = indices;
      //sort networks
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }

      std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
      {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
      });
      // remove duplicates ( must be RSSI sorted )
      if(_removeDuplicateAPs) {
        String cssid;
        for (int i = 0; i < n; i++) {
          if(indices[i] == -1) continue;
          cssid = WiFi.SSID(indices[i]);
          for (int j = i + 1; j < n; j++) {
            if(cssid == WiFi.SSID(indices[j])){
              DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
              indices[j] = -1; // set dup aps to index -1
            }
          }
        }
      }

      for (int i = 0; i < n; i++) {
        if(indices[i] == -1) continue; // skip dups

        int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
        if (!(_minimumQuality == -1 || _minimumQuality < quality)) {
          indices[i] == -1;
          DEBUG_WM(F("Skipping due to quality"));
        }
      }

      return (n);
    }
}

template <typename Generic>
void WiFiManager::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WM: ");
    Serial.println(text);
  }
}

int WiFiManager::getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

/** Is this an IP? */
boolean WiFiManager::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String WiFiManager::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
