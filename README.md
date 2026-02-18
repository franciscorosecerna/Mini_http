A lightweight HTTP server framework written in C++17, inspired by Express.js. *(well thats the idea)*
## Usage Example

```cpp
#include "core/App.h"

int main() {
    App app(4); // 4 worker threads

    app.get("/hello", [](Request& req, Response& res) {
        res.setStatus(HttpStatus::OK);
        res.send("Hello World!");
    });

    app.listen(8080);
}
```