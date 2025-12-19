#ifndef HASHUTILS_H
#define HASHUTILS_H

#include <string>
#include <vector>

class HashUtils {
public:
    // Генерация случайной соли
    static std::string generateSalt(size_t length = 32);
    
    // Хеширование пароля с солью
    static std::string hashPassword(const std::string& password, const std::string& salt);
    
    // Проверка пароля
    static bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    
    // Генерация случайного пароля
    static std::string generateRandomPassword(size_t length = 12);
    
    // Проверка сложности пароля
    static bool isPasswordStrong(const std::string& password);
    
private:
    static const std::string VALID_CHARS;
};

#endif
