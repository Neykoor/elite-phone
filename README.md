# elite-phone

Librería propia de parsing, validación y formateo de números telefónicos internacionales.

No es un fork ni una copia de `awesome-phonenumber`. La única pieza compartida con ese ecosistema es la fuente de datos: la metadata pública de territorios (`PhoneNumberMetadata.xml`) que publica el proyecto `google/libphonenumber` bajo licencia Apache 2.0. Todo el algoritmo de parsing, validación, formateo y el binding nativo son implementación propia.

## Arquitectura

- `native/` — core en C++ (N-API vía node-addon-api), igual que `infinitysqlite`.
- `src/` — capa pública en TypeScript.
- `scripts/generate-metadata.ts` — descarga el XML oficial de Google y lo convierte a `data/metadata.json`, el formato interno que consume el core C++.
- `data/metadata.json` — no se versiona a mano; se regenera con `npm run generate:metadata`.

## Roadmap

1. ✅ Infraestructura y pipeline de metadata
2. ✅ Core C++: matching de `nationalNumberPattern` y lógica de national prefix
3. ✅ Formateo: e164, international, national, rfc3966, significant
4. ✅ `findNumbers`: búsqueda de números dentro de texto
5. ✅ `AsYouType`: formateo incremental mientras se escribe
6. ⏳ Prebuilds multiplataforma (workflow de GitHub Actions, como en infinitysqlite) — todavía no armado; hoy compila local con `node-gyp rebuild`

El motor completo (fases 1-5) está probado con una suite standalone en C++ (`native/core/tests/phone_number_util_test.cc`, sin dependencias de Node) que corre en cualquier máquina con g++ y no requiere la metadata real descargada.

## Atribución de datos

`data/metadata.json` se deriva de `resources/PhoneNumberMetadata.xml` del repositorio `google/libphonenumber`, licenciado bajo Apache License 2.0. El código de este repositorio es independiente de esa licencia; solo los datos numéricos de territorios provienen de ahí.

Fuente: https://github.com/google/libphonenumber/blob/master/resources/PhoneNumberMetadata.xml
