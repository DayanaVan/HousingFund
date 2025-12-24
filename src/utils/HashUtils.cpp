#include "HashUtils.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <QString>
#include <QChar>

using namespace std;

const string HashUtils::VALID_CHARS = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "!@#$%^&*()-_=+[]{}|;:,.<>?";

string HashUtils::generateSalt(size_t length) {
    if (length < 8) length = 8;
    
    unsigned char* buffer = new unsigned char[length];
    
    if (RAND_bytes(buffer, length) != 1) {
        delete[] buffer;
        throw runtime_error("Не удалось сгенерировать случайную соль");
    }
    
    stringstream ss;
    for (size_t i = 0; i < length; i++) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(buffer[i]);
    }
    
    delete[] buffer;
    return ss.str();
}

string HashUtils::hashPassword(const string& password, const string& salt) {
    if (password.empty() || salt.empty()) {
        throw std::invalid_argument("Пароль и соль не могут быть пустыми");
    }
    
    string data = password + salt;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);
    
    // Конвертируем в hex-строку
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool HashUtils::verifyPassword(const string& password, const string& hash, const std::string& salt) {
    if (password.empty() || hash.empty() || salt.empty()) {
        return false;
    }
    
    string calculatedHash = hashPassword(password, salt);
    
    if (calculatedHash.length() != hash.length()) {
        return false;
    }
    
    unsigned char result = 0;
    for (size_t i = 0; i < hash.length(); i++) {
        result |= calculatedHash[i] ^ hash[i];
    }
    
    return result == 0;
}

string HashUtils::generateRandomPassword(size_t length) {
    if (length < 8) length = 8;
    
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, VALID_CHARS.size() - 1);
    
    string password;
    password.reserve(length);
    
    password += static_cast<char>(distribution(generator) % 26 + 'A'); 
    password += static_cast<char>(distribution(generator) % 26 + 'a');  
    password += static_cast<char>(distribution(generator) % 10 + '0'); 
    password += "!@#$%^&*"[distribution(generator) % 8];                
    
    // Остальные символы
    for (size_t i = 4; i < length; i++) {
        password += VALID_CHARS[distribution(generator)];
    }
    
    // Перемешиваем
    shuffle(password.begin(), password.end(), generator);
    
    return password;
}

bool HashUtils::isPasswordStrong(const string& password) {
    if (password.length() < 8) {
        return false;
    }
    
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    QString qPassword = QString::fromStdString(password);
    
    for (QChar c : qPassword) {
        ushort unicode = c.unicode();
        
        // Проверка на русские заглавные буквы 
        if ((unicode >= 0x0410 && unicode <= 0x042F) || unicode == 0x0401) {
            hasUpper = true;
        }
        // Проверка на русские строчные буквы 
        else if ((unicode >= 0x0430 && unicode <= 0x044F) || unicode == 0x0451) {
            hasLower = true;
        }
        // Проверка на латинские заглавные буквы
        else if (c.isUpper()) {
            hasUpper = true;
        }
        // Проверка на латинские строчные буквы
        else if (c.isLower()) {
            hasLower = true;
        }
        // Проверка на цифры
        else if (c.isDigit()) {
            hasDigit = true;
        }
        // Проверка на специальные символы
        else {
            if (!c.isSpace() && c.category() != QChar::Other_Control) {
                hasSpecial = true;
            }
        }
    }  
    return hasUpper && hasLower && hasDigit && hasSpecial;
}
