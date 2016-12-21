
// Call: https://data.sparkfun.com/streams/mKlDLm9zwVH7zwyj7orK/update/kzqmYX8GowsvG5eDvlrK


bool sendSparkfun() {

  // Use Sparkfun testing stream
  const char* host = "data.sparkfun.com";
  const char* streamId   = "mKlDLm9zwVH7zwyj7orK";
  const char* privateKey = "kzqmYX8GowsvG5eDvlrK";

  Serial.print("Connecting to "); Serial.print(host);

  WiFiClient client;
  int retries = 5;
  while (!client.connect(host, 80) && (retries-- > 0)) {
    Serial.print(".");
    // ESP.wdtFeed();    // feed the watchdog
  }
  Serial.println();
  if (!client.connected()) {
    Serial.println("Failed to connect to Sparkfun");
    return false;
  }

  String url = "/input/";
  url += streamId;
  url += "?private_key=";
  url += privateKey;
  url += "&name=";
  url += boardName;
  url += "&boots=";
  url += boardName;
  url += "&freeheap=";
  url += ESP.getFreeHeap();

  Serial.println();
  Serial.println("Sparkfun: ");
  Serial.println();

  Serial.print("Request URL: "); Serial.println(url);

  client.print(String("GET ") + url +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  int timeout = 5 * 10; // 5 seconds
  while (!client.available() && (timeout-- > 0)) {
    delay(100);
    // ESP.wdtFeed();    // feed the watchdog
  }

  if (!client.available()) {
    Serial.println("No response from Sparkfun");
    return false;
  }
  Serial.println(F("disconnected"));
  return true;
}
