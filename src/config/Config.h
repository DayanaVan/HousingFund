// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdlib>
#include <iostream>

class Config {
public:
    static constexpr int MAX_LOGIN_LENGTH = 50;
    static constexpr int MIN_PASSWORD_LENGTH = 8;
    static constexpr int MAX_PASSWORD_LENGTH = 50;
    static constexpr int MIN_LOGIN_LENGTH = 3;
    
    static std::string getConnectionString() {
        // Получаем переменные окружения
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");
        const char* dbname = std::getenv("DB_NAME");
        const char* user = std::getenv("DB_USER");
        const char* password = std::getenv("DB_PASSWORD");
        
        // Если переменные окружения установлены, используем их
        if (host && port && dbname && user && password) {
            std::string connStr = "host=" + std::string(host) +
                                  " port=" + std::string(port) +
                                  " dbname=" + std::string(dbname) +
                                  " user=" + std::string(user) +
                                  " password=" + std::string(password);
            std::cout << "Using environment variables for DB connection" << std::endl;
            return connStr;
        }
        
        // Иначе используем значения по умолчанию
        std::cout << "Using default DB connection settings" << std::endl;
        return "host=localhost port=5432 dbname=housing_fund "
               "user=housing_user password=secure_password";
    }
};

#endif // CONFIG_H
