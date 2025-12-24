#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

class Config {
public:
    static constexpr int MAX_LOGIN_LENGTH = 50;
    static constexpr int MIN_PASSWORD_LENGTH = 8;
    static constexpr int MAX_PASSWORD_LENGTH = 50;
    static constexpr int MIN_LOGIN_LENGTH = 3;
    
    static string getConnectionString() {
        // Получаем переменные окружения
        const char* host = getenv("DB_HOST");
        const char* port = getenv("DB_PORT");
        const char* dbname = getenv("DB_NAME");
        const char* user = getenv("DB_USER");
        const char* password = getenv("DB_PASSWORD");
        
        // Если переменные окружения установлены, используем их
        if (host && port && dbname && user && password) {
            string connStr = "host=" + string(host) +
                                  " port=" + string(port) +
                                  " dbname=" + string(dbname) +
                                  " user=" + string(user) +
                                  " password=" + string(password);
            return connStr;
        }
        return "host=localhost port=5432 dbname=housing_fund "
               "user=housing_user password=secure_password";
    }
};

#endif
