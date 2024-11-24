#include "JsonObject.h"

#include <stdio.h>

JsonObject::JsonObject() { mBuffer.append('{'); }

JsonObject::JsonObject(const char *jsonStr) 
{
    lwjson_init(&mLwJson, mLwTokens, 4);
    mInParsed = lwjson_parse(&mLwJson, jsonStr) == lwjsonOK;
}

void JsonObject::add(const char *key, int value)
{
    if (mBuffer.size() > 1) {
        mBuffer += ',';
    }

    addString(key);

    mBuffer += ':';

    SString<5> intStr;
    snprintf(intStr.data(), intStr.capacity(), "%i", value);
    mBuffer += intStr.data();
}

void JsonObject::add(const char *key, const char *value)
{
    if (!mBuffer.empty()) {
        mBuffer += ',';
    }

    addString(key);

    mBuffer += ':';

    addString(value);
}

SString<256> &JsonObject::dump()
{
    if (mBuffer.size() >= mBuffer.capacity()) {
        return mBuffer;
    }

    mBuffer += '}';

    return mBuffer;
}

int JsonObject::getInt(const char *key, int defaultValue)
{
    if (!mInParsed) {
        return defaultValue;
    }

    const lwjson_token_t *token = lwjson_find(&mLwJson, key);
    if (!token || token->type != LWJSON_TYPE_NUM_INT) {
        return defaultValue;
    }

    return token->u.num_int;
}

bool JsonObject::getBool(const char *key, bool defaultValue)
{
    return getInt(key, defaultValue ? 0 : 1);
}

SString<256> JsonObject::get(const char *key)
{
    if (!mInParsed) {
        return {};
    }

    const lwjson_token_t *token = lwjson_find(&mLwJson, key);
    if (!token || token->type != LWJSON_TYPE_STRING) {
        return {};
    }

    return SString<256>(token->u.str.token_value, token->u.str.token_value_len);
}

void JsonObject::addString(const char *value)
{
    mBuffer += '\"';
    mBuffer += value;
    mBuffer += '\"';
}
