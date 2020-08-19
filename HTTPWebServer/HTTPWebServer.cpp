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
// SFML
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

using json = nlohmann::json;

using namespace httplib;

const int defaultPort = 3000;
const std::string licenseName = "MosPolytech2020.2020011520201231.[cnv][mdl][slv][vsn][bsh]";
const std::string licenseKey = "AW3FSVT2UOVSsLN1/jaPC0BCknN7KQyQXZ8jl1ogNpmlIbCQThcgQ1QNwvzKI+Aqk1oYjVsuQQudrcsHKJ9ERw==";

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "RU");

    if (!CoreInit(licenseName, licenseKey)) {
        std::cerr << "ОШИБКА: Не удалось активировать геометрическое ядро конвертера" << std::endl;
        return -1;
    }

    Server svr;
    if (!svr.is_valid()) {
        std::cerr << "ОШИБКА: Не удалось создать сервер" << std::endl;
        return -1;
    }

    svr.Post("/model", [&](const auto& req, auto& res) {
        if (!req.has_file("file")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Недостаточно аргументов";
            errorResponseBody["detail"] = { { "file", "Необходимо приложить файл модели" } };
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        } else if (!req.has_file("from")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Недостаточно аргументов";
            errorResponseBody["detail"] = { { "from", "Необходимо указать входящий формат" } };
            res.status = 400;
            res.set_content(errorResponseBody.dump(), "application/json");
            return;
        } else if (!req.has_file("to")) {
            json errorResponseBody;
            errorResponseBody["error"] = "Недостаточно аргументов";
            errorResponseBody["detail"] = { { "to", "Необходимо указать исходящий формат" } };
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

        std::ofstream os;
        os.open("preview.png", std::ofstream::binary);
        if (!os.is_open()) return;
        os.write(thumbnailBuffer, thumbnailLen);
        os.close(); 

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

    std::cout << "Конвертер и средство предварительной визуализации успешно запущены и доступны по адресу: http:://127.0.0.1:" << port << std::endl;

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "ОШИБКА: Не удалось запустить сервер" << std::endl;
        return -1;
    }

    return 0;
}