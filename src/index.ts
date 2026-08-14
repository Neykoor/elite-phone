export type PhoneNumberFormat = "e164" | "international" | "national" | "rfc3966" | "significant";

export type PhoneNumberCategory =
  | "fixedLine"
  | "mobile"
  | "fixedLineOrMobile"
  | "pager"
  | "personalNumber"
  | "premiumRate"
  | "sharedCost"
  | "tollFree"
  | "uan"
  | "voip"
  | "voicemail"
  | "unknown";

export interface ParsePhoneNumberOptions {
  regionCode?: string;
}

export interface PhoneNumberResult {
  valid: boolean;
  possible?: boolean;
  regionCode?: string;
  countryCode?: number;
  category?: PhoneNumberCategory;
  e164?: string;
  international?: string;
  national?: string;
  rfc3966?: string;
  significant?: string;
  error?: string;
}

export interface FindNumbersOptions {
  defaultRegionCode?: string;
  leniency?: "possible" | "valid";
  maxTries?: number;
}

export interface PhoneNumberMatch {
  text: string;
  start: number;
  end: number;
  phoneNumber: PhoneNumberResult;
}

import { loadNativeAddon } from "./native.js";
import type { NativeAsYouTypeInstance } from "./native.js";

const UNRESOLVED_ERRORS = new Set(["empty_number", "unknown_country_code", "unknown_region_code", "region_code_required"]);

export function parsePhoneNumber(
  rawNumber: string,
  options?: ParsePhoneNumberOptions,
): PhoneNumberResult {
  const native = loadNativeAddon();
  const parsed = native.parsePhoneNumber(rawNumber, options?.regionCode ?? "");

  if (parsed.error && UNRESOLVED_ERRORS.has(parsed.error)) {
    return { valid: false, error: parsed.error };
  }

  return {
    valid: parsed.valid,
    possible: parsed.possible,
    regionCode: parsed.regionCode || undefined,
    countryCode: parsed.countryCode || undefined,
    category: parsed.category as PhoneNumberCategory,
    e164: parsed.e164,
    international: parsed.international,
    national: parsed.national,
    rfc3966: parsed.rfc3966,
    significant: parsed.significant,
  };
}

export function findNumbers(text: string, options?: FindNumbersOptions): PhoneNumberMatch[] {
  const native = loadNativeAddon();
  const matches = native.findNumbers(text, {
    defaultRegionCode: options?.defaultRegionCode,
    leniency: options?.leniency,
    maxTries: options?.maxTries,
  });

  return matches.map((match) => ({
    text: match.text,
    start: match.start,
    end: match.end,
    phoneNumber: {
      valid: match.phoneNumber.valid,
      possible: match.phoneNumber.possible,
      regionCode: match.phoneNumber.regionCode || undefined,
      countryCode: match.phoneNumber.countryCode || undefined,
      category: match.phoneNumber.category as PhoneNumberCategory,
      e164: match.phoneNumber.e164,
      international: match.phoneNumber.international,
      national: match.phoneNumber.national,
      rfc3966: match.phoneNumber.rfc3966,
      significant: match.phoneNumber.significant,
    },
  }));
}

export function getSupportedRegionCodes(): string[] {
  return loadNativeAddon().getSupportedRegionCodes();
}

export function getCountryCodeForRegionCode(regionCode: string): number {
  return loadNativeAddon().getCountryCodeForRegionCode(regionCode);
}

export function getRegionCodeForCountryCode(countryCode: number): string {
  return loadNativeAddon().getRegionCodeForCountryCode(countryCode);
}

export class AsYouType {
  private readonly instance: NativeAsYouTypeInstance;

  constructor(regionCode: string) {
    const native = loadNativeAddon();
    this.instance = new native.AsYouType(regionCode);
  }

  inputDigit(input: string): string {
    return this.instance.inputDigit(input);
  }

  reset(): void {
    this.instance.reset();
  }

  get value(): string {
    return this.instance.currentOutput();
  }
}

