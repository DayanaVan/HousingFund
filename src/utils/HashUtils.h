#ifndef HASHUTILS_H
#define HASHUTILS_H

#include <string>
#include <vector>

using namespace std;

class HashUtils {
public:
    static string generateSalt(size_t length = 32);
    static string hashPassword(const string& password, const string& salt);
    static bool verifyPassword(const string& password, const string& hash, const string& salt);
    static string generateRandomPassword(size_t length = 12);
    static bool isPasswordStrong(const string& password);
private:
    static const string VALID_CHARS;
};

#endif
