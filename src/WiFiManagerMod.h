/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   This version built by Ken Taylor https://github.com/kentaylor
   Modified by IOTAppStory.com
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include "IOTAppStory.h"
#include <memory>
#undef min
#undef max

extern "C" {
  #include "user_interface.h"
}

#define IASCNF 1
#define WFM_LABEL_BEFORE 1
#define WFM_LABEL_AFTER 2
#define WFM_NO_LABEL 0
#define WIFI_MANAGER_MAX_PARAMS 25

const char HTTP_200[] PROGMEM             = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>html{width:100%;height:100%;padding:0;margin:0;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}*, *:before, *:after{-webkit-box-sizing:inherit;-moz-box-sizing:inherit;box-sizing:inherit}body,textarea,input,select{background:0;border-radius:0;font:16px sans-serif;margin:0}body{background:#000;color:#FFF}body a{color:#FFF}body a:hover{text-decoration:none}textarea,input,select{outline:0;font-size:14px;color:#FFF;border:1px solid #ccc;padding:8px;width:100%}select{color:#FFF;-webkit-appearance: none;}.container{margin:auto;width:90%}@media(min-width:1200px){.container{margin:auto;width:30%}}@media(min-width:768px) and (max-width:1200px){.container{margin:auto;width:50%}}.btn,h2{font-size:2em}h1{font-size:3em}.btn{background:#fcbc12;border-radius:4px;border:0;color:#fff;text-align:center;text-decoration:none;display:inline-block;margin:10px 0;padding:9px 14px 10px;width:100%}.btn:hover{background:#d3d3d3}.btn:active,.btn:focus{background:#d3d3d3}label>*{display:inline}form>*{display:block;margin-bottom:10px}textarea:focus,input:focus,select:focus{border-color:#fcbc12}.msg{background:#364048;margin:10px 0;padding:1.5em;border-radius:4px}.q{float:right;width:64px;text-align:right}.l{background:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAl5JREFUeNrsV89rE0EUfm83odVaFKmWhlIPWlAUxFZb8SSieJEWeirqUXrxqH+C99JDjz3k4MGDYPVUCoInUWpOokJVEIRiaSldE8Qks89vNhPdbjbZ3XRLD2bIx87uzLz3zfs1ExYR2s+WafwEQuUNPN2otTbQCzDgAKpBjtUNDUcSEqhukaxcJylvQnQmbM0tYBoYAfrMN824ADwBXnhfVJH4+CTxuYWEBOASqWwDW0ECw8A8cCNEzjHgDHAHWAbug8AqqZJnnmQEsILZrin/R+Ai8BwYiOFWTfAV1k4wWSvJY0C8H/lCcxBYDFGuzf7F9E/63EG1ubJIVnYc+/neioAVY0dzQM73/gt4CJwFLhvo/gMzZiR35aj4aU6+PiIqfWgqnBvSsLJJ7usxBOGGdoEOtHe+0d/AJLDURN5NY60u+BGJVEFGOaPWhccFHrjdlgWmAu+zLZSTGZutB7MXQ5nsFHG2bReM+hMUyMdwWd7MDZORmMBRXx95SWsxCKyZuWEyEhPwj6udydG0SaAqWrshkMZBIbshoAKCnBgKnZ1KRbF9MEkl1FXQOoXetUD+9/hynVvs9oCZ+7ceyPqzGS6vv6Tc3c/BjGisA6WPJ9y3V9+IW+6PV6cimq4H1Z+6vP+wrhTGqef0t9YuKL4fw0GUjvJ6PbAP4VHth+xLkTEgnnWZ0m+s/cNxgnAvr0jSzmG0p61DoEOgQ2DfCYT881B2vR6mW4HEyI4gwN1Dq7poinJSNJBbK/CQHW2B3vMFHrw3TdVtXEjtbDoEVIUzh59q2dHX8v8tCP8IMADfe61URz1LDwAAAABJRU5ErkJggg==') no-repeat left center;background-size:1em}input[type='checkbox']{float:left;width:20px}.table{width:100%}.table td{padding:.5em;text-align:left}.table tbody>:nth-child(2n-1){background:#364048}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div class=\"container\">";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<a href=\"/wifi\" class=\"btn\">Wifi</a>{a}{q}<a href=\"/i\" class=\"btn\">Information</a><a href=\"/close\" class=\"btn\">Exit Portal</a>";

const char HTTP_PORTAL_IASCFG[] PROGMEM   = "<a href=\"/ias\" class=\"btn\">IOTAppStory</a>";
const char HTTP_PORTAL_APPCFG[] PROGMEM   = "<a href=\"/app\" class=\"btn\">Application</a>";

const char HTTP_PORTAL_INFO[] PROGMEM     = "<br><h3>AccessPoint Data</h3><table class=\"table\"><thead><tr><th>Name</th><th>Value</th></tr></thead><tbody><tr><td>SSID</td><td>{si}</td></tr><tr><td>Access Point IP</td><td>{api}</td></tr><tr><td>Access Point MAC</td><td>{apm}</td></tr></tbody></table><br><h3>Device Data</h3><table class=\"table\"><thead><tr><th>Name</th><th>Value</th></tr></thead><tbody><tr><td>Chip ID</td><td>{cid}</td></tr><tr><td>Flash Chip ID</td><td>{fid}</td></tr><tr><td>IDE Flash Size</td><td>{ifs} bytes</td></tr><tr><td>Real Flash Size</td><td>{rfs} bytes</td></tr><tr><td>Device IP</td><td>{dip}</td></tr><tr><td>Device MAC</td><td>{mac}</td></tr></tbody></table><br><h3>Available Pages</h3><table class=\"table\"><thead><tr><th>Page</th><th>Function</th></tr></thead><tbody><tr><td><a href=\"/\">/</a></td><td>Menu page.</td></tr><tr><td><a href=\"/wifi\">/wifi</a></td><td>Show WiFi scan results and enter WiFi configuration.</td></tr><tr><td><a href=\"/wifisave\">/wifisave</a></td><td>Save WiFi configuration information and configure device. Needs variables supplied.</td></tr><tr><td><a href=\"/close\">/close</a></td><td>Close the configuration server and configuration WiFi network.</td></tr><tr><td><a href=\"/i\">/i</a></td><td>This page.</td></tr><tr><td><a href=\"/r\">/r</a></td><td>Delete WiFi configuration and reboot. ESP device will not reconnect to a network until new WiFi configuration data is entered.</td></tr><tr><td><a href=\"/state\">/state</a></td><td>Current device state in JSON format. Interface for programmatic WiFi configuration.</td></tr><tr><td><a href=\"/scan\">/scan</a></td><td>Run a WiFi scan and return results in JSON format. Interface for programmatic WiFi configuration.</td></tr></table><p/>More information about WiFiManager at <a href=\"https://github.com/kentaylor/WiFiManager\">https://github.com/kentaylor/WiFiManager</a>.";

const char HTTP_ITEM[] PROGMEM            = "<div><a href=\"#p\" onclick=\"c(this)\">{v}</a>&nbsp;<span class=\"q {i}\">{r}%</span></div>";
const char JSON_ITEM[] PROGMEM            = "{\"SSID\":\"{v}\", \"Encryption\":{i}, \"Quality\":\"{r}\"}";

const char HTTP_WFORM_START[] PROGMEM      = "<form method=\"get\" action=\"wifisave\"><label>SSID</label><input id=\"s\" name=\"s\" maxlength=50 placeholder=\"SSID\"><label>Password</label><input id=\"p\" name=\"p\" maxlength=50 placeholder=\"password\"><small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small></br></br>";
const char HTTP_AFORM_START[] PROGMEM      = "<form method=\"get\" action=\"appsave\">";

const char HTTP_FORM_BACKBTN[] PROGMEM    = "<a href=\"\\\" class=\"btn\">< Back</a>";

const char HTTP_FORM_LABEL[] PROGMEM      = "<label for=\"{i}\">{p}</label>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<input id=\"{i}\" name=\"{n}\" maxlength={l} placeholder=\"{p}\" value=\"{v}\" {c}>";
const char HTTP_FORM_END[] PROGMEM        = "</form>";

const char HTTP_FORM_BTN[] PROGMEM        = "<button class=\"btn\" type=\"{t}\">{b}</button>";

const char HTTP_SAVED[] PROGMEM           = "<div class=\"msg\"><strong>Credentials Saved</strong><br>Trying to connect ESP to the {x} network.<br>Press Back after waiting at least 10 seconds.<br>You may have to manually reconnect to the {v} network.</div>";
const char HTTP_APPSAVED[] PROGMEM        = "<div class=\"msg\"><strong>App Configuration Saved</strong><br>Your configuration has been saved.</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";



class WiFiManagerParameter {
  public:
    WiFiManagerParameter();
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    int         getLabelPlacement();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
	int         _labelPlacement;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();
    ~WiFiManager();

    boolean       autoConnect(); //Deprecated. Do not use.
    boolean       autoConnect(char const *apName, char const *apPassword = NULL); //Deprecated. Do not use.

    //if you want to start the config portal
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();
	
	String 		  devPass = "000000"; 						// added for IOTAppStory

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);
    //Scan for WiFiNetworks in range and sort by signal strength
    //space for indices array allocated on the heap and should be freed when no longer required
    int           scanWifiNetworks(int **indicesptr);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();
    char*         getStatus(int status);

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;
    /* hostname for mDNS. Set to a valid internet address so that user
    will see an information page if they are connected to the wrong network */
	const char *myHostname = "wifi.urremote.com";
	int numberOfNetworks;
	int *networkIndices;
    int **networkIndicesptr = &networkIndices;
	

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
	void          hdlReturn(String &message, String type="text/html");		// added for IAS	(<- saves mem)
    void          handleWifi();
    void          handleWifiSave();
	
	void          handleApp();
	void          handleAppSave();
	
    void          handleServerClose();
    void          handleInfo();
    void          handleState();
    void          handleScan();

#if IASCNF == 1
    void          handleIAScfg();				// added for IAS
    void          handleAddPage();				// added for IAS
    void          handleDevSave();				// added for IAS
    void          handleEditProPage();			// added for IAS
    void          handleNewProPage();			// added for IAS
    void          handleAddToProPage();			// added for IAS
    void          handleSavePro();				// added for IAS
    void          handleSaveATP();				// added for IAS
	
    void          hdlIasCfgPages(String title, const char *file, String para = "");				// added for IAS
#endif

    void          handleReset();
    void          handleNotFound();
    boolean       captivePortal();
    void          reportStatus(String &page);

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       stopConfigPortal = false;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM(F("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work."));
      return false;
    }
};

#endif
