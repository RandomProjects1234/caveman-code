type Rgb = readonly [number, number, number]

const NAME_COLORS: Record<string, Rgb> = {
  black: [0, 0, 0],
  white: [255, 255, 255],
  red: [255, 0, 0],
  green: [0, 128, 0],
  blue: [0, 0, 255],
  yellow: [255, 255, 0],
  gold: [255, 215, 0],
  orange: [255, 165, 0],
  purple: [128, 0, 128],
  pink: [255, 192, 203],
  brown: [165, 42, 42],
  gray: [128, 128, 128],
  grey: [128, 128, 128],
  cyan: [0, 255, 255],
  magenta: [255, 0, 255],
  lime: [0, 255, 0],
  navy: [0, 0, 128],
  teal: [0, 128, 128],
  maroon: [128, 0, 0],
  olive: [128, 128, 0],
  silver: [192, 192, 192],
  aqua: [0, 255, 255],
  fuchsia: [255, 0, 255],
  violet: [238, 130, 238],
  indigo: [75, 0, 130],
  tan: [210, 180, 140],
  salmon: [250, 128, 114],
  coral: [255, 127, 80],
  crimson: [220, 20, 60],
  khaki: [240, 230, 140],
  lavender: [230, 230, 250],
  beige: [245, 245, 220],
  turquoise: [64, 224, 208],
  chocolate: [210, 105, 30],
  tomato: [255, 99, 71],
  orchid: [218, 112, 214],
  plum: [221, 160, 221],
  skyblue: [135, 206, 235],
  steelblue: [70, 130, 180],
  forestgreen: [34, 139, 34],
  seagreen: [46, 139, 87],
  darkred: [139, 0, 0],
  darkgreen: [0, 100, 0],
  darkblue: [0, 0, 139],
  lightblue: [173, 216, 230],
  lightgreen: [144, 238, 144],
  lightgray: [211, 211, 211],
  lightgrey: [211, 211, 211],
  darkgray: [169, 169, 169],
  darkgrey: [169, 169, 169],
}

function hexValue(char: string): number {
  const code = char.charCodeAt(0)
  if (code >= 48 && code <= 57) return code - 48
  if (code >= 97 && code <= 102) return code - 87
  if (code >= 65 && code <= 70) return code - 55
  return -1
}

export function parseCaveColor(text: string): string {
  if (text.startsWith('#')) {
    const hex = text.slice(1)
    if (hex.length === 3) {
      const r = hexValue(hex[0])
      const g = hexValue(hex[1])
      const b = hexValue(hex[2])
      if (r >= 0 && g >= 0 && b >= 0) {
        return `rgb(${r * 17}, ${g * 17}, ${b * 17})`
      }
    } else if (hex.length === 6) {
      const values = [
        hexValue(hex[0]),
        hexValue(hex[1]),
        hexValue(hex[2]),
        hexValue(hex[3]),
        hexValue(hex[4]),
        hexValue(hex[5]),
      ]
      if (values.every((value) => value >= 0)) {
        const r = values[0] * 16 + values[1]
        const g = values[2] * 16 + values[3]
        const b = values[4] * 16 + values[5]
        return `rgb(${r}, ${g}, ${b})`
      }
    }
    return 'rgb(0, 0, 0)'
  }
  const rgb = NAME_COLORS[text.toLowerCase()]
  return rgb ? `rgb(${rgb[0]}, ${rgb[1]}, ${rgb[2]})` : 'rgb(0, 0, 0)'
}
