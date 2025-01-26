#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config() // Оператор () позволяет создать объект без явного вызова конструктора.
    {
        reload();
    }
    // Функция reload() перезагружает настройки из конфигурационного файла
    void reload()
    // Загрузка данных
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};
