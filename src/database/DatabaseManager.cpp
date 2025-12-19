#include "DatabaseManager.h"
#include "config/Config.h"
#include "../utils/HashUtils.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <regex>

using namespace std;

DatabaseManager::DatabaseManager(const string& connStr) 
    : connectionString(connStr), conn(nullptr) {}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect() {
    try {
        conn = new pqxx::connection(connectionString);
        return conn->is_open();
    } catch (const exception& e) {
        cerr << "Connection failed: " << e.what() << endl;
        return false;
    }
}

void DatabaseManager::disconnect() {
    if (conn) {
        delete conn;
        conn = nullptr;
        cout << "Database connection closed." << endl;
    }
}

bool DatabaseManager::isConnected() const {
    return conn && conn->is_open();
}

// === ДОМА ===

bool DatabaseManager::addHouse(const House& house) {
    if (!isConnected() || !house.isValid()) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "INSERT INTO houses (address, apartments, total_area, build_year, floors) "
                     "VALUES ($1, $2, $3, $4, $5)";
        txn.exec_params(sql, house.address, house.apartments, house.totalArea, 
                       house.buildYear, house.floors);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Add house error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::updateHouse(const House& house) {
    if (!isConnected() || !house.isValid() || house.id <= 0) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "UPDATE houses SET address = $1, apartments = $2, "
                     "total_area = $3, build_year = $4, floors = $5 "
                     "WHERE id = $6";
        txn.exec_params(sql, house.address, house.apartments, house.totalArea, 
                       house.buildYear, house.floors, house.id);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Update house error: " << e.what() << endl;
        return false;
    }
}

vector<House> DatabaseManager::getAllHouses() {
    vector<House> houses;
    if (!isConnected()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT id, address, apartments, total_area, build_year, floors, "
                     "EXTRACT(EPOCH FROM created_at) FROM houses ORDER BY id";
        pqxx::result res = ntx.exec(sql);
        
        for (const auto& row : res) {
            houses.push_back(rowToHouse(row));
        }
    } catch (const exception& e) {
        cerr << "Get houses error: " << e.what() << endl;
    }
    return houses;
}

bool DatabaseManager::deleteHousesByYear(int year) {
    if (!isConnected()) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "DELETE FROM houses WHERE build_year = $1";
        txn.exec_params(sql, year);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Delete by year error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByApartments(int minApartments, int maxApartments) {
    if (!isConnected() || minApartments < 0 || maxApartments < minApartments) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "DELETE FROM houses WHERE apartments >= $1 AND apartments <= $2";
        txn.exec_params(sql, minApartments, maxApartments);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Delete by apartments error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByArea(double minArea, double maxArea) {
    if (!isConnected() || minArea < 0 || maxArea < minArea) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "DELETE FROM houses WHERE total_area >= $1 AND total_area <= $2";
        txn.exec_params(sql, minArea, maxArea);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Delete by area error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByAddress(const string& addressPattern) {
    if (!isConnected() || addressPattern.empty()) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "DELETE FROM houses WHERE address ILIKE $1";
        string likePattern = "%" + addressPattern + "%";
        txn.exec_params(sql, likePattern);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Delete by address error: " << e.what() << endl;
        return false;
    }
}

vector<House> DatabaseManager::getHousesOlderThan(int years) {
    vector<House> houses;
    if (!isConnected()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT id, address, apartments, total_area, build_year, floors, "
                     "EXTRACT(EPOCH FROM created_at) "
                     "FROM houses WHERE build_year < EXTRACT(YEAR FROM CURRENT_DATE) - $1 "
                     "ORDER BY build_year";
        pqxx::result res = ntx.exec_params(sql, years);
        
        for (const auto& row : res) {
            houses.push_back(rowToHouse(row));
        }
    } catch (const exception& e) {
        cerr << "Get old houses error: " << e.what() << endl;
    }
    return houses;
}

// === ПРОВЕРКА ДУБЛИКАТОВ ===

bool DatabaseManager::houseExists(const House& house) {
    if (!isConnected() || house.address.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        
        // Проверяем точное совпадение
        string sql = "SELECT COUNT(*) FROM houses WHERE address = $1";
        pqxx::result res = ntx.exec_params(sql, house.address);
        
        if (!res.empty() && res[0][0].as<int>() > 0) {
            return true;
        }
        
        // Проверяем нормализованный адрес
        string normalizedAddress = normalizeAddress(house.address);
        sql = "SELECT address FROM houses";
        pqxx::result allHouses = ntx.exec(sql);
        
        for (const auto& row : allHouses) {
            string dbAddress = row[0].as<string>();
            string normalizedDbAddress = normalizeAddress(dbAddress);
            
            if (normalizedAddress == normalizedDbAddress) {
                return true;
            }
        }
        
        return false;
    } catch (const exception& e) {
        cerr << "Check house exists error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::houseExistsWithDifferentId(const House& house) {
    if (!isConnected() || house.address.empty() || house.id <= 0) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT COUNT(*) FROM houses WHERE address = $1 AND id != $2";
        pqxx::result res = ntx.exec_params(sql, house.address, house.id);
        
        return !res.empty() && res[0][0].as<int>() > 0;
    } catch (const exception& e) {
        cerr << "Check house exists with different ID error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::findSimilarHouses(const House& house, vector<House>& similarHouses, double similarityThreshold) {
    similarHouses.clear();
    if (!isConnected()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        
        // Получаем все дома
        string sql = "SELECT id, address, apartments, total_area, build_year, floors, "
                     "EXTRACT(EPOCH FROM created_at) FROM houses";
        pqxx::result res = ntx.exec(sql);
        
        for (const auto& row : res) {
            House existingHouse = rowToHouse(row);
            
            // Пропускаем тот же самый дом при редактировании
            if (house.id > 0 && existingHouse.id == house.id) {
                continue;
            }
            
            double similarity = 0.0;
            
            // Проверка адреса (основной критерий)
            if (existingHouse.address == house.address) {
                similarity += 0.5;
            } else if (existingHouse.address.find(house.address) != string::npos ||
                      house.address.find(existingHouse.address) != string::npos) {
                similarity += 0.3;
            }
            
            // Проверка года постройки (±5 лет)
            if (abs(existingHouse.buildYear - house.buildYear) <= 5) {
                similarity += 0.1;
            }
            
            // Проверка количества квартир (±20%)
            int aptDiff = abs(existingHouse.apartments - house.apartments);
            double aptRatio = static_cast<double>(aptDiff) / max(existingHouse.apartments, house.apartments);
            if (aptRatio <= 0.2) {
                similarity += 0.1;
            }
            
            // Проверка площади (±15%)
            double areaDiff = fabs(existingHouse.totalArea - house.totalArea);
            double areaRatio = areaDiff / max(existingHouse.totalArea, house.totalArea);
            if (areaRatio <= 0.15) {
                similarity += 0.1;
            }
            
            // Проверка этажности
            if (existingHouse.floors == house.floors) {
                similarity += 0.1;
            } else if (abs(existingHouse.floors - house.floors) <= 1) {
                similarity += 0.05;
            }
            
            if (similarity >= similarityThreshold) {
                similarHouses.push_back(existingHouse);
            }
        }
        
        return true;
    } catch (const exception& e) {
        cerr << "Find similar houses error: " << e.what() << endl;
        return false;
    }
}

// === ПОЛЬЗОВАТЕЛИ ===

User DatabaseManager::getUserByLogin(const string& login) {
    User user;
    if (!isConnected() || login.empty()) return user;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT id, login, password_hash, salt, "
                     "EXTRACT(EPOCH FROM created_at) "
                     "FROM users WHERE login = $1";
        pqxx::result res = ntx.exec_params(sql, login);
        
        if (!res.empty()) {
            user = rowToUser(res[0]);
        }
    } catch (const exception& e) {
        cerr << "Get user error: " << e.what() << endl;
    }
    return user;
}

bool DatabaseManager::authenticateUser(const string& login, const string& passwordHash) {
    if (!isConnected() || login.empty() || passwordHash.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT COUNT(*) FROM users WHERE login = $1 AND password_hash = $2";
        pqxx::result res = ntx.exec_params(sql, login, passwordHash);
        
        return !res.empty() && res[0][0].as<int>() > 0;
    } catch (const exception& e) {
        cerr << "Auth error: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::addUser(const User& user) {
    if (!isConnected() || !user.isValid()) return false;
    
    try {
        pqxx::work txn(*conn);
        
        // Проверяем существование пользователя
        string checkSql = "SELECT COUNT(*) FROM users WHERE login = $1";
        pqxx::result checkRes = txn.exec_params(checkSql, user.login);
        
        if (!checkRes.empty() && checkRes[0][0].as<int>() > 0) {
            cerr << "Пользователь уже существует: " << user.login << endl;
            return false;
        }
        
        // Вставляем нового пользователя
        string sql = "INSERT INTO users (login, password_hash, salt) "
                     "VALUES ($1, $2, $3)";
        txn.exec_params(sql, user.login, user.passwordHash, user.salt);
        txn.commit();
        
        cout << "Пользователь создан: " << user.login << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Ошибка создания пользователя: " << e.what() << endl;
        return false;
    }
}

// === ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===

House DatabaseManager::rowToHouse(const pqxx::row& row) {
    House house;
    house.id = row[0].as<int>();
    house.address = row[1].as<string>();
    house.apartments = row[2].as<int>();
    house.totalArea = row[3].as<double>();
    house.buildYear = row[4].as<int>();
    house.floors = row[5].as<int>();
    house.createdAt = static_cast<time_t>(row[6].as<double>());
    return house;
}

User DatabaseManager::rowToUser(const pqxx::row& row) {
    User user;
    user.id = row[0].as<int>();
    user.login = row[1].as<string>();
    user.passwordHash = row[2].as<string>();
    user.salt = row[3].as<string>();
    user.createdAt = static_cast<time_t>(row[4].as<double>());
    return user;
}

string DatabaseManager::normalizeAddress(const string& address) {
    string normalized = address;
    
    // Приводим к нижнему регистру
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    // Удаляем лишние пробелы
    regex multipleSpaces("\\s+");
    normalized = regex_replace(normalized, multipleSpaces, " ");
    
    // Удаляем начальные и конечные пробелы
    normalized.erase(0, normalized.find_first_not_of(" "));
    normalized.erase(normalized.find_last_not_of(" ") + 1);
    
    // Заменяем сокращения
    regex streetRegex("\\bул\\.|улица\\b");
    normalized = regex_replace(normalized, streetRegex, "ул");
    
    regex avenueRegex("\\bпр\\.|проспект\\b|пр-т\\b");
    normalized = regex_replace(normalized, avenueRegex, "пр");
    
    regex buildingRegex("\\bд\\.|дом\\b");
    normalized = regex_replace(normalized, buildingRegex, "д");
    
    // Удаляем запятые и точки
    regex punctuation("[.,]");
    normalized = regex_replace(normalized, punctuation, "");
    
    return normalized;
}

bool DatabaseManager::exportToFile(const string& filename, 
                                  const vector<string>& fields,
                                  const string& delimiter,
                                  bool includeHeader) {
    if (!isConnected() || fields.empty()) return false;
    
    auto houses = getAllHouses();
    if (houses.empty()) return false;
    
    ofstream file(filename);
    if (!file.is_open()) return false;
    
    // Заголовок
    if (includeHeader) {
        for (size_t i = 0; i < fields.size(); ++i) {
            file << fields[i];
            if (i < fields.size() - 1) file << delimiter;
        }
        file << "\n";
    }
    
    // Данные
    for (const auto& house : houses) {
        for (size_t i = 0; i < fields.size(); ++i) {
            const auto& field = fields[i];
            
            if (field == "id") file << house.id;
            else if (field == "address") file << house.address;
            else if (field == "apartments") file << house.apartments;
            else if (field == "total_area") file << house.totalArea;
            else if (field == "build_year") file << house.buildYear;
            else if (field == "floors") file << house.floors;
            else if (field == "age") file << house.getAge();
            
            if (i < fields.size() - 1) file << delimiter;
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

int DatabaseManager::getHouseCount() {
    if (!isConnected()) return 0;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec("SELECT COUNT(*) FROM houses");
        return res[0][0].as<int>();
    } catch (...) {
        return 0;
    }
}

bool DatabaseManager::userExists(const string& username) {
    if (!isConnected() || username.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT COUNT(*) FROM users WHERE login = $1";
        pqxx::result res = ntx.exec_params(sql, username);
        return !res.empty() && res[0][0].as<int>() > 0;
    } catch (const exception& e) {
        cerr << "User exists check error: " << e.what() << endl;
        return false;
    }
}

vector<House> DatabaseManager::searchHouses(const string& query) {
    vector<House> houses;
    if (!isConnected() || query.empty()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        string sql = "SELECT id, address, apartments, total_area, build_year, floors, "
                     "EXTRACT(EPOCH FROM created_at) "
                     "FROM houses WHERE address ILIKE $1 "
                     "ORDER BY address";
        string likeQuery = "%" + query + "%";
        pqxx::result res = ntx.exec_params(sql, likeQuery);

        for (const auto& row : res) {
            houses.push_back(rowToHouse(row));
        }
    } catch (const exception& e) {
        cerr << "Search houses error: " << e.what() << endl;
    }
    return houses;
}

bool DatabaseManager::deleteHouse(int id) {
    if (!isConnected() || id <= 0) return false;
    
    try {
        pqxx::work txn(*conn);
        string sql = "DELETE FROM houses WHERE id = $1";
        txn.exec_params(sql, id);
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Delete house error: " << e.what() << endl;
        return false;
    }
}
