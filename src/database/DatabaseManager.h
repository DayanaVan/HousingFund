#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <pqxx/pqxx>
#include <vector>
#include <string>
#include "../models/House.h"
#include "../models/User.h"

class DatabaseManager {
public:
    DatabaseManager(const string& connStr);
    ~DatabaseManager();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
  
    bool addHouse(const House& house);
    bool updateHouse(const House& house);
    bool deleteHouse(int id);
    bool deleteHousesByYear(int year);
    bool deleteHousesByApartments(int minApartments, int maxApartments);
    bool deleteHousesByArea(double minArea, double maxArea);
    bool deleteHousesByAddress(const string& addressPattern);
    vector<House> getAllHouses();
    vector<House> getHousesOlderThan(int years);
    vector<House> getHousesByYear(int year);
    vector<House> searchHouses(const string& address);
    
    bool houseExists(const House& house);
    bool houseExistsWithDifferentId(const House& house);
    bool findSimilarHouses(const House& house, vector<House>& similarHouses, double similarityThreshold = 0.8);

    bool addUser(const User& user);
    bool updateUserPassword(const string& login, const string& newHash, const string& newSalt);
    User getUserByLogin(const string& login);
    bool userExists(const string& login);
    bool authenticateUser(const string& login, const string& passwordHash);
    
    bool exportToFile(const string& filename, 
                     const vector<string>& fields,
                     const string& delimiter = "\t",
                     bool includeHeader = true);
    
    int getHouseCount();
    int getUserCount();
    
private:
    pqxx::connection* conn;
    string connectionString;

    House rowToHouse(const pqxx::row& row);
    House rowToHouseFromProcedure(const pqxx::row& row);
    User rowToUser(const pqxx::row& row);
    string normalizeAddress(const string& address);
};

#endif
