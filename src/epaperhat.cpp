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

void calc_symbol_on_threshold(uint8_t *text, size_t len, int threshold, int &idx, bool &is_spacing, float &overflow)
{
    int temp_width = 0;
    is_spacing = false;
    for (; idx < len; ++idx)
    {
        is_spacing = false;
        temp_width += symbol_widths[text[idx]];
        if (temp_width > threshold)
            break;
        is_spacing = true;
        temp_width += spacing;
        if (temp_width > threshold)
            break;
    }
    overflow = temp_width - threshold;
}

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
            int total_width = get_width(data, len);
            Serial.println(total_width);
            if (total_width <= 296)
                disp2.display_text(data, len, CENTER);
            else
            {
                int split1 = 0;
                bool is_spacing = false;
                float overflow = 0;
                calc_symbol_on_threshold(data, len, (total_width - 296) / 2, split1, is_spacing, overflow);
                if (is_spacing || overflow / symbol_widths[data[split1]] < 0.5)
                    split1 += 1; //move split1 symbol on disp1 and split by next symbol
                disp1.display_text(data, split1, RIGHT);
                int split2 = 0;
                calc_symbol_on_threshold(&data[split1], len - split1, 296, split2, is_spacing, overflow);
                disp2.display_text(&data[split1], split2, WIDTH);
                disp3.display_text(&data[split1 + split2], len - split1 - split2, LEFT);
            }

            request->send(200, "text/plain", "OK");
        });
    server.begin();
}

void loop()
{
    // dnsServer.processNextRequest();
}
