#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Servo.h>

const char *APssid = "SmartLab";
const char *APpassword = "00000000";

ESP8266WebServer server(80);

Servo myservo;

void handleCheckNetwork()
{
  Serial.println("Checking network status");

  // check for wifi status
  String message = "";
  int status = WiFi.status();
  switch (status)
  {
  case WL_CONNECTED:
    // connected to network
    message += "C: ";
    message += WiFi.SSID();
    break;

  case WL_CONNECT_FAILED:
  case WL_CONNECTION_LOST:
  case WL_DISCONNECTED:
    // connection failed, lost , or disconnected
    message += "F: ";
    message += status;
    break;

  default:
    // not connected yet or idle
    message += "N: ";
    message += status;
    break;
  }
  // send message
  server.send(200, "text/plain", message);
  Serial.println(message);
}

void handleConnectNetwork()
{
  if (!server.hasArg("plain"))
  {
    server.send(200, "text/plain", "empty body");
    return;
  }

  // handle network settings
  // FIXME: currently parsing is very simple
  // "ssid=<SSID>&passwd=<PASSWORD>"
  String message = server.arg("plain");

  if (message != "")
  {
    // return state first
    Serial.println("send");
    server.send(200, "text/plain", "connecting");

    // parse request
    int idx = message.indexOf('&');
    String ssid = message.substring(5, idx);
    idx = message.indexOf('=', idx);
    String passwd = message.substring(idx + 1);

    Serial.print(ssid);
    Serial.print(", ");
    Serial.println(passwd);

    // connecting to the network
    connectToNetwork(ssid, passwd);
  }
  else
  {
    // empty request
    server.send(200, "text/plain", "empty body");
    Serial.println("empty body");
  }
}

void handleScanNetwork()
{
  Serial.println("Scanning...");
  // disconnect from previous network
  WiFi.disconnect();

  // scan network
  int numOfNetwork = WiFi.scanNetworks();
  String message = "";
  message += numOfNetwork;
  message += '\n';
  for (int i = 0; i < numOfNetwork; ++i)
  {
    message += WiFi.SSID(i);
    message += '\n';
  }

  // send message
  server.send(200, "text/plain", message);
  Serial.println(message);
}

void handleServo(){
  String message = server.arg("angle");
  int angle = message.toInt();

  myservo.write(angle);
  
  // send message
  server.send(200, "text/plain", message);
  Serial.println(message);
}

// SPIFFS
String getContentType(String filename);
bool handleFileRead(String path);

void setup()
{
  // Serial port
  Serial.begin(115200);
  delay(3000);
  Serial.println("");

  // Pin mode
  myservo.attach(12); // D6

  // WiFi
  // set to AP+STA
  WiFi.mode(WIFI_AP_STA);
  // host soft AP
  WiFi.softAP(APssid, APpassword);
  IPAddress myIP = WiFi.softAPIP(); // GET IP address
  Serial.print("HotSpt IP: ");
  Serial.println(myIP);

  // SPIFFS
  SPIFFS.begin();

  // routing
  server.on("/network", HTTP_GET, handleCheckNetwork);
  server.on("/network", HTTP_POST, handleConnectNetwork);
  server.on("/scan_network", HTTP_GET, handleScanNetwork);
  server.on("/servo", HTTP_GET, handleServo);
  server.onNotFound([]() {
    // find file and send to client
    if (!handleFileRead(server.uri()))
    {
      // file not found
      server.send(404, "text/plain", "404: Not Found");
    }
  });

  server.begin(); // Start server
  Serial.println("HTTP server started");

  // wait for wifi state
  Serial.println(WiFi.status());
}

void loop()
{
  // Handle client requests
  server.handleClient();

  // serial read
  // any other sensors
}

void connectToNetwork(String nk_ssid, String nk_pass)
{
  // disconnect from previous network
  WiFi.disconnect();

  // AP settings
  // check that AP still alive
  // FIXME: might not need to do this
  IPAddress myIP = WiFi.softAPIP(); // Get IP address
  Serial.print("HotSpt IP: ");
  Serial.println(myIP);

  // STA settings
  WiFi.begin(nk_ssid, nk_pass);

  Serial.print("Connecting");
  int timer = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    timer++;
    if (timer > 20)
    {
      // timeout
      Serial.println("timeout");
      return;
    }
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  if (filename.endsWith(".css"))
    return "text/css";
  if (filename.endsWith(".js"))
    return "application/javascript";
  if (filename.endsWith(".ico"))
    return "text/x-icon";
  return "text/plain";
}

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);

  // index
  if (path.endsWith("/"))
  {
    path += "index.html";
  }

  // Get the MIME type
  String contentType = getContentType(path);
  if (SPIFFS.exists(path))
  {
    File file = SPIFFS.open(path, "r");

    // send file
    size_t sent = server.streamFile(file, contentType);

    file.close();
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;
}
