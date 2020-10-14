#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "display.h"

void render_text(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        return (request->url() != "/style.css" && request->url() != "/script.js" && request->url() != "/set");
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/index.html", "text/html");
    }
};

DNSServer dnsServer;
AsyncWebServer server(80);
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
    SPIFFS.begin(true);
    WiFi.softAP("Dasha's hat");
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });
    server.on(
        "/set", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, render_text);
    server.begin();
}

void loop()
{
    dnsServer.processNextRequest();
}

void render_text(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    int total_width = get_width(data, len);
    if (total_width <= 296)
    {
        disp1.clear();
        disp2.display_text(data, len, CENTER);
        disp3.clear();
    }
    else if (total_width <= 296 * 3)
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
}