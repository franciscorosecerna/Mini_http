#include <iostream>
#include "../include/core/App.h"
#include "../include/http/HttpMethod.h"
#include "../include/http/HttpStatus.h"

int main() {
    try {
        App app(4);

        app.use([](Request& req, Response& res, Next next) {
            std::cout << "[Middleware] " << req.path << std::endl;
            next();
        });

        app.get("/hello", [](Request& req, Response& res) {
            res.setStatus(HttpStatus::OK);
            res.send("Hello World!");
        });

        app.get("/user/:id", [](Request& req, Response& res) {
        std::string id = req.params["id"];
        std::string jsonBody = "{ \"id\": \"" + id + "\", \"name\": \"John Cena\" }";
        res.setStatus(HttpStatus::OK);
        res.json(jsonBody);
        });

        app.get("/old", [](Request& req, Response& res) {
            res.redirect("/new");
        });

        app.post("/echo", [](Request& req, Response& res) {
            res.setStatus(HttpStatus::OK);
            res.send("Echo POST received!");
        });

        std::cout << "Starting server on port 8080..." << std::endl;
        app.listen(8080);

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}