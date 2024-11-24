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
    SString<256> &dump();

    int getInt(const char *key, int defaultValue);
    bool getBool(const char *key, bool defaultValue);
    SString<256> get(const char *key);

private:
    void addString(const char *value);
    SString<256> mBuffer;

    lwjson_token_t mLwTokens[4];
    lwjson_t mLwJson;
    bool mInParsed = false;
};
