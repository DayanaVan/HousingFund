#ifndef USER_H
#define USER_H

#include "../config/Config.h"
#include <string>
#include <ctime>

using namespace std;

struct User {
    int id;
    string login;
    string passwordHash;
    string salt;
    time_t createdAt;
    
    User() : id(0), createdAt(0) {}
    
    bool isValid() const {
        return !login.empty() && 
               !passwordHash.empty() && 
               !salt.empty() &&
               login.length() <= Config::MAX_LOGIN_LENGTH;
    }
    
    bool isPasswordValid(const string& password) const {
        return !password.empty() && 
               password.length() >= Config::MIN_PASSWORD_LENGTH &&
               password.length() <= Config::MAX_PASSWORD_LENGTH;
    }
};

#endif
