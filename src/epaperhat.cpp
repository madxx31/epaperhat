#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "epd.h"

// DNSServer dnsServer;
AsyncWebServer server(80);

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        //request->addInterestingHeader("ANY");
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        // response->print(responseHTML);
        request->send(response);
    }
};

void setup()
{
    Serial.begin(115200);
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.print("Setting AP ( Point)…");
    //your other setup stuff...
    WiFi.softAP("Dasha's hat");
    // dnsServer.start(53, "*", WiFi.softAPIP());
    // server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Get received"); // Just for debug
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });

    // Example of function that holds and HTTP_POST request on 192.168.4.1:80/set_data
    server.on(
        "/set",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
           size_t index, size_t total) {
            Serial.println("POST RECEIVED"); // Just for debug
            for (int i = 0; i < len; ++i)
            {
                Serial.print((char)data[i]); // typecast because String takes uint8_t as something else than char
            }
            request->send(200, "text/plain", "Some message");
            File file2 = SPIFFS.open("/A.bin");
            for (size_t i = 0; i < 128 * 296 / 8; i++)
            {
                if (i % 16 < 10 && i < 16 * 41)
                    EPD_SendData((byte)file2.read());
                else
                    EPD_SendData((byte)255);
            }
            file2.close();
            EPD_2IN9D_Show();
        });
    server.begin();
    EPD_initSPI();
    EPD_dispInit();
}

void loop()
{
    // dnsServer.processNextRequest();
}
