
export interface KeyStroke {
  key: string;
  modifier: number;
}

export interface Macro {
  key: number;
  name: string;
  strokes: KeyStroke[];
}

export interface Config {
  version: number;
  macros: Macro[];
}
