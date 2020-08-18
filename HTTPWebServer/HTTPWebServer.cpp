// HTTPWebServer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <sstream>
#include <httplib.h>
#include <Windows.h>
#include <Translate.h>
#include <CoreInit.h>
#include <json.hpp>
#include "../cpp-base64/base64.h"

using json = nlohmann::json;

using namespace httplib;

const int defaultPort = 3000;
const std::string licenseName = "MosPolytech2020.2020011520201231.[cnv][mdl][slv][vsn][bsh]";
const std::string licenseKey = "AW3FSVT2UOVSsLN1/jaPC0BCknN7KQyQXZ8jl1ogNpmlIbCQThcgQ1QNwvzKI+Aqk1oYjVsuQQudrcsHKJ9ERw==";

int main(int argc, char* argv[])
{
    if (!CoreInit(licenseName, licenseKey)) return -1;
    Server svr;

    svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
        return;
    });

    svr.Post("/model", [&](const auto& req, auto& res) {
        if (!req.has_file("file")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Invalid payload.";
            errorResponseBody["detail"] = { { "file", "This field is required." } };
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        } else if (!req.has_file("from")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Invalid payload.";
            errorResponseBody["detail"] = { { "from", "This field is required." } };
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        } else if (!req.has_file("to")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Invalid payload.";
            errorResponseBody["detail"] = { { "to", "This field is required." } };
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        }

        const auto& file = req.get_file_value("file");
        const auto& inFileExt = req.get_file_value("from");
        const auto& outFileExt = req.get_file_value("to");

        char* outFileBuffer = 0;
        size_t outFileLen = 0;
        char* thumbnailBuffer = 0;
        size_t thumbnailLen = 0;
        char* inFileBuffer = (char*)file.content.c_str();
        size_t inFileLen = strlen(file.content.c_str());

        std::string errorMessage;
        if (!mpu::Translate(inFileBuffer, inFileLen, inFileExt.content,
            outFileBuffer, outFileLen, outFileExt.content,
            thumbnailBuffer, thumbnailLen,
            errorMessage)) {
            json errorResponseBody;
            errorResponseBody["error"] = errorMessage;
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        }

        json responseBody;
        responseBody["output"] = base64_encode((const unsigned char*)outFileBuffer, outFileLen);
        responseBody["thumbnail"] = base64_encode((const unsigned char*)thumbnailBuffer, thumbnailLen);

       free(outFileBuffer);
       free(thumbnailBuffer);
        res.set_content(responseBody.dump(), "application/json");
        return;
    });

    int port = defaultPort;
    if (argv[1]) {
        int userPort = atoi(argv[1]);
        if (userPort > 0 || userPort < pow(2, 16)) port = userPort;
    }

    return svr.listen("0.0.0.0", port);
}