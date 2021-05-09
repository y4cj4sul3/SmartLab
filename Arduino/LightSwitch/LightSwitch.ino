#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Servo.h>

// AP config
String apSSID = "SmartLab";
String apPassword = "00000000";

// mDNS
String domainName = "smartlab";

// web server
ESP8266WebServer server(80);

// servos
Servo switchServo;   // limited servo: 0~180
Servo positionServo; // continuous servo

// switch pos (idle, btn1, btn2, ...)
int switchPos[] = {0, 90, 180, 270};
// switch pos (idle, ON, OFF)
int switchAgl[] = {90, 0, 180};

void controlSwitch(int id, bool state)
{
    // move to switch
    controlPositionServo(switchPos[id + 1] - switchPos[0]);
    delay(500);

    if (state)
    {
        // turn ON
        switchServo.write(switchAgl[1]);
    }
    else
    {
        // turn OFF
        switchServo.write(switchAgl[2]);
    }
    delay(1500);
    switchServo.write(switchAgl[0]);
    delay(500);

    // move back to idle position
    controlPositionServo(switchPos[0] - switchPos[id + 1]);
}

void controlPositionServo(int move)
{
    // TODO: convert movement to angle
    int angle = move;
    Serial.println(angle);

    if (angle > 0)
    {
        // rotate CW
        positionServo.write(0);
    }
    else
    {
        // rotate CCW
        positionServo.write(180);
        angle = -angle;
    }

    // TODO: measure
    // convert angle to delay, which is not very accurate
    // angle 90 ~> 150 ms
    int delayTime = max(angle * 2, 0);
    Serial.println(delayTime);
    delay(delayTime);

    // stop
    positionServo.write(90);
}

void handleServo()
{
    String message = server.arg("angle");
    int angle = message.toInt();

    // send message
    server.send(200, "text/plain", message);
    Serial.println(message);

    switchServo.write(angle);
}

void handleSwitch()
{
    int id = server.arg("id").toInt();
    int state = server.arg("state") == "true";

    // send message
    server.send(200, "text/plain");
    Serial.print("Turn switch ");
    Serial.print(id);
    if (state)
    {
        Serial.println(" ON");
    }
    else
    {
        Serial.println(" OFF");
    }

    controlSwitch(id, state);
}

void handlePosition()
{
    String message = server.arg("pos");
    int pos = message.toInt();

    // send message
    server.send(200, "text/plain");
    Serial.println("set pos");

    controlPositionServo(pos);
}

void setup()
{
    // Serial port
    Serial.begin(115200);
    Serial.println();

    // Pin mode
    switchServo.attach(12);   // D6
    positionServo.attach(13); // D7
    positionServo.write(90);

    // WiFi setup
    setupAP(apSSID, apPassword);

    // SPIFFS
    SPIFFS.begin();

    // routing
    server.on("/network", HTTP_GET, handleCheckNetworkStatus);
    server.on("/network", HTTP_POST, handleConnectToNetwork);
    server.on("/scan_network", HTTP_GET, handleScanNetwork);
    server.on("/servo", HTTP_GET, handleServo);
    server.on("/switch", HTTP_GET, handleSwitch);
    server.on("/position", HTTP_GET, handlePosition);
    server.onNotFound([]() {
        // find file and send to client
        if (!handleFileRead(server.uri()))
        {
            // file not found
            server.send(404, "text/plain", "404: Not Found");
            // TODO: redirect to root
        }
    });

    // start server
    server.begin();
    Serial.println("HTTP server started");

    // wait for wifi status
    if (!waitForConnection())
    {
        Serial.println("disconnect");
        WiFi.disconnect();
    }
    int status = WiFi.status();
    Serial.print("WiFi status: ");
    Serial.println(status);
    // set mDNS
    if (status == WL_CONNECTED)
    {
        setMDNS(domainName);
    }
}

void loop()
{
    // keep update MDNS
    MDNS.update();
    // Handle client requests
    server.handleClient();

    // do other things
}

void setupAP(String ssid, String passwd)
{
    // set to AP+STA mode
    WiFi.mode(WIFI_AP_STA);
    // host soft AP
    WiFi.softAP(ssid, passwd);
    // show AP IP
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void connectToNetwork(String ssid, String passwd)
{
    // disconnect from previous network
    WiFi.disconnect();

    // connect to other AP
    WiFi.begin(ssid, passwd);
    if (!waitForConnection())
    {
        return;
    }

    // get local IP
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    // set MDNS
    setMDNS(domainName);
}

bool waitForConnection()
{
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
            return false;
        }
    }
    Serial.println();
    return true;
}

void setMDNS(String domainName)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        // Start the mDNS responder for domainName.local
        if (MDNS.begin(domainName, WiFi.localIP()))
        {
            Serial.println("mDNS responder started");
        }
        else
        {
            Serial.println("Error setting up MDNS responder!");
        }
    }
    // The line below cause exception
    // some comment said without it, it would only work for PC
    // however, it works fine for PC and mobile devices for me
    // MDNS.addService("http", "tcp", 80);
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
    // index
    if (path.endsWith("/"))
    {
        path += "index.html";
    }
    Serial.println("handleFileRead: " + path);

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

void handleCheckNetworkStatus()
{
    Serial.println("Checking network status");

    // check for wifi status
    String message = "";
    int status = WiFi.status();
    switch (status)
    {
    case WL_CONNECTED:
        // connected to network
        message += "C: " + WiFi.SSID() + ", " + WiFi.localIP().toString();
        break;

    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED:
        // connection failed, lost, or disconnected
        message += "F: " + status;
        break;

    default:
        // not connected yet or idle
        message += "N: " + status;
        break;
    }
    // send message
    server.send(200, "text/plain", message);
    Serial.println(message);
}

void handleConnectToNetwork()
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
    // WiFi.disconnect();

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
