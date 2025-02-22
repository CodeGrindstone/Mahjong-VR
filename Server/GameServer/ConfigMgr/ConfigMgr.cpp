//
// Created by 31435 on 2024/9/10.
//

#include "ConfigMgr.h"


ConfigMgr::ConfigMgr()
{
    boost::filesystem::path current_path = boost::filesystem::current_path();
    boost::filesystem::path config_path = current_path / "config.ini";
    std::cout << "Config path: " << config_path << std::endl;

    // 使用Boost.PropertyTree来读取INI文件
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);

    // 遍历INI文件中的所有section
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;
        const boost::property_tree::ptree& section_tree = section_pair.second;
        // 对于每个section，遍历其所有的key-value对
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;
            const std::string& value = key_value_pair.second.get_value<std::string>();
            section_config[key] = value;
        }
        SectionInfo sectionInfo;
        sectionInfo.m_section_data = section_config;
        // 将section的key-value对保存到config_map中
        m_config_map[section_name] = sectionInfo;
    }
    // 输出所有的section和key-value对
    for (const auto& section_entry : m_config_map) {
        const std::string& section_name = section_entry.first;
        SectionInfo section_config = section_entry.second;
        std::cout << "[" << section_name << "]" << std::endl;
        for (const auto& key_value_pair : section_config.m_section_data) {
            std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
        }
    }
}


ConfigMgr::~ConfigMgr()
{
}

SectionInfo ConfigMgr::operator[](const std::string& section_name)
{
    if(m_config_map.find(section_name) == m_config_map.end())
    {
        return SectionInfo();
    }

    return m_config_map[section_name];
}
