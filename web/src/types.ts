
export interface KeyStroke {
  key: string;
  modifier: number;
}

export interface Macro {
  strokes: KeyStroke[];
}

export interface Config {
  version: number;
  macros: Macro[];
}
