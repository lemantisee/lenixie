#include "Settings.h"

#include <cstring>

#include "stm32f1xx_hal.h"

#include "Logger.h"

namespace
{
    const uint32_t pageSize = 0x400; // 1 kB
    const uint32_t pageAddresStart = 0x0800FC00;
    const uint32_t pageAddresEnd = pageAddresStart + pageSize - 1;
    
}

void Settings::init() 
{
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

    getInstance().readSettings();
}

int Settings::getTimezone(int defaultvalue)
{
    if (getInstance().mData.empty()) {
        return defaultvalue;
    }
    return getInstance().mData.timezone;
}

SString<128> Settings::getNtpUrl(const char *defaultValue)
{
    if (getInstance().mData.empty()) {
        return defaultValue;
    }

    const char *url = getInstance().mData.ntpUrl;
    return SString<128>(url);
}

SString<128> Settings::getWifiSSID(const char *defaultValue)
{
    if (getInstance().mData.empty()) {
        return defaultValue;
    }

    const char *ssid = getInstance().mData.wifiSsid;
    return SString<128>(ssid);
}

SString<128> Settings::getWifiPassword(const char *defaultValue)
{
    if (getInstance().mData.empty()) {
        return defaultValue;
    }

    const char *pass = getInstance().mData.wifiPassword;
    return SString<128>(pass);
}

void Settings::setTimezone(int timezone)
{
    getInstance().mData.timezone = timezone;
    getInstance().writeSettings();
}

void Settings::setNtpUrl(const SString<128> &url) 
{
    if (url.empty()) {
        return;
    }

    std::strcpy(getInstance().mData.ntpUrl, url.c_str());
    getInstance().writeSettings();
}

void Settings::setWifiSSID(const SString<128> &ssid)
{
    if (ssid.empty()) {
        return;
    }

    std::strcpy(getInstance().mData.wifiSsid, ssid.c_str());
    getInstance().writeSettings();
}

void Settings::setWifiPassword(const SString<128> &password)
{
    if (password.empty()) {
        return;
    }

    std::strcpy(getInstance().mData.wifiPassword, password.c_str());
    getInstance().writeSettings();
}

Settings &Settings::getInstance()
{
    static Settings settings;
    return settings;
}

void Settings::readSettings()
{
    mData = {};

    const uint32_t *source_addr = (uint32_t *)pageAddresStart;
    uint32_t *dest_addr = (uint32_t *)&mData;
    for (uint16_t i = 0; i < sizeof(SettingsData) / sizeof(uint32_t); i++) {
        *dest_addr = *(__IO uint32_t *)source_addr;
        ++source_addr;
        ++dest_addr;
    }
}

void Settings::writeSettings()
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseHandle;
    eraseHandle.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseHandle.NbPages = 1;
    eraseHandle.PageAddress = pageAddresStart;
    eraseHandle.Banks = 0;

    uint32_t error = 0;

    if (HAL_FLASHEx_Erase(&eraseHandle, &error) != HAL_OK) {
        LOG_ERROR("Unable to erase page");
        HAL_FLASH_Lock();
        return;
    }

    // Write settings
    mData.setNotEmpty();
    const uint32_t *source_addr = (uint32_t *)&mData;
    uint32_t *dest_addr = (uint32_t *)pageAddresStart;
    for (uint16_t i = 0; i < sizeof(SettingsData) / sizeof(uint32_t); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, uint32_t(dest_addr), *source_addr)
            != HAL_OK) {
            LOG_ERROR("Unable to write to flash");
            break;
        }
        ++source_addr;
        ++dest_addr;
    }

    HAL_FLASH_Lock();
}
