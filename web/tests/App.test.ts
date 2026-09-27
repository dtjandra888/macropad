import { describe, expect, test, vi, beforeEach, afterEach } from "vitest";
import {
    fireEvent,
    render,
    screen,
    waitFor,
} from "@testing-library/svelte";

import App from "../src/App.svelte";

const validConfig = {
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
    ],
};

describe("App", () => {
    beforeEach(() => {
        vi.stubGlobal(
            "fetch",
            vi.fn().mockResolvedValue(
                new Response(
                    JSON.stringify(validConfig),
                    {
                        status: 200,
                        headers: {
                            "Content-Type": "application/json",
                        },
                    },
                ),
            ),
        );
    });

    afterEach(() => {
        vi.restoreAllMocks();
    });

    test("shows the loading state while configuration is loading", async () => {
        let resolveFetch!: (
            response: Response,
        ) => void;

        vi.stubGlobal(
            "fetch",
            vi.fn().mockImplementation(
                () =>
                    new Promise<Response>((resolve) => {
                        resolveFetch = resolve;
                    }),
            ),
        );

        render(App);

        screen.getByText("Loading...");
        screen.getByText("Loading configuration...");

        resolveFetch(
            new Response(
                JSON.stringify(validConfig),
                {
                    status: 200,
                    headers: {
                        "Content-Type": "application/json",
                    },
                },
            ),
        );

        await waitFor(() => {
            screen.getByText("Connected");
        });
    });

    test("loads configuration on startup", async () => {
        render(App);

        await waitFor(() => {
            expect(fetch).toHaveBeenCalledWith("/api/config");
        });
    });

    test("shows the Macros page after configuration loads", async () => {
        render(App);

        await waitFor(() => {
            screen.getByRole("heading", { name: "Macros" });
        });

        screen.getByText("Configure what each key does.");
    });

    test("shows Connected when configuration loads successfully", async () => {
        render(App);

        await waitFor(() => {
            screen.getByText("Connected");
        });

        expect(
            screen.queryByText("Disconnected"),
        ).toBeNull();
    });

    test("shows Disconnected when configuration fails to load", async () => {
        vi.stubGlobal(
            "fetch",
            vi.fn().mockRejectedValue(
                new Error("Failed to load configuration."),
            ),
        );

        render(App);

        await waitFor(() => {
            screen.getByText("Disconnected");
        });

        expect(
            screen.queryByText("Loading configuration..."),
        ).toBeNull();

        expect(
            screen.queryByText("Loading..."),
        ).toBeNull();
    });

    test("navigates to the JSON page", async () => {
        render(App);

        await waitFor(() => {
            screen.getByRole("heading", { name: "Macros" });
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "JSON" }),
        );

        screen.getByRole("heading", { name: "JSON" });
    });

    test("navigates to the About page", async () => {
        render(App);

        await waitFor(() => {
            screen.getByRole("heading", { name: "Macros" });
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "About" }),
        );

        screen.getByRole("heading", { name: "About" });
    });

    test("navigates back to the Macros page", async () => {
        render(App);

        await waitFor(() => {
            screen.getByRole("heading", { name: "Macros" });
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "JSON" }),
        );

        screen.getByRole("heading", { name: "JSON" });

        await fireEvent.click(
            screen.getByRole("button", { name: "Macros" }),
        );

        screen.getByRole("heading", { name: "Macros" });
    });

    test("saves configuration with POST /api/config", async () => {
        const mockFetch = vi.fn()
            .mockResolvedValueOnce(
                new Response(
                    JSON.stringify(validConfig),
                    {
                        status: 200,
                        headers: {
                            "Content-Type": "application/json",
                        },
                    },
                ),
            )
            .mockResolvedValueOnce(
                new Response(null, {
                    status: 200,
                }),
            );

        vi.stubGlobal("fetch", mockFetch);

        render(App);

        await waitFor(() => {
            screen.getByRole("heading", { name: "Macros" });
        });

        await fireEvent.click(
            screen.getByRole("button", { name: "Save Changes" }),
        );

        await waitFor(() => {
            expect(mockFetch).toHaveBeenCalledTimes(2);
        });

        expect(mockFetch).toHaveBeenNthCalledWith(
            2,
            "/api/config",
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(validConfig),
            },
        );
    });
});
