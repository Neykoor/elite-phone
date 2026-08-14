import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import type {
  MetadataSnapshot,
  NumberFormatRule,
  NumberTypeDesc,
  PossibleLength,
  TerritoryMetadata,
} from "../src/types/metadata.js";

const CURRENT_DIR = dirname(fileURLToPath(import.meta.url));
const INPUT_PATH = resolve(CURRENT_DIR, "../data/metadata.json");
const OUTPUT_CC_PATH = resolve(CURRENT_DIR, "../native/generated/metadata.gen.cc");
const OUTPUT_H_PATH = resolve(CURRENT_DIR, "../native/generated/metadata.gen.h");

function escapeCpp(value: string): string {
  return value.replace(/\\/g, "\\\\").replace(/"/g, '\\"');
}

function cppBool(value: boolean): string {
  return value ? "true" : "false";
}

function cppPossibleLengths(lengths: PossibleLength[]): string {
  const items = lengths.map((length) =>
    Array.isArray(length) ? `{${length[0]}, ${length[1]}}` : `{${length}, ${length}}`,
  );
  return `{${items.join(", ")}}`;
}

function cppOptionalDesc(desc: NumberTypeDesc | undefined): string {
  if (!desc) return "std::nullopt";
  return [
    "NumberTypeDesc{",
    `      ${cppPossibleLengths(desc.possibleLengths)},`,
    `      ${cppPossibleLengths(desc.possibleLengthsLocalOnly)},`,
    `      "${escapeCpp(desc.exampleNumber ?? "")}",`,
    `      "${escapeCpp(desc.nationalNumberPattern)}"`,
    "    }",
  ].join("\n    ");
}

function cppFormats(formats: NumberFormatRule[]): string {
  if (formats.length === 0) return "{}";
  const items = formats.map((format) => {
    const leadingDigits = format.leadingDigits.map((digits) => `"${escapeCpp(digits)}"`).join(", ");
    return [
      "NumberFormatRule{",
      `        "${escapeCpp(format.pattern)}",`,
      `        {${leadingDigits}},`,
      `        "${escapeCpp(format.format)}",`,
      `        "${escapeCpp(format.intlFormat ?? "")}",`,
      `        "${escapeCpp(format.nationalPrefixFormattingRule ?? "")}",`,
      `        ${cppBool(format.nationalPrefixOptionalWhenFormatting)},`,
      `        "${escapeCpp(format.carrierCodeFormattingRule ?? "")}"`,
      "      }",
    ].join("\n      ");
  });
  return `{\n      ${items.join(",\n      ")}\n    }`;
}

function cppTerritoryBlock(territory: TerritoryMetadata): string {
  const lines: string[] = [];
  lines.push("  {");
  lines.push("    TerritoryMetadata territory;");
  lines.push(`    territory.id = "${escapeCpp(territory.id)}";`);
  lines.push(`    territory.countryCode = ${territory.countryCode};`);
  lines.push(`    territory.mainCountryForCode = ${cppBool(territory.mainCountryForCode)};`);
  lines.push(`    territory.leadingDigits = "${escapeCpp(territory.leadingDigits ?? "")}";`);
  lines.push(
    `    territory.preferredInternationalPrefix = "${escapeCpp(territory.preferredInternationalPrefix ?? "")}";`,
  );
  lines.push(`    territory.internationalPrefix = "${escapeCpp(territory.internationalPrefix ?? "")}";`);
  lines.push(`    territory.nationalPrefix = "${escapeCpp(territory.nationalPrefix ?? "")}";`);
  lines.push(
    `    territory.nationalPrefixForParsing = "${escapeCpp(territory.nationalPrefixForParsing ?? "")}";`,
  );
  lines.push(
    `    territory.nationalPrefixTransformRule = "${escapeCpp(territory.nationalPrefixTransformRule ?? "")}";`,
  );
  lines.push(
    `    territory.nationalPrefixFormattingRule = "${escapeCpp(territory.nationalPrefixFormattingRule ?? "")}";`,
  );
  lines.push(
    `    territory.nationalPrefixOptionalWhenFormatting = ${cppBool(territory.nationalPrefixOptionalWhenFormatting)};`,
  );
  lines.push(
    `    territory.carrierCodeFormattingRule = "${escapeCpp(territory.carrierCodeFormattingRule ?? "")}";`,
  );
  lines.push(`    territory.mobileNumberPortableRegion = ${cppBool(territory.mobileNumberPortableRegion)};`);
  lines.push(`    territory.generalDescPattern = "${escapeCpp(territory.generalDesc.nationalNumberPattern)}";`);
  lines.push(`    territory.availableFormats = ${cppFormats(territory.availableFormats)};`);
  lines.push(`    territory.noInternationalDialling = ${cppOptionalDesc(territory.noInternationalDialling)};`);
  lines.push(`    territory.fixedLine = ${cppOptionalDesc(territory.fixedLine)};`);
  lines.push(`    territory.mobile = ${cppOptionalDesc(territory.mobile)};`);
  lines.push(`    territory.pager = ${cppOptionalDesc(territory.pager)};`);
  lines.push(`    territory.tollFree = ${cppOptionalDesc(territory.tollFree)};`);
  lines.push(`    territory.premiumRate = ${cppOptionalDesc(territory.premiumRate)};`);
  lines.push(`    territory.sharedCost = ${cppOptionalDesc(territory.sharedCost)};`);
  lines.push(`    territory.personalNumber = ${cppOptionalDesc(territory.personalNumber)};`);
  lines.push(`    territory.voip = ${cppOptionalDesc(territory.voip)};`);
  lines.push(`    territory.uan = ${cppOptionalDesc(territory.uan)};`);
  lines.push(`    territory.voicemail = ${cppOptionalDesc(territory.voicemail)};`);
  lines.push("    MetadataRegistry::instance().registerTerritory(std::move(territory));");
  lines.push("  }");
  return lines.join("\n");
}

function main(): void {
  const snapshot = JSON.parse(readFileSync(INPUT_PATH, "utf8")) as MetadataSnapshot;

  const header = ["#pragma once", "", "namespace elitephone {", "", "void registerGeneratedMetadata();", "", "}", ""].join(
    "\n",
  );

  const body = [
    '#include "metadata.gen.h"',
    "",
    '#include "../core/metadata_registry.h"',
    '#include "../core/types.h"',
    "",
    "#include <optional>",
    "#include <utility>",
    "",
    "namespace elitephone {",
    "",
    "void registerGeneratedMetadata() {",
    snapshot.territories.map(cppTerritoryBlock).join("\n"),
    "}",
    "",
    "}",
    "",
  ].join("\n");

  mkdirSync(dirname(OUTPUT_CC_PATH), { recursive: true });
  writeFileSync(OUTPUT_H_PATH, header);
  writeFileSync(OUTPUT_CC_PATH, body);
  console.log(`Generado ${OUTPUT_CC_PATH} con ${snapshot.territories.length} territorios`);
}

main();
