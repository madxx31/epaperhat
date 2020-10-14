#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "display.h"

DNSServer dnsServer;
AsyncWebServer server(80);
Display disp1(25, 33, 26, 13, 14, 27);
Display disp2(16, 17, 32, 4, 2, 15);
Display disp3(22, 23, 5, 21, 19, 18);

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

void render_text(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

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

const String symbols[] = {
    String(" "), String("!"), String("\""), String("#"), String("$"), String("%"), String("&"), String("'"), String("("),
    String(")"), String("*"), String("+"), String(","), String("-"), String("."), String("/"), String("0"), String("1"),
    String("2"), String("3"), String("4"), String("5"), String("6"), String("7"), String("8"), String("9"), String(":"),
    String(";"), String("<"), String("="), String(">"), String("?"), String("@"), String("A"), String("B"), String("C"),
    String("D"), String("E"), String("F"), String("G"), String("H"), String("I"), String("J"), String("K"), String("L"),
    String("M"), String("N"), String("O"), String("P"), String("Q"), String("R"), String("S"), String("T"), String("U"),
    String("V"), String("W"), String("X"), String("Y"), String("Z"), String("["), String("\\"), String("]"), String("^"),
    String("_"), String("`"), String("{"), String("|"), String("}"), String("~"), String("Ё"), String("А"), String("Б"),
    String("В"), String("Г"), String("Д"), String("Е"), String("Ж"), String("З"), String("И"), String("Й"), String("К"),
    String("Л"), String("М"), String("Н"), String("О"), String("П"), String("Р"), String("С"), String("Т"), String("У"),
    String("Ф"), String("Х"), String("Ц"), String("Ч"), String("Ш"), String("Щ"), String("Ъ"), String("Ы"), String("Ь"),
    String("Э"), String("Ю"), String("Я"), String("№")};

void update_html(uint8_t *data, size_t len)
{
    String entry = "";
    for (size_t i = 0; i < len; i++)
    {
        entry += symbols[data[i]];
    }
    Serial.println(entry);
    File f = SPIFFS.open("/index.html");
    String html = f.readString();
    f.close();
    html.replace(String("<p class=\"history-item\">") + entry + String("</p>"), "");
    html.replace(String("<div id=\"history\">"), String("<div id=\"history\"><p class=\"history-item\">") + entry + String("</p>"));
    f = SPIFFS.open("/index.html", "w");
    f.print(html);
    f.close();
}

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

void render_text(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    int total_width = get_width(data, len);
    if (total_width <= 296)
    {
        disp1.display_clear();
        disp2.display_text(data, len, CENTER);
        disp3.display_clear();
        update_html(data, len);
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
        update_html(data, len);
    }
    request->send(200, "text/plain", "OK");
}