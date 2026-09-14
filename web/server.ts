import express from "express";
import { readFile, writeFile } from "node:fs/promises";
import path from "node:path";

const app = express();
const port = 3000;

const configPath = path.resolve("config.json");

app.use(express.json());

app.get("/api/config", async (_req, res) => {
  try {
    const config = await readFile(configPath, "utf-8");

    res.type("application/json").send(config);
  } catch (error) {
    console.error(error);
    res.status(500).json({
      error: "Failed to read configuration",
    });
  }
});

app.post("/api/config", async (req, res) => {
  try {
    const config = req.body;

    await writeFile(
      configPath,
      JSON.stringify(config, null, 2) + "\n",
      "utf-8"
    );

    res.sendStatus(204);
  } catch (error) {
    console.error(error);
    res.status(500).json({
      error: "Failed to save configuration",
    });
  }
});

app.listen(port, () => {
  console.log(`Configuration server running on http://localhost:${port}`);
});
