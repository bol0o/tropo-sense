import express from "express";
import morgan from "morgan";

const app = express();

// logi dostępu (metoda, URL, status, czas)
app.use(morgan("dev"));

// parser JSON (z limitem – dopasuj do swoich payloadów)
app.use(express.json({ limit: "256kb" }));

// endpoint do przyjmowania danych z modemu
app.post("/ingest", (req, res) => {
  // IP nadawcy
  const ip =
    req.headers["x-forwarded-for"]?.toString().split(",")[0].trim() ||
    req.socket.remoteAddress;

  console.log("----- NEW POST /ingest -----");
  console.log("From:", ip);
  console.log("Headers:", JSON.stringify(req.headers, null, 2));

  // Sam JSON (Express już sparsował)
  console.log("Body:", JSON.stringify(req.body, null, 2));

  // jeśli chcesz zweryfikować strukturę:
  // if (!req.body || typeof req.body !== "object" || !req.body.foo) {
  //   return res.status(400).json({ ok: false, error: "Missing field: foo" });
  // }

  res.json({ ok: true });
});

// prosty healthcheck
app.get("/health", (_req, res) => res.json({ status: "ok" }));

// obsługa błędów parsera JSON (np. niepoprawny JSON)
app.use((err, _req, res, _next) => {
  if (err instanceof SyntaxError) {
    console.error("JSON parse error:", err.message);
    return res.status(400).json({ ok: false, error: "Invalid JSON" });
  }
  console.error("Unhandled error:", err);
  res.status(500).json({ ok: false, error: "Internal Server Error" });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`GSM API listening on http://0.0.0.0:${PORT}`);
});