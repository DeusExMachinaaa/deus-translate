// lang.cpp - store de idiomas + parser JSON mínimo (string->string)
#include "lang.h"
#include "encoding.h"

#include <cstdio>
#include <cctype>
#include <unordered_map>

namespace {

const int kMaxPlayers = 1000; // SA:MP MAX_PLAYERS

typedef std::unordered_map<std::string, std::string> StringMap;

std::unordered_map<std::string, StringMap> g_langs; // CODE -> (key -> texto)
std::string g_defaultCode;
std::string g_playerCode[kMaxPlayers];

std::string upper(const std::string& s) {
    std::string r(s);
    for (size_t i = 0; i < r.size(); ++i)
        r[i] = (char)std::toupper((unsigned char)r[i]);
    return r;
}

bool readFile(const std::string& path, std::string& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return false; }
    out.resize((size_t)n);
    size_t rd = (n > 0) ? fread(&out[0], 1, (size_t)n, f) : 0;
    fclose(f);
    out.resize(rd);
    // Sacar BOM UTF-8 si está.
    if (out.size() >= 3 &&
        (unsigned char)out[0] == 0xEF &&
        (unsigned char)out[1] == 0xBB &&
        (unsigned char)out[2] == 0xBF)
        out.erase(0, 3);
    return true;
}

// Parser JSON mínimo: objeto de string->string. Objetos anidados se aplanan
// con '.'. Los bytes de los valores se preservan tal cual (passthrough).
struct Parser {
    const char* p;
    const char* end;
    bool ok;
    Parser(const std::string& s)
        : p(s.c_str()), end(s.c_str() + s.size()), ok(true) {}

    void ws() {
        while (p < end) {
            char c = *p;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { p++; }
            else if (c == '/' && p + 1 < end && p[1] == '/') {
                p += 2; while (p < end && *p != '\n') p++;
            } else break;
        }
    }

    bool str(std::string& out) {
        ws();
        if (p >= end || *p != '"') { ok = false; return false; }
        p++;
        out.clear();
        while (p < end) {
            char c = *p++;
            if (c == '"') return true;
            if (c != '\\') { out.push_back(c); continue; }
            if (p >= end) break;
            char e = *p++;
            switch (e) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'u': {
                    if (p + 4 > end) { ok = false; return false; }
                    int cp = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = *p++;
                        cp <<= 4;
                        if (h >= '0' && h <= '9') cp |= h - '0';
                        else if (h >= 'a' && h <= 'f') cp |= h - 'a' + 10;
                        else if (h >= 'A' && h <= 'F') cp |= h - 'A' + 10;
                        else { ok = false; return false; }
                    }
                    out.push_back(cp <= 0xFF ? (char)cp : '?');
                    break;
                }
                default: out.push_back(e); break;
            }
        }
        ok = false;
        return false;
    }

    void object(StringMap& map, const std::string& prefix) {
        ws();
        if (p >= end || *p != '{') { ok = false; return; }
        p++;
        ws();
        if (p < end && *p == '}') { p++; return; }
        while (p < end && ok) {
            std::string key;
            if (!str(key)) { ok = false; return; }
            ws();
            if (p >= end || *p != ':') { ok = false; return; }
            p++;
            ws();
            std::string full = prefix.empty() ? key : prefix + "." + key;
            if (p < end && *p == '"') {
                std::string val;
                if (!str(val)) { ok = false; return; }
                map[full] = val;
            } else if (p < end && *p == '{') {
                object(map, full);
                if (!ok) return;
            } else {
                std::string tok; // número/true/false/null -> texto crudo
                while (p < end && *p != ',' && *p != '}' &&
                       *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
                    tok.push_back(*p++);
                if (tok.empty()) { ok = false; return; }
                map[full] = tok;
            }
            ws();
            if (p < end && *p == ',') { p++; ws(); continue; }
            if (p < end && *p == '}') { p++; return; }
            ok = false;
            return;
        }
    }
};

} // namespace (anónimo)
namespace lang {

bool Load(const std::string& code, const std::string& file, const std::string& charset) {
    std::string data;
    if (!readFile(file, data)) return false;
    Parser parser(data);
    StringMap map;
    parser.object(map, "");
    if (!parser.ok) return false;
    if (!charset.empty()) {
        // Los valores vienen en UTF-8; transcodificar al code page del cliente.
        // Las claves son ASCII, no hace falta convertirlas.
        for (StringMap::iterator it = map.begin(); it != map.end(); ++it) {
            std::string conv;
            if (enc::Utf8ToCodepage(it->second, charset, conv)) it->second = conv;
        }
    }
    g_langs[upper(code)] = map;
    return true;
}

void SetDefault(const std::string& code) {
    g_defaultCode = upper(code);
}

bool SetPlayer(int playerid, const std::string& code) {
    if (playerid < 0 || playerid >= kMaxPlayers) return false;
    std::string c = upper(code);
    if (g_langs.find(c) == g_langs.end()) return false;
    g_playerCode[playerid] = c;
    return true;
}

std::string GetPlayerCode(int playerid) {
    if (playerid < 0 || playerid >= kMaxPlayers) return g_defaultCode;
    if (!g_playerCode[playerid].empty()) return g_playerCode[playerid];
    return g_defaultCode;
}
bool GetTextByCode(const std::string& code, const std::string& key, std::string& out) {
    std::unordered_map<std::string, StringMap>::iterator it = g_langs.find(upper(code));
    if (it == g_langs.end()) return false;
    StringMap::iterator kv = it->second.find(key);
    if (kv == it->second.end()) return false;
    out = kv->second;
    return true;
}

bool GetText(int playerid, const std::string& key, std::string& out) {
    std::string code = GetPlayerCode(playerid);
    if (!code.empty() && GetTextByCode(code, key, out)) return true;
    if (!g_defaultCode.empty() && g_defaultCode != code &&
        GetTextByCode(g_defaultCode, key, out))
        return true;
    return false;
}

int Count() {
    return (int)g_langs.size();
}

void ResetPlayer(int playerid) {
    if (playerid >= 0 && playerid < kMaxPlayers)
        g_playerCode[playerid].clear();
}

} // namespace lang
