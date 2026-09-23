// main.cpp - Deus Translate: plugin de localización (i18n) para SA:MP
// Natives: Lang_Load, Lang_SetDefault, Lang_SetPlayer, Lang_GetPlayer,
//          Lang_GetText, SendLanguageMessage
#include <cstdio>
#include <cstring>
#include <string>

#include "amx/amx.h"
#include "plugincommon.h"
#include "lang.h"

typedef void (*logprintf_t)(const char* format, ...);
static logprintf_t logprintf = nullptr;
extern void* pAMXFunctions;   // definido en sdk/amxplugin.cpp

// ---------------------------------------------------------------------------
// Helpers AMX <-> std::string
// ---------------------------------------------------------------------------
static std::string GetString(AMX* amx, cell amx_addr) {
    cell* addr = nullptr;
    amx_GetAddr(amx, amx_addr, &addr);
    int len = 0;
    amx_StrLen(addr, &len);
    std::string s;
    if (len > 0) {
        s.resize(len + 1);
        amx_GetString(&s[0], addr, 0, len + 1);
        s.resize(len);
    }
    return s;
}

static void SetString(AMX* amx, cell amx_addr, const std::string& src, size_t maxcells) {
    cell* addr = nullptr;
    amx_GetAddr(amx, amx_addr, &addr);
    amx_SetString(addr, src.c_str(), 0, 0, maxcells);
}

// ---------------------------------------------------------------------------
// Llamar a la native SendClientMessage del server desde C++
// ---------------------------------------------------------------------------
static AMX_NATIVE FindNative(AMX* amx, const char* name) {
    int index = -1;
    if (amx_FindNative(amx, name, &index) != AMX_ERR_NONE) return nullptr;
    AMX_HEADER* hdr = (AMX_HEADER*)amx->base;
    AMX_FUNCSTUB* stub =
        (AMX_FUNCSTUB*)(amx->base + hdr->natives + index * hdr->defsize);
    return (AMX_NATIVE)(size_t)stub->address; // 'address' es el 1er campo del stub
}

static void CallSendClientMessage(AMX* amx, int playerid, int color,
                                  const std::string& text) {
    AMX_NATIVE f = FindNative(amx, "SendClientMessage");
    if (!f) { if (logprintf) logprintf("[Deus Translate] SendClientMessage no disponible"); return; }
    cell amx_addr = 0;
    cell* phys = nullptr;
    int cells = (int)text.size() + 1;
    if (amx_Allot(amx, cells, &amx_addr, &phys) != AMX_ERR_NONE) return;
    amx_SetString(phys, text.c_str(), 0, 0, cells);
    cell params[4];
    params[0] = 3 * sizeof(cell);
    params[1] = (cell)playerid;
    params[2] = (cell)color;
    params[3] = amx_addr;
    f(amx, params);
    amx_Release(amx, amx_addr);
}

// ---------------------------------------------------------------------------
// Formateo estilo printf leyendo los args variádicos del AMX
// Soporta: %s %d %i %u %x %c %f %.Nf %%
// ---------------------------------------------------------------------------
static std::string FormatText(AMX* amx, const std::string& tpl,
                              const cell* params, int firstArg) {
    int argc = (int)(params[0] / sizeof(cell)); // args válidos: params[1..argc]
    int arg = firstArg;
    std::string out;
    char tmp[64];
    size_t i = 0, n = tpl.size();
    while (i < n) {
        char c = tpl[i++];
        if (c != '%') { out.push_back(c); continue; }
        if (i >= n) { out.push_back('%'); break; }
        if (tpl[i] == '%') { out.push_back('%'); i++; continue; }
        int prec = -1;
        if (tpl[i] == '.') {
            size_t j = i + 1; int p = 0; bool any = false;
            while (j < n && tpl[j] >= '0' && tpl[j] <= '9') { p = p*10 + (tpl[j]-'0'); j++; any = true; }
            if (any && j < n && tpl[j] == 'f') { prec = p; i = j; }
        }
        char spec = (i < n) ? tpl[i++] : '\0';
        cell* cp = nullptr;
        bool has = (arg <= argc);
        if (has) amx_GetAddr(amx, params[arg++], &cp);
        switch (spec) {
            case 'd': case 'i':
                if (has && cp) { snprintf(tmp, sizeof(tmp), "%d", (int)*cp); out += tmp; }
                break;
            case 'u':
                if (has && cp) { snprintf(tmp, sizeof(tmp), "%u", (unsigned)*cp); out += tmp; }
                break;
            case 'x':
                if (has && cp) { snprintf(tmp, sizeof(tmp), "%x", (unsigned)*cp); out += tmp; }
                break;
            case 'c':
                if (has && cp) out.push_back((char)*cp);
                break;
            case 'f':
                if (has && cp) {
                    float fv = amx_ctof(*cp);
                    char spc[16];
                    if (prec >= 0) snprintf(spc, sizeof(spc), "%%.%df", prec);
                    else strcpy(spc, "%f");
                    snprintf(tmp, sizeof(tmp), spc, fv);
                    out += tmp;
                }
                break;
            case 's':
                if (has && cp) {
                    int len = 0; amx_StrLen(cp, &len);
                    if (len > 0) {
                        std::string s; s.resize(len + 1);
                        amx_GetString(&s[0], cp, 0, len + 1);
                        s.resize(len); out += s;
                    }
                }
                break;
            default:
                out.push_back('%');
                if (spec) out.push_back(spec);
                break;
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// Natives
// ---------------------------------------------------------------------------
// native Lang_Load(const code[], const file[]);
static cell AMX_NATIVE_CALL n_Lang_Load(AMX* amx, cell* params) {
    std::string code = GetString(amx, params[1]);
    std::string file = GetString(amx, params[2]);
    bool ok = lang::Load(code, file);
    if (logprintf) {
        if (ok) logprintf("  >> [Deus Translate] idioma '%s' cargado (%s)", code.c_str(), file.c_str());
        else    logprintf("  >> [Deus Translate] ERROR al cargar '%s' (%s)", code.c_str(), file.c_str());
    }
    return ok ? 1 : 0;
}
// native Lang_SetDefault(const code[]);
static cell AMX_NATIVE_CALL n_Lang_SetDefault(AMX* amx, cell* params) {
    lang::SetDefault(GetString(amx, params[1]));
    return 1;
}
// native Lang_SetPlayer(playerid, const code[]);
static cell AMX_NATIVE_CALL n_Lang_SetPlayer(AMX* amx, cell* params) {
    return lang::SetPlayer((int)params[1], GetString(amx, params[2])) ? 1 : 0;
}
// native Lang_GetPlayer(playerid, dest[], size = sizeof dest);
static cell AMX_NATIVE_CALL n_Lang_GetPlayer(AMX* amx, cell* params) {
    std::string code = lang::GetPlayerCode((int)params[1]);
    SetString(amx, params[2], code, (size_t)params[3]);
    return (cell)code.size();
}
// native Lang_GetText(playerid, const key[], dest[], size = sizeof dest);
static cell AMX_NATIVE_CALL n_Lang_GetText(AMX* amx, cell* params) {
    std::string key = GetString(amx, params[2]);
    std::string out;
    bool found = lang::GetText((int)params[1], key, out);
    if (!found) out = key;
    SetString(amx, params[3], out, (size_t)params[4]);
    return found ? 1 : 0;
}
// native SendLanguageMessage(playerid, color, const key[], {Float,_}:...);
static cell AMX_NATIVE_CALL n_SendLanguageMessage(AMX* amx, cell* params) {
    int playerid = (int)params[1];
    int color = (int)params[2];
    std::string key = GetString(amx, params[3]);
    std::string tpl;
    if (!lang::GetText(playerid, key, tpl)) tpl = key;
    std::string msg = FormatText(amx, tpl, params, 4);
    CallSendClientMessage(amx, playerid, color, msg);
    return 1;
}
static const AMX_NATIVE_INFO g_Natives[] = {
    { "Lang_Load",           n_Lang_Load },
    { "Lang_SetDefault",     n_Lang_SetDefault },
    { "Lang_SetPlayer",      n_Lang_SetPlayer },
    { "Lang_GetPlayer",      n_Lang_GetPlayer },
    { "Lang_GetText",        n_Lang_GetText },
    { "SendLanguageMessage", n_SendLanguageMessage },
    { nullptr, nullptr }
};

// ---------------------------------------------------------------------------
// Exports del plugin
// ---------------------------------------------------------------------------
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}
PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData) {
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    logprintf("  >> Deus Translate v1.0 by DeusExMachina");
    logprintf("  >> https://github.com/DeusExMachinaaa");
    return true;
}
PLUGIN_EXPORT void PLUGIN_CALL Unload() {
    if (logprintf) logprintf("  >> Deus Translate descargado.");
}
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx) {
    return amx_Register(amx, g_Natives, -1);
}
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx) {
    return AMX_ERR_NONE;
}
PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() { }
