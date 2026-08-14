import { writeFileSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { XMLParser } from "fast-xml-parser";
import type {
  MetadataSnapshot,
  NumberFormatRule,
  NumberTypeDesc,
  PossibleLength,
  TerritoryMetadata,
} from "../src/types/metadata.js";

const METADATA_URL =
  "https://raw.githubusercontent.com/google/libphonenumber/master/resources/PhoneNumberMetadata.xml";

const CURRENT_DIR = dirname(fileURLToPath(import.meta.url));
const OUTPUT_PATH = resolve(CURRENT_DIR, "../data/metadata.json");

function toArray<T>(value: T | T[] | undefined): T[] {
  if (value === undefined) return [];
  return Array.isArray(value) ? value : [value];
}

function normalizePattern(raw: unknown): string {
  return String(raw ?? "").replace(/\s+/g, "");
}

function parsePossibleLengths(raw: string | undefined): PossibleLength[] {
  if (!raw) return [];
  return raw.split(",").map((part) => {
    const trimmed = part.trim();
    const rangeMatch = trimmed.match(/^\[(\d+)-(\d+)\]$/);
    if (rangeMatch) {
      return [Number(rangeMatch[1]), Number(rangeMatch[2])] as [number, number];
    }
    return Number(trimmed);
  });
}

function parseNumberTypeDesc(node: any): NumberTypeDesc | undefined {
  if (!node) return undefined;
  const possibleLengths = node.possibleLengths ?? {};
  return {
    possibleLengths: parsePossibleLengths(possibleLengths["@_national"]),
    possibleLengthsLocalOnly: parsePossibleLengths(possibleLengths["@_localOnly"]),
    exampleNumber: node.exampleNumber !== undefined ? String(node.exampleNumber) : undefined,
    nationalNumberPattern: normalizePattern(node.nationalNumberPattern),
  };
}

function parseNumberFormat(node: any): NumberFormatRule {
  return {
    pattern: String(node["@_pattern"]),
    leadingDigits: toArray(node.leadingDigits).map((value) => normalizePattern(value)),
    format: String(node.format),
    intlFormat: node.intlFormat !== undefined ? String(node.intlFormat) : undefined,
    nationalPrefixFormattingRule: node["@_nationalPrefixFormattingRule"],
    nationalPrefixOptionalWhenFormatting:
      node["@_nationalPrefixOptionalWhenFormatting"] === "true",
    carrierCodeFormattingRule: node["@_carrierCodeFormattingRule"],
  };
}

function parseTerritory(node: any): TerritoryMetadata {
  const availableFormats = toArray(node.availableFormats?.numberFormat).map(parseNumberFormat);
  const subtypes = [
    parseNumberTypeDesc(node.fixedLine),
    parseNumberTypeDesc(node.mobile),
    parseNumberTypeDesc(node.pager),
    parseNumberTypeDesc(node.tollFree),
    parseNumberTypeDesc(node.premiumRate),
    parseNumberTypeDesc(node.sharedCost),
    parseNumberTypeDesc(node.personalNumber),
    parseNumberTypeDesc(node.voip),
    parseNumberTypeDesc(node.uan),
    parseNumberTypeDesc(node.voicemail),
    parseNumberTypeDesc(node.noInternationalDialling),
  ];
  const possibleLengths = subtypes
    .filter((desc): desc is NumberTypeDesc => desc !== undefined)
    .flatMap((desc) => desc.possibleLengths);
  return {
    id: String(node["@_id"]),
    countryCode: Number(node["@_countryCode"]),
    mainCountryForCode: node["@_mainCountryForCode"] === "true",
    leadingDigits: node["@_leadingDigits"],
    preferredInternationalPrefix: node["@_preferredInternationalPrefix"],
    internationalPrefix: node["@_internationalPrefix"],
    nationalPrefix: node["@_nationalPrefix"],
    nationalPrefixForParsing: node["@_nationalPrefixForParsing"],
    nationalPrefixTransformRule: node["@_nationalPrefixTransformRule"],
    preferredExtnPrefix: node["@_preferredExtnPrefix"],
    nationalPrefixFormattingRule: node["@_nationalPrefixFormattingRule"],
    nationalPrefixOptionalWhenFormatting: node["@_nationalPrefixOptionalWhenFormatting"] === "true",
    carrierCodeFormattingRule: node["@_carrierCodeFormattingRule"],
    mobileNumberPortableRegion: node["@_mobileNumberPortableRegion"] === "true",
    generalDesc: { nationalNumberPattern: normalizePattern(node.generalDesc?.nationalNumberPattern) },
    possibleLengths,
    noInternationalDialling: parseNumberTypeDesc(node.noInternationalDialling),
    fixedLine: parseNumberTypeDesc(node.fixedLine),
    mobile: parseNumberTypeDesc(node.mobile),
    pager: parseNumberTypeDesc(node.pager),
    tollFree: parseNumberTypeDesc(node.tollFree),
    premiumRate: parseNumberTypeDesc(node.premiumRate),
    sharedCost: parseNumberTypeDesc(node.sharedCost),
    personalNumber: parseNumberTypeDesc(node.personalNumber),
    voip: parseNumberTypeDesc(node.voip),
    uan: parseNumberTypeDesc(node.uan),
    voicemail: parseNumberTypeDesc(node.voicemail),
    availableFormats,
  };
}

async function main(): Promise<void> {
  const response = await fetch(METADATA_URL);
  if (!response.ok) {
    throw new Error(`No se pudo descargar la metadata: ${response.status} ${response.statusText}`);
  }
  const xml = await response.text();
  const parser = new XMLParser({ ignoreAttributes: false, attributeNamePrefix: "@_" });
  const parsed = parser.parse(xml);
  const territoryNodes = toArray(parsed.phoneNumberMetadata?.territories?.territory);
  const territories = territoryNodes.map(parseTerritory);

  const snapshot: MetadataSnapshot = {
    libphonenumberVersion: "unknown",
    generatedAt: new Date().toISOString(),
    sourceUrl: METADATA_URL,
    territories,
  };

  mkdirSync(dirname(OUTPUT_PATH), { recursive: true });
  writeFileSync(OUTPUT_PATH, JSON.stringify(snapshot, null, 2));
  console.log(`Generados ${territories.length} territorios en ${OUTPUT_PATH}`);
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
