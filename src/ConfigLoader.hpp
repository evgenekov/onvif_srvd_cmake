#pragma once

#include <array>
#include <cstdlib>
#include <libconfig.h++>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ConfigLoader
{
  public:
    explicit ConfigLoader(std::string const &configFile);

    template <typename T>
    void getSetting(T &var, std::string const &setting) const
    {
        std::lock_guard<std::mutex> lock(m_cfgMutex);
        try
        {
            // LLVM's libc++ doesn't like linking with std::string interface
            if constexpr (std::is_same_v<T, std::string>)
            {
                var = static_cast<char const *>(m_cfg->lookup(setting));
            }
            else
            {
                var = static_cast<T>(m_cfg->lookup(setting));
            }
        }
        catch (libconfig::SettingNotFoundException &)
        {
            // var keeps its default, all is OK.
        }
    }

    template <typename T>
    void getArray(std::vector<T> &var, std::string const &setting) const
    {
        std::lock_guard<std::mutex> lock(m_cfgMutex);
        try
        {
            const libconfig::Setting &items = m_cfg->lookup(setting);
            std::size_t length = static_cast<std::size_t>(items.getLength());
            var.clear();
            var.reserve(length);
            for (std::size_t i = 0; i < length; ++i)
            {
                var.push_back(static_cast<T &&>(items[i]));
            }
        }
        catch (libconfig::SettingNotFoundException &)
        {
            // var keeps its default, all is OK.
        }
    }

    template <typename T, std::size_t N>
    void getArray(std::array<T, N> &var, std::string const &setting) const
    {
        std::lock_guard<std::mutex> lock(m_cfgMutex);
        try
        {
            const libconfig::Setting &items = m_cfg->lookup(setting);
            std::size_t length = static_cast<std::size_t>(items.getLength());
            if (length != N)
            {
                std::string msg{m_configFile + ", " + setting + ": Error reading data for array of size " +
                                std::to_string(N)};
                throw std::runtime_error(msg.c_str());
            }
            for (std::size_t i = 0; i < length; ++i)
            {
                var.at(i) = static_cast<T &&>(items[i]);
            }
        }
        catch (libconfig::SettingNotFoundException &)
        {
            // var keeps its default, all is OK.
        }
    }

  private:
    std::string m_configFile;
    std::unique_ptr<libconfig::Config> m_cfg;
    mutable std::mutex m_cfgMutex;
};
