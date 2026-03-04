#include <mini_http/mini_http.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <nlohmann/json.hpp>

using namespace mini_http;
using json = nlohmann::json;

struct User {
    int id;
    std::string name;
    std::string email;

    json toJson() const {
        return {{"id", id}, {"name", name}, {"email", email}};
    }
};

struct Product {
    int id;
    std::string name;
    std::vector<std::string> variants;

    json toJson(const std::string& variant = "") const {
        json j = {{"id", id}, {"name", name}};
        if (!variant.empty()) j["variant"] = variant;
        return j;
    }
};

std::mutex usersMutex;

std::vector<User> usersDB = {
    {1, "Alice", "alice@mail.com"},
    {2, "Bob", "bob@mail.com"},
    {3, "Charlie", "charlie@mail.com"},
};

std::vector<Product> productsDB = {
    {1, "Remera", {"S", "M", "L", "XL"}},
    {2, "Zapatilla", {"38", "39", "40", "41"}},
};

int parseId(const std::string& s) {
    if (s.empty()) return -1;
    for (char c : s)
        if (c < '0' || c > '9') return -1;
    try {
        long long v = std::stoll(s);
        if (v <= 0 || v > INT32_MAX) return -1;
        return static_cast<int>(v);
    } catch (...) {
        return -1;
    }
}

void getAllUsers(Request& req, Response& res) {
    std::cout << "[DB] SELECT * FROM users\n";

    std::lock_guard<std::mutex> lock(usersMutex);

    json body = json::array();
    for (const auto& u : usersDB) body.push_back(u.toJson());

    res.ok(body);
}

void getUser(Request& req, Response& res) {
    int id = parseId(req.params["id"]);
    if (id == -1) { res.badRequest("Invalid user ID"); return; }

    std::cout << "[DB] SELECT * FROM users WHERE id = " << id << "\n";

    std::lock_guard<std::mutex> lock(usersMutex);

    auto it = std::find_if(usersDB.begin(), usersDB.end(),
        [id](const User& u) { return u.id == id; });

    if (it == usersDB.end()) { res.notFound("User not found"); return; }

    res.ok(it->toJson());
}

void createUser(Request& req, Response& res) {
    std::lock_guard<std::mutex> lock(usersMutex);

    int newId = usersDB.back().id + 1;
    User newUser = {newId, "NewUser" + std::to_string(newId), "new@mail.com"};

    std::cout << "[DB] INSERT INTO users VALUES (" << newId << ", ...)\n";
    usersDB.push_back(newUser);

    res.created(newUser.toJson());
}

void deleteUser(Request& req, Response& res) {
    int id = parseId(req.params["id"]);
    if (id == -1) { res.badRequest("Invalid user ID"); return; }

    std::cout << "[DB] DELETE FROM users WHERE id = " << id << "\n";

    std::lock_guard<std::mutex> lock(usersMutex);

    auto it = std::find_if(usersDB.begin(), usersDB.end(),
        [id](const User& u) { return u.id == id; });

    if (it == usersDB.end()) { res.notFound("User not found"); return; }

    usersDB.erase(it);
    res.noContent();
}

void getProducts(Request& req, Response& res) {
    std::cout << "[DB] SELECT * FROM products\n";

    json body = json::array();
    for (const auto& p : productsDB) body.push_back(p.toJson());

    res.ok(body);
}

void getProductVariant(Request& req, Response& res) {
    int id = parseId(req.params["id"]);
    if (id == -1) { res.badRequest("Invalid product ID"); return; }

    std::string variant = req.params["variant"];

    std::cout << "[DB] SELECT * FROM products WHERE id = " << id
              << " AND variant = '" << variant << "'\n";

    auto it = std::find_if(productsDB.begin(), productsDB.end(),
        [id](const Product& p) { return p.id == id; });

    if (it == productsDB.end()) { res.notFound("Product not found"); return; }

    bool variantExists = std::find(it->variants.begin(), it->variants.end(), variant)
                         != it->variants.end();

    if (!variantExists) { res.notFound("Variant not found"); return; }

    res.ok(it->toJson(variant));
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