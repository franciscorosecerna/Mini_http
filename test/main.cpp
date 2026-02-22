#include <mini_http/mini_http.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace mini_http;

struct User {
    int id;
    std::string name;
    std::string email;
};

struct Product {
    int id;
    std::string name;
    std::vector<std::string> variants;
};

std::vector<User> usersDB = {
    {1, "Alice",   "alice@mail.com"},
    {2, "Bob",     "bob@mail.com"},
    {3, "Charlie", "charlie@mail.com"},
};

std::vector<Product> productsDB = {
    {1, "Remera",   {"S", "M", "L", "XL"}},
    {2, "Zapatilla",{"38", "39", "40", "41"}},
};

std::string userToJson(const User& u) {
    return R"({"id":)" + std::to_string(u.id) +
           R"(,"name":")" + u.name +
           R"(","email":")" + u.email + R"("})";
}

std::string productToJson(const Product& p, const std::string& variant = "") {
    std::string json = R"({"id":)" + std::to_string(p.id) +
                       R"(,"name":")" + p.name + R"(")";
    if (!variant.empty()) {
        json += R"(,"variant":")" + variant + R"(")";
    }
    json += "}";
    return json;
}

void getAllUsers(Request& req, Response& res) {
    std::cout << "[DB] SELECT * FROM users\n";

    std::string body = "[";
    for (size_t i = 0; i < usersDB.size(); ++i) {
        body += userToJson(usersDB[i]);
        if (i + 1 < usersDB.size()) body += ",";
    }
    body += "]";

    res.setHeader("Content-Type", "application/json");
    res.send(body);
}

void getUser(Request& req, Response& res) {
    int id = std::stoi(req.params["id"]);
    std::cout << "[DB] SELECT * FROM users WHERE id = " << id << "\n";

    auto it = std::find_if(usersDB.begin(), usersDB.end(),
        [id](const User& u) { return u.id == id; });

    if (it == usersDB.end()) {
        res.setStatus(HttpStatus::NOT_FOUND);
        res.send(R"({"error":"User not found"})");
        return;
    }

    res.setHeader("Content-Type", "application/json");
    res.send(userToJson(*it));
}

void createUser(Request& req, Response& res) {
    int newId = usersDB.back().id + 1;
    User newUser = {newId, "NewUser" + std::to_string(newId), "new@mail.com"};

    std::cout << "[DB] INSERT INTO users VALUES (" << newId << ", ...)\n";
    usersDB.push_back(newUser);

    res.setStatus(HttpStatus::CREATED);
    res.setHeader("Content-Type", "application/json");
    res.send(userToJson(newUser));
}

void deleteUser(Request& req, Response& res) {
    int id = std::stoi(req.params["id"]);
    std::cout << "[DB] DELETE FROM users WHERE id = " << id << "\n";

    auto it = std::find_if(usersDB.begin(), usersDB.end(),
        [id](const User& u) { return u.id == id; });

    if (it == usersDB.end()) {
        res.setStatus(HttpStatus::NOT_FOUND);
        res.send(R"({"error":"User not found"})");
        return;
    }

    usersDB.erase(it);
    res.setStatus(HttpStatus::NO_CONTENT);
    res.send("");
}

void getProducts(Request& req, Response& res) {
    std::cout << "[DB] SELECT * FROM products\n";

    std::string body = "[";
    for (size_t i = 0; i < productsDB.size(); ++i) {
        body += productToJson(productsDB[i]);
        if (i + 1 < productsDB.size()) body += ",";
    }
    body += "]";

    res.setHeader("Content-Type", "application/json");
    res.send(body);
}

void getProductVariant(Request& req, Response& res) {
    int id = std::stoi(req.params["id"]);
    std::string variant = req.params["variant"];

    std::cout << "[DB] SELECT * FROM products WHERE id = " << id
              << " AND variant = '" << variant << "'\n";

    auto it = std::find_if(productsDB.begin(), productsDB.end(),
        [id](const Product& p) { return p.id == id; });

    if (it == productsDB.end()) {
        res.setStatus(HttpStatus::NOT_FOUND);
        res.send(R"({"error":"Product not found"})");
        return;
    }

    auto& variants = it->variants;
    bool variantExists = std::find(variants.begin(), variants.end(), variant)
                         != variants.end();

    if (!variantExists) {
        res.setStatus(HttpStatus::NOT_FOUND);
        res.send(R"({"error":"Variant not found"})");
        return;
    }

    res.setHeader("Content-Type", "application/json");
    res.send(productToJson(*it, variant));
}

int main() {
    Router users;
    users.get("/", getAllUsers);
    users.get("/:id", getUser);
    users.post("/", createUser);
    users.del("/:id", deleteUser);

    Router products;
    products.get("/", getProducts);
    products.get("/:id/:variant", getProductVariant);

    App app(4);

    app.use([](Request& req, Response& res, Next next) {
        std::cout << "[" << req.path << "]\n";
        next();
    });

    app.use("/users", users);
    app.use("/products", products);

    std::cout << "Server running on http://localhost:8080\n";
    app.start(8080);

    return 0;
}