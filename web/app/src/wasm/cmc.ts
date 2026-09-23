export interface RunResult {
  ok: boolean;
  output: string[];
  stopped: boolean;
  error: string;
}

export interface CompileResult {
  ok: boolean;
  python: string;
  error: string;
}

export interface CheckResult {
  ok: boolean;
  error: string;
}

export interface BlockFieldSpec {
  key: string;
  label: string;
  type: string;
  width: number;
}

export interface BlockSpec {
  title: string;
  category: string;
  container: string;
  value: boolean;
  fields: BlockFieldSpec[];
  defaults: Record<string, string>;
}

export interface BlockSpecs {
  order: string[];
  categories: string[];
  specs: Record<string, BlockSpec>;
}

export interface BlockNode {
  kind: string;
  fields: Record<string, string>;
  body: BlockNode[];
  elseBody: BlockNode[];
}

export interface BlockSourceResult {
  ok: boolean;
  error: string;
  blocks: BlockNode[];
}

export interface BlockToSourceResult {
  ok: boolean;
  error: string;
  source: string;
}

export interface BlockCheckResult {
  ok: boolean;
  problems: string[];
}

export interface WordInfo {
  name: string;
  display: string;
  category: string;
  summary: string;
  syntax: string;
  doc: string;
  example: string;
  aliases: string[];
  extraSyntax: string[];
  tips: string[];
  related: string[];
}

interface EmscriptenModule {
  _malloc(size: number): number;
  _free(pointer: number): void;
  _cmc_version(): number;
  _cmc_run(source: number, filename: number): void | Promise<void>;
  _cmc_compile(source: number, filename: number): void;
  _cmc_check(source: number): void;
  _cmc_blocks_specs(): void;
  _cmc_blocks_from_source(source: number): void;
  _cmc_blocks_to_source(blocks: number): void;
  _cmc_blocks_check(blocks: number): void;
  _cmc_dictionary(): void;
  _cmc_last(): number;
  _cmc_free(pointer: number): void;
  UTF8ToString(pointer: number): string;
  stringToUTF8(text: string, pointer: number, maxBytes: number): void;
  lengthBytesUTF8(text: string): number;
}

export interface CmcHooks {
  emit?: (line: string) => void;
  draw?: (op: string, a: number, b: number, c: number, d: number, text: string) => void;
  shouldStop?: () => boolean;
  ask?: (prompt: string, done: (answer: string) => void) => void;
}

export class Cmc {
  private module: EmscriptenModule;
  private pendingRun: ((raw: string) => void) | null = null;

  constructor(module: EmscriptenModule) {
    this.module = module;
  }

  handleDone(): void {
    if (this.pendingRun) {
      const resolve = this.pendingRun;
      this.pendingRun = null;
      resolve(this.readResult());
    }
  }

  private alloc(text: string): number {
    const size = this.module.lengthBytesUTF8(text) + 1;
    const pointer = this.module._malloc(size);
    this.module.stringToUTF8(text, pointer, size);
    return pointer;
  }

  private takeString(pointer: number): string {
    const text = this.module.UTF8ToString(pointer);
    this.module._cmc_free(pointer);
    return text;
  }

  private readResult(): string {
    return this.module.UTF8ToString(this.module._cmc_last());
  }

  private isPending(value: unknown): value is Promise<unknown> {
    return (
      typeof value === "object" && value !== null && typeof (value as Promise<unknown>).then === "function"
    );
  }

  private callString(fn: (pointer: number) => void | Promise<void>, text: string): string | Promise<string> {
    const pointer = this.alloc(text);
    const finish = () => {
      this.module._free(pointer);
      return this.readResult();
    };
    const result = fn(pointer);
    if (this.isPending(result)) return result.then(finish);
    return finish();
  }

  private callPair(
    fn: (a: number, b: number) => void | Promise<void>,
    first: string,
    second: string,
  ): string | Promise<string> {
    const a = this.alloc(first);
    const b = this.alloc(second);
    const finish = () => {
      this.module._free(a);
      this.module._free(b);
      return this.readResult();
    };
    const result = fn(a, b);
    if (this.isPending(result)) return result.then(finish);
    return finish();
  }

  private parseResult<T>(raw: string | Promise<string>): T {
    if (typeof raw === "string") return JSON.parse(raw) as T;
    return raw.then((text) => JSON.parse(text) as T) as unknown as T;
  }

  version(): string {
    return this.takeString(this.module._cmc_version());
  }

  run(source: string, filename = "<web>"): Promise<RunResult> {
    return new Promise<RunResult>((resolve) => {
      this.pendingRun = (raw) => resolve(JSON.parse(raw) as RunResult);
      const a = this.alloc(source);
      const b = this.alloc(filename);
      try {
        this.module._cmc_run(a, b);
      } catch (error) {
        this.pendingRun = null;
        resolve({ ok: false, output: [], stopped: false, error: String(error) });
      } finally {
        this.module._free(a);
        this.module._free(b);
      }
    });
  }

  compile(source: string, filename = "<web>"): CompileResult {
    return this.parseResult<CompileResult>(
      this.callPair(this.module._cmc_compile.bind(this.module), source, filename),
    );
  }

  check(source: string): CheckResult {
    return this.parseResult<CheckResult>(
      this.callString(this.module._cmc_check.bind(this.module), source),
    );
  }

  blockSpecs(): BlockSpecs {
    this.module._cmc_blocks_specs();
    return JSON.parse(this.readResult());
  }

  blocksFromSource(source: string): BlockSourceResult {
    return this.parseResult<BlockSourceResult>(
      this.callString(this.module._cmc_blocks_from_source.bind(this.module), source),
    );
  }

  blocksToSource(blocks: BlockNode[]): BlockToSourceResult {
    return this.parseResult<BlockToSourceResult>(
      this.callString(this.module._cmc_blocks_to_source.bind(this.module), JSON.stringify(blocks)),
    );
  }

  blocksCheck(blocks: BlockNode[]): BlockCheckResult {
    return this.parseResult<BlockCheckResult>(
      this.callString(this.module._cmc_blocks_check.bind(this.module), JSON.stringify(blocks)),
    );
  }

  dictionary(): WordInfo[] {
    this.module._cmc_dictionary();
    return JSON.parse(this.readResult());
  }
}

let cached: Promise<Cmc> | null = null;

const dynamicImport = new Function("url", "return import(url)") as (
  url: string,
) => Promise<{ default: (config: Record<string, unknown>) => Promise<EmscriptenModule> }>;

function publicUrl(path: string): string {
  const base = import.meta.env.BASE_URL || "/";
  if (base.startsWith(".")) {
    const page = typeof document !== "undefined" ? document.baseURI : import.meta.url;
    return new URL(path, page).href;
  }
  return `${base}${path}`.replace(/\/{2,}/g, "/").replace(":/", "://");
}

export function loadCmc(hooks: CmcHooks = {}): Promise<Cmc> {
  if (cached) return cached;
  cached = (async () => {
    const url = publicUrl("wasm/cmc.js");
    const factory = (await dynamicImport(url)).default;
    const doneRef: { done?: () => void } = {};
    const module = await factory({
      locateFile: (path: string) => publicUrl(`wasm/${path}`),
      cmcEmit: (line: string) => hooks.emit?.(line),
      cmcDraw: (op: string, a: number, b: number, c: number, d: number, text: string) =>
        hooks.draw?.(op, a, b, c, d, text),
      cmcShouldStop: () => hooks.shouldStop?.() ?? false,
      cmcAsk: (prompt: string, done: (answer: string) => void) => hooks.ask?.(prompt, done),
      cmcDone: () => doneRef.done?.(),
    });
    const cmc = new Cmc(module);
    doneRef.done = () => cmc.handleDone();
    return cmc;
  })();
  return cached;
}
