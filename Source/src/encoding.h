#ifndef DEUS_TRANSLATE_ENCODING_H
#define DEUS_TRANSLATE_ENCODING_H

#include <string>

// Conversión de charset para los textos de localización.
//
// SA:MP no usa UTF-8: el cliente renderiza los bytes crudos con su code page
// (p.ej. Windows-1251 para cirílico, Windows-1252 para occidental). Con esto los
// archivos de idioma se pueden escribir en UTF-8 y el plugin los transcodifica
// al code page del cliente al cargarlos.
namespace enc {

// Convierte 'utf8' (texto UTF-8) al code page indicado por 'charset', p.ej.
// "cp1251", "1251" o "windows-1251". Escribe el resultado en 'out' y devuelve
// true. Si 'charset' está vacío o no se reconoce un code page, devuelve false
// (el llamador debe dejar el texto tal cual: passthrough). Los caracteres sin
// representación en el code page destino se reemplazan por '?'.
//
// Implementación por plataforma: WideCharToMultiByte en Windows, iconv en Linux
// (ambas provistas por el SO; no agrega dependencias externas).
bool Utf8ToCodepage(const std::string& utf8, const std::string& charset, std::string& out);

} // namespace enc

#endif // DEUS_TRANSLATE_ENCODING_H
