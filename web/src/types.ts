
export interface KeyStroke {
  key: string;
  modifier: string[];
}

export interface Macro {
  strokes: KeyStroke[];
}

export interface Config {
  macros: Macro[];
}
