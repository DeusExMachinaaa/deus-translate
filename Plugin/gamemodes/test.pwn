// test.pwn - prueba del plugin Deus Translate (i18n)
//
// Compilar con pawncc (usa los includes de tu server):
//   pawncc test.pwn -i"pawno\include"
// Poner deus_translate(.dll/.so) en 'plugins/' y en server.cfg:
//   plugins deus_translate
// Copiar los JSON a scriptfiles/locales/ del server.

#include <a_samp>
#include "deus_translate"

main()
{
    print("\n== test.pwn (Deus Translate i18n) ==\n");
}

public OnGameModeInit()
{
    SetGameModeText("Deus Translate i18n test");
    AddPlayerClass(0, 1958.3783, 1343.1572, 15.3746, 269.1425, 0, 0, 0, 0, 0, 0);

    // Rutas relativas a la raíz del server
    Lang_Load("ES", "scriptfiles/locales/es.json");
    Lang_Load("EN", "scriptfiles/locales/en.json");
    Lang_Load("RU", "scriptfiles/locales/ru.json");
    Lang_SetDefault("EN");
    return 1;
}

public OnPlayerConnect(playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof name);

    // Aún sin idioma asignado -> usa el default (EN)
    SendLanguageMessage(playerid, -1, "MSG_HOLA", name);
    SendClientMessage(playerid, 0xAAAAAAFF, "Comandos: /es /en /ru | /hola /dinero /bye /lang /anuncio /orden");
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/es", true)) { Lang_SetPlayer(playerid, "ES"); SendLanguageMessage(playerid, -1, "MSG_LANG_SET"); return 1; }
    if (!strcmp(cmdtext, "/en", true)) { Lang_SetPlayer(playerid, "EN"); SendLanguageMessage(playerid, -1, "MSG_LANG_SET"); return 1; }
    if (!strcmp(cmdtext, "/ru", true)) { Lang_SetPlayer(playerid, "RU"); SendLanguageMessage(playerid, -1, "MSG_LANG_SET"); return 1; }

    if (!strcmp(cmdtext, "/hola", true))
    {
        new name[MAX_PLAYER_NAME];
        GetPlayerName(playerid, name, sizeof name);
        SendLanguageMessage(playerid, -1, "MSG_HOLA", name);
        return 1;
    }
    if (!strcmp(cmdtext, "/dinero", true))
    {
        SendLanguageMessage(playerid, -1, "MSG_DINERO", 2500);
        return 1;
    }
    if (!strcmp(cmdtext, "/bye", true))
    {
        SendLanguageMessage(playerid, -1, "MSG_BYE");
        return 1;
    }
    if (!strcmp(cmdtext, "/lang", true))
    {
        new code[8];
        Lang_GetPlayer(playerid, code, sizeof code);
        SendLanguageMessage(playerid, -1, "MSG_LANG_CUR", code);
        return 1;
    }
    if (!strcmp(cmdtext, "/anuncio", true))
    {
        // A todos, cada uno en su idioma
        SendLanguageMessageToAll(0x88FF88FF, "MSG_ANNOUNCE", "reinicio en 5 min");
        return 1;
    }
    if (!strcmp(cmdtext, "/orden", true))
    {
        // Demo de reordenamiento: mismos args, distinto orden segun el idioma
        // EN: "Lucas beat Pedro."   ES: "Pedro fue vencido por Lucas."
        SendLanguageMessage(playerid, -1, "MSG_ORDER", "Lucas", "Pedro");
        return 1;
    }
    return 0;
}
