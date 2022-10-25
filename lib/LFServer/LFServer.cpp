#include "LFServer.h"

String getContentType(String path)
{
    if (path.endsWith(".js"))
    {
        return "application/javascript";
    }
    else if (path.endsWith(".css"))
    {
        return "text/css";
    }
    else if (path.endsWith(".html"))
    {
        return "text/html";
    }
    else if (path.endsWith(".png"))
    {
        return "image/png";
    }
    else if (path.endsWith(".svg"))
    {
        return "image/svg+xml";
    }
    else
    {
        return "";
    }
}

void notFoundHandler(AsyncWebServerRequest *req)
{
    req->send(404, "text/plain", "Not found!");
}

void handleSvg(AsyncWebServer *server)
{
}

void handleFsFile(AsyncWebServerRequest *req, String path, String contentType, bool gzip = true)
{
    if (LittleFS.exists(path))
    {
        auto *response = req->beginResponse(LittleFS, path, contentType);
        if (gzip)
        {
            response->addHeader("Content-Encoding", "gzip");
        }
        req->send(response);
        return;
    }
    notFoundHandler(req);
};

void handleStaticAssets(AsyncWebServerRequest *req)
{
    /* REG_EX = "^\\/assets\\/(.*)$" */
    String path = "/assets/" + req->pathArg(0);
    String contentType = getContentType(path);
    bool gzip = !path.endsWith(".svg"); // Only For (SVG)

    if (gzip) // Add GZ Compression Extention;
        path += ".gz";

    if (LittleFS.exists(path))
    {
        auto *response = req->beginResponse(LittleFS, path, contentType);
        if (gzip)
            response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "max-age=86400");
        req->send(response);
        return;
    }
    notFoundHandler(req);
};

void registerLittleFServer(AsyncWebServer *server)
{

    // Handle "/" with index.html.gz
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *req)
               { handleFsFile(req, "/index.html.gz", "text/html"); });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *req)
               { handleFsFile(req, "/manifest.json.gz", "application/manifest+json"); });

    // Handle static assets like: css/js/images/etc.
    server->on("^\\/assets\\/(.*)$", HTTP_GET, handleStaticAssets);
}