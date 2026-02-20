const BASE = "http://localhost:8080";

async function runTest(name, fn) {
  try {
    await fn();
    console.log("✔", name);
  } catch (err) {
    console.error("✖", name, "-", err.message);
  }
}

async function testUserParam() {
  const res = await fetch(`${BASE}/user/42`);

  if (res.status !== 200) throw new Error("Expected 200");

  const json = await res.json();

  if (json.id !== "42")
    throw new Error("Wrong ID param parsing");

  if (json.name !== "John Cena")
    throw new Error("Wrong JSON response");
}

async function testRedirect() {
  const res = await fetch(`${BASE}/old`, {
    redirect: "manual"
  });

  if (![301, 302].includes(res.status))
    throw new Error("Expected redirect status");

  const location = res.headers.get("location");
  if (!location)
    throw new Error("Missing Location header");
}

async function testPostEcho() {
  const payload = { test: 123 };

  const res = await fetch(`${BASE}/echo`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(payload)
  });

  if (res.status !== 200)
    throw new Error("Expected 200");

  const body = await res.text();

  if (body !== "Echo POST received!")
    throw new Error("Unexpected POST response");
}

async function test404() {
  const res = await fetch(`${BASE}/unknown`);

  if (res.status !== 404)
    throw new Error("Expected 404");
}

async function testWrongMethod() {
  const res = await fetch(`${BASE}/hello`, {
    method: "POST"
  });

  if (![404, 405].includes(res.status))
    throw new Error("Expected 404 or 405");
}

async function testKeepAlive() {
  const agent = fetch.Agent ? new fetch.Agent() : null; // for node <18 ignore

  const res1 = await fetch(`${BASE}/hello`);
  if (res1.status !== 200)
    throw new Error("First request failed");

  const res2 = await fetch(`${BASE}/hello`);
  if (res2.status !== 200)
    throw new Error("Second request failed");
}

async function testConcurrency() {
  const requests = Array.from({ length: 50 }).map((_, i) =>
    fetch(`${BASE}/hello?i=${i}`).then(r => {
      if (!r.ok) throw(new Error(`Request ${i} failed with status ${r.status}`));
      return r.text();
    })
  );

  const results = await Promise.all(requests);

  if (results.length !== 50)
    throw new Error("Concurrency failed");
}

async function testLargePost() {
  const largePayload = "x".repeat(5000);

  const res = await fetch(`${BASE}/echo`, {
    method: "POST",
    headers: {
      "Content-Type": "text/plain",
      "Content-Length": largePayload.length.toString()
    },
    body: largePayload
  });

  if (res.status !== 200)
    throw new Error("Large POST failed");
}

async function testCustomHeaders() {
  const res = await fetch(`${BASE}/hello`, {
    headers: {
      "X-Test-Header": "MiniHttp"
    }
  });

  if (res.status !== 200)
    throw new Error("Expected 200");
}

async function testRapidFire() {
  for (let i = 0; i < 100; i++) {
    const res = await fetch(`${BASE}/hello`);
    if (!res.ok)
      throw new Error(`Rapid fire failed at request ${i}`);
  }
}

async function testConnectionCloseHeader() {
  const res = await fetch(`${BASE}/hello`, {
    headers: {
      "Connection": "close"
    }
  });

  if (res.status !== 200)
    throw new Error("Connection close request failed");
}

async function testResponseContentType() {
  const res = await fetch(`${BASE}/user/42`);

  const contentType = res.headers.get("content-type");

  if (!contentType || !contentType.includes("application/json"))
    throw new Error("Missing or wrong Content-Type header");
}

async function runAll() {
  console.log("Running Mini_http tests...\n");

  await runTest("GET /user/:id", testUserParam);
  await runTest("GET redirect", testRedirect);
  await runTest("POST /echo", testPostEcho);
  await runTest("404 route", test404);
  await runTest("Wrong method", testWrongMethod);
  await runTest("Keep-Alive test", testKeepAlive);
  await runTest("Concurrency test", testConcurrency);
  await runTest("Large POST body", testLargePost);
  await runTest("Custom headers", testCustomHeaders);
  await runTest("Rapid fire test", testRapidFire);
  await runTest("Connection: close header", testConnectionCloseHeader);
  await runTest("Response Content-Type", testResponseContentType);

  console.log("\nTests finished.");
}

runAll();