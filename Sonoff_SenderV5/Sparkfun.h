
// my Chip IDs to give it a cleartype name
#define WA1 14117293
#define WA2 12612352

// Call: https://data.sparkfun.com/streams/mKlDLm9zwVH7zwyj7orK/update/kzqmYX8GowsvG5eDvlrK


bool sendSparkfun() {

  // Use Sparkfun testing stream
  const char* host = "data.sparkfun.com";
  const char* streamId   = "mKlDLm9zwVH7zwyj7orK";
  const char* privateKey = "kzqmYX8GowsvG5eDvlrK";

  DEBUG_PRINT("Connecting to "); DEBUG_PRINT(host);

  WiFiClient client;
  int retries = 5;
  while (!client.connect(host, 80) && (retries-- > 0)) {
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN();
  if (!client.connected()) {
    DEBUG_PRINTLN("Failed to connect to Sparkfun");
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

  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Sparkfun: ");
  DEBUG_PRINTLN();

  DEBUG_PRINT("Request URL: ");
  DEBUG_PRINTLN(url);

  client.print(String("GET ") + url +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  int timeout = 5 * 10; // 5 seconds
  while (!client.available() && (timeout-- > 0)) {
    delay(100);
  }

  if (!client.available()) {
    DEBUG_PRINTLN("No response from Sparkfun");
    return false;
  }
  DEBUG_PRINTLN("disconnected");
  return true;
}
