const BASE = "http://localhost:8080";

async function runTest(name, fn) {
  try {
    await fn();
    console.log("✔", name);
  } catch (err) {
    console.error("✖", name, "-", err.message);
  }
}

async function testGetAllUsers() {
  const res = await fetch(`${BASE}/users`);

  if (res.status !== 200) throw new Error("Expected 200");

  const json = await res.json();

  if (!Array.isArray(json)) throw new Error("Expected array");
  if (json.length === 0) throw new Error("Expected at least one user");
  if (!json[0].id || !json[0].name || !json[0].email)
    throw new Error("User missing fields");
}

async function testGetUserById() {
  const res = await fetch(`${BASE}/users/1`);

  if (res.status !== 200) throw new Error("Expected 200");

  const json = await res.json();

  if (json.id !== 1) throw new Error("Wrong user ID");
  if (!json.name || !json.email) throw new Error("User missing fields");

  const contentType = res.headers.get("content-type");
  if (!contentType || !contentType.includes("application/json"))
    throw new Error("Missing or wrong Content-Type");
}

async function testGetUserNotFound() {
  const res = await fetch(`${BASE}/users/9999`);

  if (res.status !== 404) throw new Error("Expected 404 for unknown user");

  const json = await res.json();
  if (!json.error) throw new Error("Expected error field in body");
}

async function testCreateUser() {
  const res = await fetch(`${BASE}/users`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name: "Test User", email: "test@mail.com" })
  });

  if (res.status !== 201) throw new Error("Expected 201 Created");

  const json = await res.json();
  if (!json.id || !json.name) throw new Error("Created user missing fields");
}

async function testDeleteUser() {
  const created = await fetch(`${BASE}/users`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name: "ToDelete", email: "del@mail.com" })
  });
  const user = await created.json();

  const res = await fetch(`${BASE}/users/${user.id}`, {
    method: "DELETE"
  });

  if (res.status !== 204) throw new Error("Expected 204 No Content");
}

async function testDeleteUserNotFound() {
  const res = await fetch(`${BASE}/users/9999`, { method: "DELETE" });

  if (res.status !== 404) throw new Error("Expected 404 for unknown user");
}

async function testGetAllProducts() {
  const res = await fetch(`${BASE}/products`);

  if (res.status !== 200) throw new Error("Expected 200");

  const json = await res.json();

  if (!Array.isArray(json)) throw new Error("Expected array");
  if (json.length === 0) throw new Error("Expected at least one product");
  if (!json[0].id || !json[0].name) throw new Error("Product missing fields");
}

async function testGetProductVariant() {
  const res = await fetch(`${BASE}/products/1/M`);

  if (res.status !== 200) throw new Error("Expected 200");

  const json = await res.json();

  if (json.id !== 1) throw new Error("Wrong product ID");
  if (json.variant !== "M") throw new Error("Wrong variant param");

  const contentType = res.headers.get("content-type");
  if (!contentType || !contentType.includes("application/json"))
    throw new Error("Missing or wrong Content-Type");
}

async function testGetProductVariantNotFound() {
  const res = await fetch(`${BASE}/products/1/XXXL`);

  if (res.status !== 404) throw new Error("Expected 404 for unknown variant");

  const json = await res.json();
  if (!json.error) throw new Error("Expected error field in body");
}

async function testGetProductNotFound() {
  const res = await fetch(`${BASE}/products/9999/M`);

  if (res.status !== 404) throw new Error("Expected 404 for unknown product");
}

async function test404() {
  const res = await fetch(`${BASE}/unknown`);

  if (res.status !== 404) throw new Error("Expected 404");
}

async function testWrongMethod() {
  const res = await fetch(`${BASE}/users/1`, { method: "POST" });

  if (![404, 405].includes(res.status))
    throw new Error("Expected 404 or 405");
}

async function testKeepAlive() {
  const res1 = await fetch(`${BASE}/users`);
  if (res1.status !== 200) throw new Error("First request failed");

  const res2 = await fetch(`${BASE}/users`);
  if (res2.status !== 200) throw new Error("Second request failed");
}

async function testConcurrency() {
  const requests = Array.from({ length: 50 }).map((_, i) =>
    fetch(`${BASE}/users`).then(r => {
      if (!r.ok) throw new Error(`Request ${i} failed with status ${r.status}`);
      return r.json();
    })
  );

  const results = await Promise.all(requests);

  if (results.length !== 50) throw new Error("Concurrency failed");
  if (!Array.isArray(results[0])) throw new Error("Unexpected response shape");
}

async function testRapidFire() {
  for (let i = 0; i < 100; i++) {
    const res = await fetch(`${BASE}/products`);
    if (!res.ok) throw new Error(`Rapid fire failed at request ${i}`);
  }
}

async function testConnectionClose() {
  const res = await fetch(`${BASE}/users`, {
    headers: { "Connection": "close" }
  });

  if (res.status !== 200) throw new Error("Connection close request failed");
}

async function testCustomHeaders() {
  const res = await fetch(`${BASE}/users`, {
    headers: { "X-Test-Header": "MiniHttp" }
  });

  if (res.status !== 200) throw new Error("Expected 200");
}

async function runAll() {
  console.log("Running Mini_http tests...\n");

  console.log("── Users ──────────────────────────────");
  await runTest("GET  /users", testGetAllUsers);
  await runTest("GET  /users/:id", testGetUserById);
  await runTest("GET  /users/:id (404)", testGetUserNotFound);
  await runTest("POST /users", testCreateUser);
  await runTest("DEL  /users/:id", testDeleteUser);
  await runTest("DEL  /users/:id (404)", testDeleteUserNotFound);

  console.log("\n── Products ───────────────────────────");
  await runTest("GET  /products", testGetAllProducts);
  await runTest("GET  /products/:id/:variant", testGetProductVariant);
  await runTest("GET  /products/:id/:variant (404)", testGetProductVariantNotFound);
  await runTest("GET  /products/:id (not found)", testGetProductNotFound);

  console.log("\n── General ────────────────────────────");
  await runTest("404 route", test404);
  await runTest("Wrong method", testWrongMethod);
  await runTest("Keep-Alive", testKeepAlive);
  await runTest("Concurrency (50 reqs)", testConcurrency);
  await runTest("Rapid fire (100 reqs)", testRapidFire);
  await runTest("Connection: close", testConnectionClose);
  await runTest("Custom headers", testCustomHeaders);

  console.log("\nTests finished.");
}

runAll();

/*
$sources = Get-ChildItem -Recurse -Path src -Filter *.cpp | ForEach-Object { $_.FullName }
g++ -std=c++17 -Iinclude -Iinclude/core -Iinclude/http -Iinclude/net C:\Users\Usuario\Desktop\mini_http\test\main.cpp $sources -o myserver.exe -lws2_32
*/