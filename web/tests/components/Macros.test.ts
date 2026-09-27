import { describe, expect, test, vi } from "vitest";
import {
    fireEvent,
    render,
    screen,
} from "@testing-library/svelte";

import Macros from "../../src/components/Macros.svelte";
import type { Macro } from "../../src/types";

const macros: Macro[] = [
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
        strokes: [],
    },
];

describe("Macros", () => {
    test("renders the page heading", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        screen.getByRole("heading", { name: "Macros" });
        screen.getByText("Configure what each key does.");
        screen.getByRole("button", { name: "Save Changes" });
    });

    test("renders a card for each macro", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        expect(screen.getAllByRole("article")).toHaveLength(3);

        screen.getByText("Key 0");
        screen.getByText("Key 1");
        screen.getByText("Key 2");
    });

    test("displays the number of keystrokes for assigned macros", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        expect(screen.getAllByText("1 keystroke(s)")).toHaveLength(2);
    });

    test("displays 'No macro assigned' for an empty macro", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        screen.getByText("No macro assigned");
    });

    test("renders an Edit button for each macro", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        expect(
            screen.getAllByRole("button", { name: "Edit" }),
        ).toHaveLength(3);
    });

    test("calls onSave when Save Changes is clicked", async () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros,
                onSave,
            },
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        expect(onSave).toHaveBeenCalledTimes(1);
    });

    test("renders no macro cards when macros is empty", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                macros: [],
                onSave,
            },
        });

        expect(screen.queryAllByRole("article")).toHaveLength(0);
    });

    test("uses the default empty macros array", () => {
        const onSave = vi.fn().mockResolvedValue(undefined);

        render(Macros, {
            props: {
                onSave,
            },
        });

        expect(screen.queryAllByRole("article")).toHaveLength(0);
    });
});
