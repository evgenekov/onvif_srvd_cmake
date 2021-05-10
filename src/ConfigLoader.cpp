#include "ConfigLoader.hpp"
#include <iostream>

ConfigLoader::ConfigLoader(std::string const &configFile)
    : m_configFile{configFile}, m_cfg(std::make_unique<libconfig::Config>())
{
    try
    {
        m_cfg->readFile(configFile.c_str());
        m_cfg->setAutoConvert(true);
    }
    catch (libconfig::FileIOException const &fioex)
    {
        std::string err = configFile + " : " + fioex.what() + " while attempting to read file.";
        std::cerr << err << std::endl;
        throw std::runtime_error(err);
    }
    catch (libconfig::ParseException const &pex)
    {
        std::string err =
            configFile + " : " + pex.getError() + " while parsing line " + std::to_string(pex.getLine()) + ".";
        std::cerr << err << std::endl;
        throw std::runtime_error(err);
    }
}
