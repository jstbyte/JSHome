#pragma once
#include <FS.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <ESPAsyncWebServer.h>

#define INDEX_HTML "/index.html.gz"

String getContentType(String path);
void handleSvg(AsyncWebServer *server);
void notFoundHandler(AsyncWebServerRequest *req);
void registerLittleFServer(AsyncWebServer *server);
void handleStaticAssets(AsyncWebServerRequest *req);
void handleFsFile(AsyncWebServerRequest *req, String path, String contentType, bool gzip = true);