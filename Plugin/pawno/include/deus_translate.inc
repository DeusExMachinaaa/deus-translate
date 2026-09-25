// deus_translate.inc - natives del plugin Deus Translate (localización / i18n)
#if defined _deus_translate_included
    #endinput
#endif
#define _deus_translate_included

/*
 * Carga un archivo JSON y lo asocia a un código de idioma.
 *   Lang_Load("ES", "scriptfiles/locales/es.json");
 * La ruta es relativa a la carpeta raíz del server (donde está samp-server.exe
 * / samp03svr), NO a scriptfiles.
 *
 * 'charset' (opcional): si se indica un code page del cliente (p.ej. "cp1251"
 * para cirílico, "cp1252" occidental), el JSON se lee como UTF-8 y se
 * transcodifica a ese code page al cargar. Así los archivos se escriben/editan
 * en UTF-8 y se ven bien en el juego. Si se omite (o ""), el texto pasa tal cual
 * (bytes crudos): guardá el JSON directamente en el charset del cliente.
 *   Lang_Load("RU", "scriptfiles/locales/ru.json", "cp1251");
 */
native Lang_Load(const code[], const file[], const charset[] = "");

// Idioma por defecto (fallback) si el jugador no tiene idioma o le falta la clave.
native Lang_SetDefault(const code[]);

// Asigna un idioma a un jugador. Devuelve 0 si ese idioma no está cargado.
native Lang_SetPlayer(playerid, const code[]);

// Escribe en 'dest' el código de idioma actual del jugador (o el default).
native Lang_GetPlayer(playerid, dest[], size = sizeof dest);

// Escribe en 'dest' el texto de 'key' para el idioma del jugador (fallback al
// default). Devuelve 0 si no la encontró (y copia la propia key en 'dest').
native Lang_GetText(playerid, const key[], dest[], size = sizeof dest);

// Limpia el idioma asignado a un jugador
native Lang_ResetPlayer(playerid);

/*
 * Resuelve 'key' segun el idioma del jugador, formatea (%s %d %i %u %x %c %f
 * %.Nf %% y posicionales %N$ para reordenar, p.ej. "%2$s %1$s") con los
 * argumentos extra y lo envia con SendClientMessage.
 *   SendLanguageMessage(playerid, -1, "MSG_HOLA", nombre);
 */
native SendLanguageMessage(playerid, color, const key[], {Float,_}:...);

/*
 * Como SendLanguageMessage pero a TODOS los jugadores conectados: cada uno
 * recibe el mensaje resuelto en su propio idioma.
 *   SendLanguageMessageToAll(-1, "MSG_ANNOUNCE", texto);
 */
native SendLanguageMessageToAll(color, const key[], {Float,_}:...);
