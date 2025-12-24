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
        cerr << "Ошибка подключения к БД: " << e.what() << endl;
        return false;
    }
}

void DatabaseManager::disconnect() {
    if (conn) {
        delete conn;
        conn = nullptr;
    }
}

bool DatabaseManager::isConnected() const {
    return conn && conn->is_open();
}

// ДОМА 
bool DatabaseManager::addHouse(const House& house) {
    if (!isConnected() || !house.isValid()) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT add_house($1, $2, $3, $4, $5)",
            house.address, 
            house.apartments, 
            house.totalArea, 
            house.buildYear, 
            house.floors
        );
        
        txn.commit();
        
        if (!res.empty()) {
            int result = res[0][0].as<int>();
            return result > 0;
        }
        return false;
    } catch (const exception& e) {
        cerr << "Ошибка добавления дома: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::updateHouse(const House& house) {
    if (!isConnected() || !house.isValid() || house.id <= 0) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT update_house($1, $2, $3, $4, $5, $6)",
            house.id,
            house.address, 
            house.apartments, 
            house.totalArea, 
            house.buildYear, 
            house.floors
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка обновления дома: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHouse(int id) {
    if (!isConnected() || id <= 0) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT delete_house($1)",
            id
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка удаления дома: " << e.what() << endl;
        return false;
    }
}

vector<House> DatabaseManager::getAllHouses() {
    vector<House> houses;
    if (!isConnected()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec("SELECT * FROM get_all_houses()");
        
        for (const auto& row : res) {
            houses.push_back(rowToHouseFromProcedure(row));
        }
    } catch (const exception& e) {
        cerr << "Ошибка получения списка домов: " << e.what() << endl;
    }
    return houses;
}

bool DatabaseManager::deleteHousesByYear(int year) {
    if (!isConnected()) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT delete_houses_by_year($1)",
            year
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка удаления домов по году: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByApartments(int minApartments, int maxApartments) {
    if (!isConnected() || minApartments < 0 || maxApartments < minApartments) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT delete_houses_by_apartments($1, $2)",
            minApartments, maxApartments
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка удаления домов по квартирам: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByArea(double minArea, double maxArea) {
    if (!isConnected() || minArea < 0 || maxArea < minArea) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT delete_houses_by_area($1, $2)",
            minArea, maxArea
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка удаления домов по площади: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::deleteHousesByAddress(const string& addressPattern) {
    if (!isConnected() || addressPattern.empty()) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT delete_houses_by_address($1)",
            addressPattern
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка удаления домов по адресу: " << e.what() << endl;
        return false;
    }
}

vector<House> DatabaseManager::getHousesOlderThan(int years) {
    vector<House> houses;
    if (!isConnected()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT * FROM get_houses_older_than($1)",
            years
        );
        
        for (const auto& row : res) {
            House house;
            house.id = row["house_id"].as<int>();
            house.address = row["house_address"].as<string>();
            house.apartments = row["house_apartments"].as<int>();
            house.totalArea = row["house_total_area"].as<double>();
            house.buildYear = row["house_build_year"].as<int>();
            house.floors = row["house_floors"].as<int>();
            houses.push_back(house);
        }
    } catch (const exception& e) {
        cerr << "Ошибка получения старых домов: " << e.what() << endl;
    }
    return houses;
}

vector<House> DatabaseManager::getHousesByYear(int year) {
    vector<House> houses;
    if (!isConnected()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT * FROM get_houses_by_year($1)",
            year
        );
        
        for (const auto& row : res) {
            houses.push_back(rowToHouseFromProcedure(row));
        }
    } catch (const exception& e) {
        cerr << "Ошибка получения домов по году: " << e.what() << endl;
    }
    return houses;
}

vector<House> DatabaseManager::searchHouses(const string& query) {
    vector<House> houses;
    if (!isConnected() || query.empty()) return houses;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT * FROM search_houses($1)",
            query
        );
        
        for (const auto& row : res) {
            houses.push_back(rowToHouseFromProcedure(row));
        }
    } catch (const exception& e) {
        cerr << "Ошибка поиска домов: " << e.what() << endl;
    }
    return houses;
}

// ПРОВЕРКА ДУБЛИКАТОВ 
bool DatabaseManager::houseExists(const House& house) {
    if (!isConnected() || house.address.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT house_exists($1)",
            house.address
        );
        
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка проверки существования дома: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::houseExistsWithDifferentId(const House& house) {
    if (!isConnected() || house.address.empty() || house.id <= 0) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT house_exists_except_id($1, $2)",
            house.address,
            house.id
        );
        
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка проверки дома с другим ID: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::findSimilarHouses(const House& house, vector<House>& similarHouses, double similarityThreshold) {
    similarHouses.clear();
    if (!isConnected()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT * FROM find_similar_houses($1, $2)",
            house.id,
            similarityThreshold
        );
        
        for (const auto& row : res) {
            House similarHouse;
            similarHouse.id = row["house_id"].as<int>();
            similarHouse.address = row["house_address"].as<string>();
            similarHouse.apartments = row["house_apartments"].as<int>();
            similarHouse.totalArea = row["house_total_area"].as<double>();
            similarHouse.buildYear = row["house_build_year"].as<int>();
            similarHouse.floors = row["house_floors"].as<int>();
            similarHouses.push_back(similarHouse);
        }
        
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка поиска похожих домов: " << e.what() << endl;
        return false;
    }
}

// ПОЛЬЗОВАТЕЛИ 
bool DatabaseManager::addUser(const User& user) {
    if (!isConnected() || !user.isValid()) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT add_user($1, $2, $3)",
            user.login, 
            user.passwordHash, 
            user.salt
        );
        
        txn.commit();
        
        if (!res.empty()) {
            int result = res[0][0].as<int>();
            return result > 0;
        }
        return false;
    } catch (const exception& e) {
        cerr << "Ошибка добавления пользователя: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::updateUserPassword(const string& login, const string& newHash, const string& newSalt) {
    if (!isConnected() || login.empty() || newHash.empty() || newSalt.empty()) return false;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result res = txn.exec_params(
            "SELECT update_user_password($1, $2, $3)",
            login,
            newHash,
            newSalt
        );
        
        txn.commit();
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка обновления пароля: " << e.what() << endl;
        return false;
    }
}

User DatabaseManager::getUserByLogin(const string& login) {
    User user;
    if (!isConnected() || login.empty()) return user;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT * FROM get_user_by_login($1)",
            login
        );
        
        if (!res.empty()) {
            user.id = res[0]["user_id"].as<int>();
            user.login = res[0]["user_login"].as<string>();
            user.passwordHash = res[0]["user_password_hash"].as<string>();
            user.salt = res[0]["user_salt"].as<string>();
        }
    } catch (const exception& e) {
        cerr << "Ошибка получения пользователя: " << e.what() << endl;
    }
    return user;
}

bool DatabaseManager::userExists(const string& login) {
    if (!isConnected() || login.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT user_exists($1)",
            login
        );
        
        return !res.empty() && res[0][0].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка проверки существования пользователя: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::authenticateUser(const string& login, const string& passwordHash) {
    if (!isConnected() || login.empty() || passwordHash.empty()) return false;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec_params(
            "SELECT authenticate_user($1, $2)",
            login, 
            passwordHash
        );
        
        return !res.empty() && res[0]["is_authenticated"].as<bool>();
    } catch (const exception& e) {
        cerr << "Ошибка аутентификации: " << e.what() << endl;
        return false;
    }
}

// ЭКСПОРТ 
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

// СТАТИСТИКА
int DatabaseManager::getHouseCount() {
    if (!isConnected()) return 0;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec("SELECT get_house_count()");
        return res[0][0].as<int>();
    } catch (...) {
        return 0;
    }
}

int DatabaseManager::getUserCount() {
    if (!isConnected()) return 0;
    
    try {
        pqxx::nontransaction ntx(*conn);
        pqxx::result res = ntx.exec("SELECT get_user_count()");
        return res[0][0].as<int>();
    } catch (...) {
        return 0;
    }
}

// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ 
House DatabaseManager::rowToHouse(const pqxx::row& row) {
    House house;
    house.id = row[0].as<int>();
    house.address = row[1].as<string>();
    house.apartments = row[2].as<int>();
    house.totalArea = row[3].as<double>();
    house.buildYear = row[4].as<int>();
    house.floors = row[5].as<int>();
    if (row.size() > 6) {
        house.createdAt = static_cast<time_t>(row[6].as<double>());
    }
    return house;
}

House DatabaseManager::rowToHouseFromProcedure(const pqxx::row& row) {
    House house;
    house.id = row["house_id"].as<int>();
    house.address = row["house_address"].as<string>();
    house.apartments = row["house_apartments"].as<int>();
    house.totalArea = row["house_total_area"].as<double>();
    house.buildYear = row["house_build_year"].as<int>();
    house.floors = row["house_floors"].as<int>();
    return house;
}

User DatabaseManager::rowToUser(const pqxx::row& row) {
    User user;
    user.id = row[0].as<int>();
    user.login = row[1].as<string>();
    user.passwordHash = row[2].as<string>();
    user.salt = row[3].as<string>();
    if (row.size() > 4) {
        user.createdAt = static_cast<time_t>(row[4].as<double>());
    }
    return user;
}
