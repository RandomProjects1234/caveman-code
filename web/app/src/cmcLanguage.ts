import type { Monaco } from '@monaco-editor/react'

export const CMC_LANGUAGE = 'cmc'
export const CMC_THEME = 'ogabooga-cave'

const KEYWORDS = [
  'oga',
  'blorp',
  'grunk',
  'binga',
  'wonga',
  'unga',
  'booga',
  'zug',
  'zoop',
  'clump',
  'ork',
  'skrib',
  'ugg',
  'and',
  'or',
  'not',
  'in',
  'if',
  'else',
  'while',
  'for',
  'function',
  'def',
  'fn',
  'repeat',
  'end',
  'done',
  'return',
  'give',
  'ask',
  'print',
  'say',
  'let',
  'make',
  'draw',
  'comment',
]

const BUILTINS = [
  'snorf',
  'plop',
  'nom',
  'skoop',
  'yoink',
  'goop',
  'shout',
  'whisper',
  'flip',
  'find',
  'split',
  'join',
  'what',
  'munga',
  'numba',
  'round',
  'flat',
  'roof',
  'abs',
  'small',
  'big',
  'root',
  'pow',
  'wait',
]

const CONSTANTS = ['gronk', 'nork', 'plop', 'pi', 'true', 'false', 'nothing']

let registered = false

export function registerCmcLanguage(monaco: Monaco): void {
  if (registered) return
  registered = true

  monaco.languages.register({ id: CMC_LANGUAGE })

  monaco.languages.setLanguageConfiguration(CMC_LANGUAGE, {
    comments: { lineComment: 'ugg' },
    brackets: [
      ['(', ')'],
      ['[', ']'],
    ],
    autoClosingPairs: [
      { open: '(', close: ')' },
      { open: '[', close: ']' },
      { open: '"', close: '"' },
      { open: "'", close: "'" },
    ],
    surroundingPairs: [
      { open: '(', close: ')' },
      { open: '[', close: ']' },
      { open: '"', close: '"' },
      { open: "'", close: "'" },
    ],
    wordPattern: /[A-Za-z_][A-Za-z0-9_]*/,
  })

  monaco.languages.setMonarchTokensProvider(CMC_LANGUAGE, {
    keywords: KEYWORDS,
    builtins: BUILTINS,
    constants: CONSTANTS,
    tokenizer: {
      root: [
        [/ugg(?![A-Za-z0-9_]).*$/, 'comment'],
        [
          /[a-zA-Z_][a-zA-Z0-9_]*/,
          {
            cases: {
              '@keywords': 'keyword',
              '@builtins': 'builtin',
              '@constants': 'constant',
              '@default': 'identifier',
            },
          },
        ],
        [/\d+(\.\d+)?/, 'number'],
        [/"([^"\\]|\\.)*$/, 'string.invalid'],
        [/'([^'\\]|\\.)*$/, 'string.invalid'],
        [/"/, { token: 'string.quote', bracket: '@open', next: '@doubleString' }],
        [/'/, { token: 'string.quote', bracket: '@open', next: '@singleString' }],
        [/[=><!~?:&|+\-*/^%]+/, 'operator'],
        [/[{}()[\]]/, '@brackets'],
        [/[;,.]/, 'delimiter'],
        [/\s+/, 'white'],
      ],
      doubleString: [
        [/[^\\"]+/, 'string'],
        [/\\./, 'string.escape'],
        [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
      ],
      singleString: [
        [/[^\\']+/, 'string'],
        [/\\./, 'string.escape'],
        [/'/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
      ],
    },
  })

  monaco.editor.defineTheme(CMC_THEME, {
    base: 'vs-dark',
    inherit: true,
    rules: [
      { token: 'keyword', foreground: 'ffb340', fontStyle: 'bold' },
      { token: 'builtin', foreground: '64b5f6' },
      { token: 'constant', foreground: 'c792ea' },
      { token: 'string', foreground: '4cc38a' },
      { token: 'string.escape', foreground: 'ffd166' },
      { token: 'string.invalid', foreground: 'ff6b6b' },
      { token: 'number', foreground: 'c792ea' },
      { token: 'comment', foreground: '8a7a68', fontStyle: 'italic' },
      { token: 'operator', foreground: 'b39b80' },
      { token: 'identifier', foreground: 'f7ead9' },
      { token: 'delimiter', foreground: 'b39b80' },
      { token: 'delimiter.bracket', foreground: 'b39b80' },
    ],
    colors: {
      'editor.background': '#241a12',
      'editor.foreground': '#f7ead9',
      'editor.selectionBackground': '#4a3728',
      'editor.inactiveSelectionBackground': '#3d2d1f',
      'editor.selectionHighlightBackground': '#4a372855',
      'editor.lineHighlightBackground': '#31241a',
      'editor.lineHighlightBorder': '#00000000',
      'editorCursor.foreground': '#ffb340',
      'editorLineNumber.foreground': '#8a7a68',
      'editorLineNumber.activeForeground': '#b39b80',
      'editorIndentGuide.background': '#3d2d1f',
      'editorIndentGuide.activeBackground': '#4a3728',
      'editorWhitespace.foreground': '#4a3728',
      'editorBracketMatch.background': '#4a3728',
      'editorBracketMatch.border': '#ffb340',
      'editorWidget.background': '#31241a',
      'editorWidget.border': '#4a3728',
      'editorSuggestWidget.background': '#31241a',
      'editorSuggestWidget.border': '#4a3728',
      'editorSuggestWidget.selectedBackground': '#4a3728',
      'editorHoverWidget.background': '#31241a',
      'editorHoverWidget.border': '#4a3728',
      'editorGutter.background': '#241a12',
      'editorError.foreground': '#ff6b6b',
      'editorFindMatch': '#ffb34088',
      'editorFindMatchHighlight': '#ffb34044',
      'scrollbarSlider.background': '#4a372880',
      'scrollbarSlider.hoverBackground': '#4a3728cc',
      'scrollbarSlider.activeBackground': '#4a3728',
      'editorOverviewRuler.border': '#00000000',
      'minimap.background': '#241a12',
    },
  })
}
