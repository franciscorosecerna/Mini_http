A lightweight HTTP server framework written in C++17, inspired by Express.js.

## Usage Example

```cpp
int main() {
    Router users;
    users.get("/", getAllUsers);
    users.get("/:id", getUser);
    users.post("/", createUser);
    users.del("/:id", deleteUser);

    Router products;
    products.get("/", getProducts);
    products.get("/:id/:variant", getProductVariant);

    App app(4); // 4 worker threads

    app.use([](Request& req, Response& res, Next next) {
        std::cout << "[" << req.path << "]\n";
        next();
    });

    app.use("/users", users);
    app.use("/products", products);

    std::cout << "Server running on http://localhost:8080\n";
    app.listen(8080);

    return 0;
}
```