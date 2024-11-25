//
// Created by 31435 on 2024/9/10.
//

#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <map>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

struct SectionInfo
{
    std::map<std::string, std::string> m_section_data;
    SectionInfo(){}
    ~SectionInfo(){}

    SectionInfo(const SectionInfo& rhs)
    {
        m_section_data = rhs.m_section_data;
    }

    SectionInfo& operator=(const SectionInfo& rhs)
    {
        if(&rhs==this)  return *this;
        this->m_section_data = rhs.m_section_data;
        return *this;
    }

    std::string operator[](const std::string& key)
    {
        if(m_section_data.find(key)==m_section_data.end())
        {
            return std::string("");
        }
        return m_section_data[key];
    }
};

class ConfigMgr
{
public:
    ConfigMgr(const ConfigMgr& rhs) = delete;
    ConfigMgr& operator=(const ConfigMgr& rhs) = delete;
    SectionInfo operator[](const std::string& section_name);
    ~ConfigMgr();
    static ConfigMgr& getInstance()
    {
        static ConfigMgr instance;
        return instance;
    }
private:
    ConfigMgr();
    // 存储section和key-value对的map
    std::map<std::string, SectionInfo> m_config_map;
};



#endif //CONFIGMGR_H
