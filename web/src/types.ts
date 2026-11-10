export interface KeyStroke {
  key: string;
  modifiers: string[];
}

export type MacroMode = "macro" | "hold";

export interface Macro {
  // "macro" (default): play strokes in order. "hold": hold strokes[0] while
  // the key is held; must have exactly one stroke.
  mode?: MacroMode;
  strokes: KeyStroke[];
}

export interface Config {
  macros: Macro[];
}
