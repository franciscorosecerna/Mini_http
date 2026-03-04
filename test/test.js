const BASE = "http://localhost:8080";

let passed = 0;
let failed = 0;

async function runTest(name, fn) {
  try {
    await fn();
    console.log("✔", name);
    passed++;
  } catch (err) {
    console.error("✖", name, "-", err.message);
    failed++;
  }
}

function assertStatus(res, expected) {
  if (res.status !== expected)
    throw new Error(`Expected HTTP ${expected}, got ${res.status}`);
}

function assertJsonContentType(res) {
  const ct = res.headers.get("content-type");
  if (!ct || !ct.includes("application/json"))
    throw new Error(`Expected Content-Type: application/json, got "${ct}"`);
}

function assertUserShape(u) {
  if (typeof u.id !== "number")    throw new Error(`user.id must be a number, got ${typeof u.id}`);
  if (typeof u.name !== "string")  throw new Error(`user.name must be a string`);
  if (typeof u.email !== "string") throw new Error(`user.email must be a string`);
  if (Object.keys(u).length !== 3) throw new Error(`User has unexpected extra fields: ${Object.keys(u).join(", ")}`);
}

function assertProductShape(p) {
  if (typeof p.id !== "number")   throw new Error(`product.id must be a number`);
  if (typeof p.name !== "string") throw new Error(`product.name must be a string`);
}

function assertErrorShape(json) {
  if (typeof json.error !== "string" || json.error.trim() === "")
    throw new Error(`Expected non-empty string in "error" field`);
}

async function assertJson(res) {
  try {
    return await res.json();
  } catch {
    throw new Error("Response body is not valid JSON");
  }
}

async function createUser() {
  const res = await fetch(`${BASE}/users`, { method: "POST" });
  assertStatus(res, 201);
  return assertJson(res);
}

async function testGetAllUsers() {
  const res = await fetch(`${BASE}/users`);
  assertStatus(res, 200);
  assertJsonContentType(res);

  const json = await assertJson(res);
  if (!Array.isArray(json))      throw new Error("Expected JSON array");
  if (json.length === 0)         throw new Error("Expected at least one user");

  for (const u of json) assertUserShape(u);

  const ids = json.map(u => u.id);
  if (new Set(ids).size !== ids.length) throw new Error("Duplicate user IDs in response");
}

async function testGetUserById() {
  const res = await fetch(`${BASE}/users/1`);
  assertStatus(res, 200);
  assertJsonContentType(res);

  const json = await assertJson(res);
  assertUserShape(json);
  if (json.id !== 1) throw new Error(`Expected id=1, got id=${json.id}`);
  if (json.name !== "Alice")         throw new Error(`Expected name="Alice", got "${json.name}"`);
  if (json.email !== "alice@mail.com") throw new Error(`Expected email="alice@mail.com", got "${json.email}"`);
}

async function testGetUserByIdAllSeeded() {
  const expected = [
    { id: 1, name: "Alice", email: "alice@mail.com" },
    { id: 2, name: "Bob", email: "bob@mail.com" },
    { id: 3, name: "Charlie", email: "charlie@mail.com" },
  ];
  for (const exp of expected) {
    const res = await fetch(`${BASE}/users/${exp.id}`);
    assertStatus(res, 200);
    const json = await assertJson(res);
    if (json.id !== exp.id)       throw new Error(`id mismatch for user ${exp.id}`);
    if (json.name !== exp.name)   throw new Error(`name mismatch for user ${exp.id}: got "${json.name}"`);
    if (json.email !== exp.email) throw new Error(`email mismatch for user ${exp.id}: got "${json.email}"`);
  }
}

async function testGetUserNotFound() {
  const res = await fetch(`${BASE}/users/9999`);
  assertStatus(res, 404);
  assertJsonContentType(res);

  const json = await assertJson(res);
  assertErrorShape(json);
}

async function testGetUserInvalidId() {
  const res = await fetch(`${BASE}/users/abc`);
  if (res.status === 500) throw new Error("Server crashed on non-numeric ID (got 500)");
  if (![400, 404].includes(res.status))
    throw new Error(`Expected 400 or 404 for non-numeric ID, got ${res.status}`);
}

async function testCreateUser() {
  const res = await fetch(`${BASE}/users`, { method: "POST" });
  assertStatus(res, 201);
  assertJsonContentType(res);

  const json = await assertJson(res);
  assertUserShape(json);
  if (typeof json.id !== "number" || json.id <= 0)
    throw new Error(`Expected positive numeric id, got ${json.id}`);

  const getRes = await fetch(`${BASE}/users/${json.id}`);
  assertStatus(getRes, 200);
  const fetched = await assertJson(getRes);
  if (fetched.id !== json.id)
    throw new Error(`Created user id=${json.id} not found via GET`);
}

async function testCreateUserIdsAreMonotonic() {
  const a = await createUser();
  const b = await createUser();
  if (b.id <= a.id)
    throw new Error(`Expected strictly increasing IDs, got ${a.id} then ${b.id}`);
}

async function testDeleteUser() {
  const user = await createUser();

  const delRes = await fetch(`${BASE}/users/${user.id}`, { method: "DELETE" });
  assertStatus(delRes, 204);

  const body = await delRes.text();
  if (body.trim() !== "")
    throw new Error(`Expected empty body on 204, got: "${body}"`);

  const getRes = await fetch(`${BASE}/users/${user.id}`);
  assertStatus(getRes, 404);
}

async function testDoubleDelete() {
  const user = await createUser();
  await fetch(`${BASE}/users/${user.id}`, { method: "DELETE" });

  const res = await fetch(`${BASE}/users/${user.id}`, { method: "DELETE" });
  assertStatus(res, 404);
}

async function testDeleteUserNotFound() {
  const res = await fetch(`${BASE}/users/9999`, { method: "DELETE" });
  assertStatus(res, 404);
  assertJsonContentType(res);
  const json = await assertJson(res);
  assertErrorShape(json);
}

async function testDeleteDoesNotAffectOtherUsers() {
  const user = await createUser();
  await fetch(`${BASE}/users/${user.id}`, { method: "DELETE" });

  const res = await fetch(`${BASE}/users/1`);
  assertStatus(res, 200);
  const json = await assertJson(res);
  if (json.id !== 1) throw new Error("Seeded user 1 disappeared after unrelated delete");
}

async function testGetAllProducts() {
  const res = await fetch(`${BASE}/products`);
  assertStatus(res, 200);
  assertJsonContentType(res);

  const json = await assertJson(res);
  if (!Array.isArray(json)) throw new Error("Expected JSON array");
  if (json.length === 0)    throw new Error("Expected at least one product");

  for (const p of json) assertProductShape(p);

  const ids = json.map(p => p.id);
  if (new Set(ids).size !== ids.length) throw new Error("Duplicate product IDs in response");
}

async function testGetProductVariants() {
  for (const variant of ["S", "M", "L", "XL"]) {
    const res = await fetch(`${BASE}/products/1/${variant}`);
    assertStatus(res, 200);
    assertJsonContentType(res);

    const json = await assertJson(res);
    assertProductShape(json);
    if (json.id !== 1)          throw new Error(`Expected id=1 for variant ${variant}`);
    if (json.variant !== variant) throw new Error(`Expected variant="${variant}", got "${json.variant}"`);
    if (json.name !== "Remera")  throw new Error(`Expected name="Remera", got "${json.name}"`);
  }
}

async function testGetProductVariantsProduct2() {
  for (const variant of ["38", "39", "40", "41"]) {
    const res = await fetch(`${BASE}/products/2/${variant}`);
    assertStatus(res, 200);
    const json = await assertJson(res);
    if (json.id !== 2)            throw new Error(`Expected id=2 for variant ${variant}`);
    if (json.variant !== variant)  throw new Error(`Expected variant="${variant}", got "${json.variant}"`);
  }
}

async function testGetProductVariantNotFound() {
  const res = await fetch(`${BASE}/products/1/XXXL`);
  assertStatus(res, 404);
  assertJsonContentType(res);

  const json = await assertJson(res);
  assertErrorShape(json);
}

async function testGetProductNotFound() {
  const res = await fetch(`${BASE}/products/9999/M`);
  assertStatus(res, 404);
  assertJsonContentType(res);

  const json = await assertJson(res);
  assertErrorShape(json);
}

async function testGetProductVariantResponseHasNoExtraFields() {
  const res = await fetch(`${BASE}/products/1/M`);
  assertStatus(res, 200);
  const json = await assertJson(res);

  const allowed = new Set(["id", "name", "variant"]);
  for (const key of Object.keys(json)) {
    if (!allowed.has(key))
      throw new Error(`Unexpected field "${key}" in product variant response`);
  }
}

async function test404() {
  const res = await fetch(`${BASE}/unknown`);
  assertStatus(res, 404);
}

async function testWrongMethod() {
  const res = await fetch(`${BASE}/users/1`, { method: "POST" });
  if (![404, 405].includes(res.status))
    throw new Error(`Expected 404 or 405, got ${res.status}`);
  if (res.status === 405) {
    const allow = res.headers.get("Allow");
    if (!allow) throw new Error("405 response missing Allow header");
  }
}

async function testPutNotAllowed() {
  const res = await fetch(`${BASE}/users/1`, { method: "PUT" });
  if (![404, 405].includes(res.status))
    throw new Error(`Expected 404 or 405 for PUT, got ${res.status}`);
}

async function testKeepAlive() {
  for (let i = 0; i < 5; i++) {
    const res = await fetch(`${BASE}/users`);
    assertStatus(res, 200);
    const json = await assertJson(res);
    if (!Array.isArray(json)) throw new Error(`Request ${i}: expected array`);
  }
}

async function testConcurrency() {
  const N = 50;
  const requests = Array.from({ length: N }, (_, i) =>
    fetch(`${BASE}/users`).then(async r => {
      if (!r.ok) throw new Error(`Request ${i} failed with status ${r.status}`);
      const json = await r.json();
      if (!Array.isArray(json)) throw new Error(`Request ${i}: body is not an array`);
      return json;
    })
  );

  const results = await Promise.all(requests);
  if (results.length !== N) throw new Error(`Expected ${N} results, got ${results.length}`);

  const baseline = JSON.stringify(results[0]);
  for (let i = 1; i < results.length; i++) {
    if (JSON.stringify(results[i]) !== baseline)
      throw new Error(`Response ${i} differs from baseline (possible data race)`);
  }
}

async function testConcurrentMixedEndpoints() {
  const requests = [
    ...Array.from({ length: 20 }, () => fetch(`${BASE}/users`)),
    ...Array.from({ length: 20 }, () => fetch(`${BASE}/products`)),
    ...Array.from({ length: 10 }, () => fetch(`${BASE}/users/1`)),
  ];

  const results = await Promise.all(requests);
  for (const r of results) {
    if (!r.ok) throw new Error(`Mixed concurrent request failed: ${r.status}`);
  }
}

async function testRapidFireDoesNotLeak() {
  for (let i = 0; i < 100; i++) {
    const res = await fetch(`${BASE}/products`);
    if (!res.ok) throw new Error(`Rapid fire failed at request ${i} with status ${res.status}`);
    const json = await res.json();
    if (!Array.isArray(json) || json.length === 0)
      throw new Error(`Rapid fire: unexpected body at request ${i}`);
  }
}

async function testConnectionClose() {
  const res = await fetch(`${BASE}/users`, {
    headers: { "Connection": "close" }
  });
  assertStatus(res, 200);
  assertJsonContentType(res);
  const json = await assertJson(res);
  if (!Array.isArray(json)) throw new Error("Expected array after Connection: close");
}

async function testCustomRequestHeaders() {
  const res = await fetch(`${BASE}/users`, {
    headers: {
      "X-Test-Header": "MiniHttp",
      "X-Another":     "value123",
    }
  });
  assertStatus(res, 200);
}

async function testLargeIdDoesNotCrash() {
  const res = await fetch(`${BASE}/users/999999999999`);
  if (res.status === 500) throw new Error("Server crashed on large numeric ID");
  if (![400, 404].includes(res.status))
    throw new Error(`Expected 400 or 404 for oversized ID, got ${res.status}`);
}

async function runAll() {
  console.log("Running Mini_http tests...\n");

  console.log("── Users ──────────────────────────────────────────────────");
  await runTest("GET /users→ array, correct shape, no dup IDs", testGetAllUsers);
  await runTest("GET /users/1 → exact seeded data", testGetUserById);
  await runTest("GET /users/:id → all 3 seeded users match", testGetUserByIdAllSeeded);
  await runTest("GET /users/9999 → 404 + error body", testGetUserNotFound);
  await runTest("GET /users/abc → no crash (400 or 404)", testGetUserInvalidId);
  await runTest("GET /users/999999999999 → no crash (400 or 404)", testLargeIdDoesNotCrash);
  await runTest("POST /users → 201, valid shape, retrievable", testCreateUser);
  await runTest("POST /users x2 → IDs are strictly increasing", testCreateUserIdsAreMonotonic);
  await runTest("DEL /users/:id → 204, empty body, then 404", testDeleteUser);
  await runTest("DEL /users/:id x2 → second delete returns 404", testDoubleDelete);
  await runTest("DEL /users/9999 → 404 + error body", testDeleteUserNotFound);
  await runTest("DEL /users/:id → seeded users unaffected", testDeleteDoesNotAffectOtherUsers);

  console.log("\n── Products ───────────────────────────────────────────────");
  await runTest("GET /products → array, correct shape, no dup IDs", testGetAllProducts);
  await runTest("GET /products/1/:variant → all variants of Remera", testGetProductVariants);
  await runTest("GET /products/2/:variant → all variants of Zapatilla", testGetProductVariantsProduct2);
  await runTest("GET /products/1/M → no extra fields in response", testGetProductVariantResponseHasNoExtraFields);
  await runTest("GET /products/1/XXXL → 404 + error body", testGetProductVariantNotFound);
  await runTest("GET /products/9999/M → 404 + error body", testGetProductNotFound);

  console.log("\n── General ────────────────────────────────────────────────");
  await runTest("GET /unknown → 404", test404);
  await runTest("POST /users/:id → 404 or 405 (+ Allow header if 405)", testWrongMethod);
  await runTest("PUT /users/:id → 404 or 405", testPutNotAllowed);
  await runTest("Keep-Alive (5 sequential) → all 200", testKeepAlive);
  await runTest("Concurrency (50 parallel) → all identical, no data races", testConcurrency);
  await runTest("Mixed endpoints (50 par.) → routing stable under load", testConcurrentMixedEndpoints);
  await runTest("Rapid fire (100 seq.) → no state leak", testRapidFireDoesNotLeak);
  await runTest("Connection: close → 200, valid JSON", testConnectionClose);
  await runTest("Custom request headers → no crash", testCustomRequestHeaders);

  console.log(`${passed} passed | ${failed} failed | ${passed + failed} total`);

  if (failed > 0) process.exit(1);
}

runAll();