import { describe, expect, test, vi } from "vitest";
import {
    fireEvent,
    render,
    screen,
    waitFor,
} from "@testing-library/svelte";

import Json from "../../src/components/Json.svelte";
import type { Config } from "../../src/types";

const validConfig: Config = {
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
        {
            strokes: [
                {
                    key: "HID_V",
                    modifiers: [
                        "HID_MOD_LCTRL",
                    ],
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

describe("Json", () => {
    test("renders the JSON configuration", () => {
        const onConfigChange = vi.fn();
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Json, {
            props: {
                config: validConfig,
                onConfigChange,
                onSave,
            },
        });

        screen.getByRole("heading", { name: "JSON" });
        screen.getByRole("button", { name: "Save Changes" });
    });

    test("saves a valid configuration", async () => {
        const onConfigChange = vi.fn();
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Json, {
            props: {
                config: validConfig,
                onConfigChange,
                onSave,
            },
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        expect(onConfigChange).toHaveBeenCalledTimes(1);
        expect(onConfigChange).toHaveBeenCalledWith(validConfig);

        expect(onSave).toHaveBeenCalledTimes(1);
    });

    test("does not save an invalid configuration", async () => {
        const onConfigChange = vi.fn();
        const onSave = vi.fn().mockResolvedValue(undefined);

        const invalidConfig = {
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
        } as unknown as Config;

        render(Json, {
            props: {
                config: invalidConfig,
                onConfigChange,
                onSave,
            },
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        expect(onConfigChange).not.toHaveBeenCalled();
        expect(onSave).not.toHaveBeenCalled();

        screen.getByText("Invalid configuration format.");
    });

    test("displays an error when onSave rejects", async () => {
        const onConfigChange = vi.fn();

        const onSave = vi.fn().mockRejectedValue(
            new Error("Failed to save configuration."),
        );

        render(Json, {
            props: {
                config: validConfig,
                onConfigChange,
                onSave,
            },
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        expect(onConfigChange).toHaveBeenCalledTimes(1);
        expect(onSave).toHaveBeenCalledTimes(1);

        await waitFor(() => {
            screen.getByText("Failed to save configuration.");
        });
    });

    test("passes the configuration to onConfigChange before saving", async () => {
        const calls: string[] = [];

        const onConfigChange = vi.fn(() => {
            calls.push("config");
        });

        const onSave = vi.fn(async () => {
            calls.push("save");
        });

        render(Json, {
            props: {
                config: validConfig,
                onConfigChange,
                onSave,
            },
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        expect(calls).toEqual([
            "config",
            "save",
        ]);
    });
});
