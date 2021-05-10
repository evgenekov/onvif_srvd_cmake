#include "Configuration.hpp"

Configuration::Configuration(std::optional<std::string> const &configFile)
{
    if (configFile.has_value())
    {
        ConfigLoader loader{configFile.value()};
        loadAllSettings(loader);
    }
}

Configuration::Configuration(ConfigLoader &loader)
{
    loadAllSettings(loader);
}

void Configuration::loadAllSettings(ConfigLoader &loader)
{
    // Daemon Info
    loader.getSetting(pid_file, "pidFile");
    loader.getSetting(logLevel, "log_level");
    loader.getSetting(logFile, "log_file");
    loader.getSetting(logFileSizeMb, "log_file_size_mb");
    loader.getSetting(logFileCount, "log_file_count");
    loader.getSetting(logAsync, "log_async");

    // ONVIF Service Options
    loader.getSetting(port, "port");
    loader.getSetting(user, "user");
    loader.getSetting(password, "password");
    loader.getSetting(manufacturer, "manufacturer");
    loader.getSetting(model, "model");
    loader.getSetting(firmware_version, "firmware_ver");
    loader.getSetting(serial_number, "serial_num");
    loader.getSetting(hardware_id, "hardware_id");
    loader.getSetting(interfaces, "interfaces");
    loader.getSetting(tz_format, "tz_format");

    loader.getArray(scopes, "scopes");
    loader.getArray(profiles, "profiles");
    loader.getArray(rtspStreams, "rtspStreams");
}
