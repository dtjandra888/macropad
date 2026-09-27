
import { describe, expect, test } from "vitest";
import { isValidConfig } from "../src/lib/config";

describe("isValidConfig", () => {
  test("accepts a valid configuration", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: [
                "HID_MOD_LCTRL",
                "HID_MOD_LSHIFT",
              ],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(true);
  });

  test("accepts multiple macros", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: ["HID_MOD_LCTRL"],
            },
          ],
        },
        {
          strokes: [
            {
              key: "HID_V",
              modifiers: ["HID_MOD_LCTRL"],
            },
          ],
        },
        {
          strokes: [
            {
              key: "HID_J",
              modifiers: [],
            },
            {
              key: "HID_E",
              modifiers: [],
            },
            {
              key: "HID_L",
              modifiers: [],
            },
            {
              key: "HID_L",
              modifiers: [],
            },
            {
              key: "HID_O",
              modifiers: [],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(true);
  });

  test("accepts strokes with no modifiers", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_H",
              modifiers: [],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(true);
  });

  test("accepts strokes with multiple modifiers", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: [
                "HID_MOD_LCTRL",
                "HID_MOD_LSHIFT",
                "HID_MOD_LALT",
              ],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(true);
  });

  test("rejects null", () => {
    expect(isValidConfig(null)).toBe(false);
  });

  test("rejects non-object values", () => {
    expect(isValidConfig("config")).toBe(false);
    expect(isValidConfig(42)).toBe(false);
    expect(isValidConfig(true)).toBe(false);
  });

  test("rejects a configuration without macros", () => {
    const config = {};

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a configuration with non-array macros", () => {
    const config = {
      macros: {},
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a macro without strokes", () => {
    const config = {
      macros: [
        {},
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a macro with non-array strokes", () => {
    const config = {
      macros: [
        {
          strokes: {},
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a stroke without a key", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              modifiers: [],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a stroke with a non-string key", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: 4,
              modifiers: [],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects a stroke without modifiers", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects non-array modifiers", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: "HID_MOD_LCTRL",
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects non-string modifiers", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: [1],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });

  test("rejects nested modifier arrays", () => {
    const config = {
      macros: [
        {
          strokes: [
            {
              key: "HID_C",
              modifiers: [
                ["HID_MOD_LCTRL"],
              ],
            },
          ],
        },
      ],
    };

    expect(isValidConfig(config)).toBe(false);
  });
});
