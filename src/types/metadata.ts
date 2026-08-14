export type PossibleLength = number | [number, number];

export interface NumberTypeDesc {
  possibleLengths: PossibleLength[];
  possibleLengthsLocalOnly: PossibleLength[];
  exampleNumber?: string;
  nationalNumberPattern: string;
}

export interface NumberFormatRule {
  pattern: string;
  leadingDigits: string[];
  format: string;
  intlFormat?: string;
  nationalPrefixFormattingRule?: string;
  nationalPrefixOptionalWhenFormatting: boolean;
  carrierCodeFormattingRule?: string;
}

export interface TerritoryMetadata {
  id: string;
  countryCode: number;
  mainCountryForCode: boolean;
  leadingDigits?: string;
  preferredInternationalPrefix?: string;
  internationalPrefix?: string;
  nationalPrefix?: string;
  nationalPrefixForParsing?: string;
  nationalPrefixTransformRule?: string;
  preferredExtnPrefix?: string;
  nationalPrefixFormattingRule?: string;
  nationalPrefixOptionalWhenFormatting: boolean;
  carrierCodeFormattingRule?: string;
  mobileNumberPortableRegion: boolean;
  generalDesc: { nationalNumberPattern: string };
  possibleLengths: PossibleLength[];
  noInternationalDialling?: NumberTypeDesc;
  fixedLine?: NumberTypeDesc;
  mobile?: NumberTypeDesc;
  pager?: NumberTypeDesc;
  tollFree?: NumberTypeDesc;
  premiumRate?: NumberTypeDesc;
  sharedCost?: NumberTypeDesc;
  personalNumber?: NumberTypeDesc;
  voip?: NumberTypeDesc;
  uan?: NumberTypeDesc;
  voicemail?: NumberTypeDesc;
  availableFormats: NumberFormatRule[];
}

export interface MetadataSnapshot {
  libphonenumberVersion: string;
  generatedAt: string;
  sourceUrl: string;
  territories: TerritoryMetadata[];
}
