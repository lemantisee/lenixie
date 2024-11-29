#pragma once

#include "SString.h"
#include "lwjson.h"

class JsonObject
{
public:
    JsonObject();
    JsonObject(const char *jsonStr);

    void add(const char *key, int value);
    void add(const char *key, const char *value);
    const SString<256> &dump();

    int getInt(const char *key, int defaultValue) const;
    bool getBool(const char *key, bool defaultValue) const;
    SString<256> get(const char *key) const;

private:
    void addString(const char *value);
    SString<256> mBuffer;

    lwjson_token_t mLwTokens[8];
    lwjson_t mLwJson;
    bool mInParsed = false;
};
