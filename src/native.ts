import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import { dirname, resolve } from "node:path";

const require = createRequire(import.meta.url);
const packageRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");

export interface NativeParseResult {
  valid: boolean;
  possible: boolean;
  regionCode: string;
  countryCode: number;
  nationalNumber: string;
  category: string;
  e164?: string;
  international?: string;
  national?: string;
  rfc3966?: string;
  significant?: string;
  error?: string;
}

export interface NativeFindNumbersOptions {
  defaultRegionCode?: string;
  leniency?: "possible" | "valid";
  maxTries?: number;
}

export interface NativeMatch {
  text: string;
  start: number;
  end: number;
  phoneNumber: NativeParseResult;
}

export interface NativeAsYouTypeInstance {
  inputDigit(input: string): string;
  reset(): void;
  currentOutput(): string;
}

export interface NativeAddon {
  nativeVersion(): string;
  parsePhoneNumber(rawNumber: string, defaultRegionCode?: string): NativeParseResult;
  findNumbers(text: string, options?: NativeFindNumbersOptions): NativeMatch[];
  getSupportedRegionCodes(): string[];
  getCountryCodeForRegionCode(regionCode: string): number;
  getRegionCodeForCountryCode(countryCode: number): string;
  AsYouType: new (regionCode: string) => NativeAsYouTypeInstance;
}

let cachedAddon: NativeAddon | undefined;

export function loadNativeAddon(): NativeAddon {
  if (!cachedAddon) {
    const loadPrebuild = require("node-gyp-build") as (dir: string) => NativeAddon;
    cachedAddon = loadPrebuild(packageRoot);
  }
  return cachedAddon;
}
