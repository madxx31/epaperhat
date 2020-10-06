#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "display.h"

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

Display disp1(25, 33, 26, 13, 14, 27);
Display disp2(16, 17, 32, 4, 2, 15);
Display disp3(22, 23, 5, 21, 19, 18);

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
    server.on(
        "/set",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
           size_t index, size_t total) {
            Serial.println("POST RECEIVED"); // Just for debug
            disp2.EPD_Init_2in9d();
            int spacing = 5;
            int total_width = 0;
            for (int i = 0; i < len; ++i)
            {
                total_width += symbol_widths[data[i]];
                Serial.print(String(data[i])); // typecast because String takes uint8_t as something else than char

            }
            total_width += (spacing * (len - 1));
            if (total_width <= 296)
            {
                disp2.fill(ceil((296 - total_width) / 2));
                for (int i = 0; i < len; ++i)
                {
                    disp2.display_symbol(data[len - i - 1]);
                    disp2.fill(spacing);
                }
                disp2.fill(floor((296 - total_width) / 2));
            }
            Serial.print("Total width ");
            Serial.print(total_width);
            request->send(200, "text/plain", "Some message");
            // File file1 = SPIFFS.open("/font/&.bin");
            // File file2 = SPIFFS.open("/font/Ы.bin");
            // File file3 = SPIFFS.open("/font/\\.bin");
            // for (size_t i = 0; i < 128 * 296 / 8; i++)
            // {
            //     if (i < 128 / 8 * get_symbol_width("&"))
            //     {
            //         byte d = file1.read();
            //         disp1.EPD_SendData(d);
            //         disp2.EPD_SendData(d);
            //         disp3.EPD_SendData(d);
            //     }
            //     else if (i < 128 / 8 * get_symbol_width("&") + 128 / 8 * get_symbol_width("Ы"))
            //     {
            //         byte d = file2.read();
            //         disp1.EPD_SendData(d);
            //         disp2.EPD_SendData(d);
            //         disp3.EPD_SendData(d);
            //     }
            //     else if (i < 128 / 8 * get_symbol_width("&") + 128 / 8 * get_symbol_width("Ы") + 128 / 8 * get_symbol_width("\\"))
            //     {
            //         byte d = file3.read();
            //         disp1.EPD_SendData(d);
            //         disp2.EPD_SendData(d);
            //         disp3.EPD_SendData(d);
            //     }
            //     else
            //     {
            //         disp1.EPD_SendData((byte)0);
            //         disp2.EPD_SendData((byte)0);
            //         disp3.EPD_SendData((byte)0);
            //     }
            // }
            // file1.close();
            // file2.close();
            // file3.close();
            // disp1.EPD_2IN9D_Show();
            disp2.EPD_2IN9D_Show();
            // disp3.EPD_2IN9D_Show();
        });
    server.begin();
    // EPD_initSPI();
    // EPD_dispInit();
}

void loop()
{
    // dnsServer.processNextRequest();
}
