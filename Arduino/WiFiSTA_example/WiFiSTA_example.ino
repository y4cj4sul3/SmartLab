#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"

// WiFi config
const char *ssid = "<SSID>";
const char *password = "<PASSWORD>";
IPAddress staticIP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 0);
IPAddress subnet(255, 255, 255, 0);

// server
ESP8266WebServer server(80);

void handleCheckConnection()
{
  Serial.println("Connect Successfully");

  server.send(200, "text/html", "success");
}

void handleGetArgTest()
{
  for (int i = 0; i < server.args(); ++i)
  {
    Serial.print("Arg: ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(server.arg(i));
  }

  server.send(200, "text/html", "success");
}

void handleExampleFunction()
{
  server.send(200, "text/html", "Hello World!");
}

void setup()
{
  Serial.begin(115200);
  WiFi.disconnect();

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);

  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", handleCheckConnection);
  server.on("/GetArgTest", handleGetArgTest);
  server.on("/ExampleFunction", handleExampleFunction);

  server.begin();
}

void loop()
{
  server.handleClient();
}
