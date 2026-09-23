#ifndef DEUS_TRANSLATE_LANG_H
#define DEUS_TRANSLATE_LANG_H

#include <string>

// Store de localización (i18n). Todo texto se maneja como bytes crudos:
// no se hace conversión de charset (guardá los JSON en el charset del cliente).
namespace lang {

// Carga un JSON y lo asocia al código de idioma (p.ej. "ES").
// Devuelve true si el archivo se pudo leer y parsear.
bool Load(const std::string& code, const std::string& file);

// Idioma por defecto (fallback global) cuando un jugador no tiene idioma
// asignado o su idioma no contiene la clave pedida.
void SetDefault(const std::string& code);

// Asigna un idioma a un jugador. Devuelve false si el idioma no está cargado.
bool SetPlayer(int playerid, const std::string& code);

// Código de idioma actual del jugador (o el default si no tiene). Puede ser "".
std::string GetPlayerCode(int playerid);

// Resuelve 'key' para el idioma del jugador (con fallback al default).
// Si la encuentra escribe el texto en 'out' y devuelve true.
bool GetText(int playerid, const std::string& key, std::string& out);

// Igual que GetText pero con un código de idioma explícito.
bool GetTextByCode(const std::string& code, const std::string& key, std::string& out);

// Cantidad de idiomas cargados.
int Count();

// Limpia el idioma asignado a un jugador (útil en OnPlayerDisconnect).
void ResetPlayer(int playerid);

} // namespace lang

#endif // DEUS_TRANSLATE_LANG_H
