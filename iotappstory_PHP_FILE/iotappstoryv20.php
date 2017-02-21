<?PHP
header('Content-type: text/plain; charset=utf8', true);
function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}
function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}
if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater!\n";
    exit();
}
if(
    !check_header('HTTP_X_ESP8266_STA_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_FREE_SPACE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
    !check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION')
) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater! (header)\n";
    exit();
}
$db = array(
    "18:FE:34:FA:34:13" => "Dummy_Load_OTA_V2_Nextion.ino.nodemcu",
    "18:FE:AA:AA:AA:AA" => "DOOR-7-g14f53a19",
    "18:FE:AA:AA:AA:BB" => "TEMP-1.0.0"
);
$spiffs_flag = false;	/* untill updatespiffs also supports md5 we have to use this flag */

if(!isset($db[$_SERVER['HTTP_X_ESP8266_STA_MAC']])) {
    header($_SERVER["SERVER_PROTOCOL"].' 404 ESP MAC not configured for update, no .bin file associated with that mac address.', true, 404);
}

$localBinary = "./bin/".$db[$_SERVER['HTTP_X_ESP8266_STA_MAC']].".bin";
$localSpiffs = "./bin/".$db[$_SERVER['HTTP_X_ESP8266_STA_MAC']].".spiffs.bin";


if($_SERVER['HTTP_X_ESP8266_MODE'] == "sketch"){
	// Check if version has been set and does not match, if not, check if
	// MD5 hash between local binary and ESP8266 binary do not match if not.
	// then no update has been found.
	if((!check_header('HTTP_X_ESP8266_SDK_VERSION') && $db[$_SERVER['HTTP_X_ESP8266_STA_MAC']] != $_SERVER['HTTP_X_ESP8266_VERSION']) || $_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($localBinary)) {
		sendFile($localBinary);
	} else {
		header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
	}
}else if($_SERVER['HTTP_X_ESP8266_MODE'] == "spiffs"){
	// Check if version has been set and does not match, if not, check if
	// Check if spiffs.bin exists
	if((!check_header('HTTP_X_ESP8266_SDK_VERSION') && $db[$_SERVER['HTTP_X_ESP8266_STA_MAC']] != $_SERVER['HTTP_X_ESP8266_VERSION']) || file_exists($localSpiffs) && $spiffs_flag == true) {
		sendFile($localSpiffs);
	} else {
		header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified OR Selected', true, 304);
	}
}else{
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater! (header)\n";
    exit();
}
