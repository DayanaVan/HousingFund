#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "../src/database/DatabaseManager.h"
#include "../src/config/Config.h"
#include "../src/models/House.h"
#include "../src/models/User.h"
#include "../src/utils/HashUtils.h"

// Тестовый класс для работы с БД
class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовое подключение
        dbManager = new DatabaseManager(Config::getConnectionString());
        
        // Очищаем тестовые данные перед каждым тестом
        clearTestData();
        
        // Подключаемся к БД
        ASSERT_TRUE(dbManager->connect());
    }
    
    void TearDown() override {
        // Очищаем тестовые данные после каждого теста
        clearTestData();
        
        // Отключаемся от БД
        dbManager->disconnect();
        delete dbManager;
    }
    
    // Очистка тестовых данных
    void clearTestData() {
        try {
            pqxx::connection conn(Config::getConnectionString());
            if (conn.is_open()) {
                pqxx::work txn(conn);
                // Удаляем тестовые дома
                txn.exec("DELETE FROM houses WHERE address LIKE 'Тестовый адрес%'");
                // Удаляем тестовых пользователей (кроме стандартных)
                txn.exec("DELETE FROM users WHERE login LIKE 'testuser%'");
                txn.commit();
            }
        } catch (const std::exception& e) {
            // Игнорируем ошибки при очистке
        }
    }
    
    DatabaseManager* dbManager;
};

// Тест подключения к БД
TEST_F(DatabaseTest, ConnectionTest) {
    EXPECT_TRUE(dbManager->isConnected());
}

// Тест добавления дома
TEST_F(DatabaseTest, AddHouseTest) {
    House house;
    house.address = "Тестовый адрес, д. 1";
    house.apartments = 10;
    house.totalArea = 500.5;
    house.buildYear = 2020;
    house.floors = 5;
    
    EXPECT_TRUE(dbManager->addHouse(house));
}

// Тест получения всех домов
TEST_F(DatabaseTest, GetHousesTest) {
    // Сначала добавляем тестовый дом
    House house;
    house.address = "Тестовый адрес для получения";
    house.apartments = 5;
    house.totalArea = 300.0;
    house.buildYear = 2010;
    house.floors = 3;
    
    ASSERT_TRUE(dbManager->addHouse(house));
    
    // Получаем все дома
    auto houses = dbManager->getAllHouses();
    
    // Проверяем, что список не пустой
    EXPECT_FALSE(houses.empty());
    
    // Ищем наш тестовый дом
    bool found = false;
    for (const auto& h : houses) {
        if (h.address == house.address) {
            found = true;
            EXPECT_EQ(h.apartments, house.apartments);
            EXPECT_DOUBLE_EQ(h.totalArea, house.totalArea);
            EXPECT_EQ(h.buildYear, house.buildYear);
            EXPECT_EQ(h.floors, house.floors);
            break;
        }
    }
    
    EXPECT_TRUE(found);
}

// Тест поиска домов старше определенного возраста
TEST_F(DatabaseTest, GetHousesOlderThanTest) {
    // Добавляем старый дом
    House oldHouse;
    oldHouse.address = "Старый тестовый дом";
    oldHouse.apartments = 20;
    oldHouse.totalArea = 1000.0;
    oldHouse.buildYear = 1950; // 74 года (на 2024)
    oldHouse.floors = 4;
    
    ASSERT_TRUE(dbManager->addHouse(oldHouse));
    
    // Добавляем новый дом
    House newHouse;
    newHouse.address = "Новый тестовый дом";
    newHouse.apartments = 50;
    newHouse.totalArea = 3000.0;
    newHouse.buildYear = 2020; // 4 года (на 2024)
    newHouse.floors = 10;
    
    ASSERT_TRUE(dbManager->addHouse(newHouse));
    
    // Ищем дома старше 60 лет
    auto oldHouses = dbManager->getHousesOlderThan(60);
    
    // Должен найти только старый дом
    EXPECT_EQ(oldHouses.size(), 1);
    if (!oldHouses.empty()) {
        EXPECT_EQ(oldHouses[0].address, oldHouse.address);
    }
}

// Тест удаления домов по году
TEST_F(DatabaseTest, DeleteHousesByYearTest) {
    // Добавляем несколько домов одного года
    for (int i = 1; i <= 3; i++) {
        House house;
        house.address = "Тестовый дом " + std::to_string(i) + " для удаления";
        house.apartments = i * 10;
        house.totalArea = i * 100.0;
        house.buildYear = 1999; // Одинаковый год для всех
        house.floors = i;
        
        ASSERT_TRUE(dbManager->addHouse(house));
    }
    
    // Проверяем, что дома добавились
    auto housesBefore = dbManager->getAllHouses();
    int countBefore = 0;
    for (const auto& h : housesBefore) {
        if (h.buildYear == 1999) {
            countBefore++;
        }
    }
    
    EXPECT_GE(countBefore, 3);
    
    // Удаляем дома 1999 года
    EXPECT_TRUE(dbManager->deleteHousesByYear(1999));
    
    // Проверяем, что дома удалились
    auto housesAfter = dbManager->getAllHouses();
    int countAfter = 0;
    for (const auto& h : housesAfter) {
        if (h.buildYear == 1999) {
            countAfter++;
        }
    }
    
    EXPECT_EQ(countAfter, 0);
}

// Тест работы с пользователями
TEST_F(DatabaseTest, UserManagementTest) {
    // Создаем тестового пользователя
    User testUser;
    testUser.login = "testuser";
    testUser.salt = HashUtils::generateSalt();
    testUser.passwordHash = HashUtils::hashPassword("testpassword123", testUser.salt);
    testUser.isAdmin = false;
    
    // Добавляем пользователя
    EXPECT_TRUE(dbManager->addUser(testUser));
    
    // Проверяем существование пользователя
    EXPECT_TRUE(dbManager->userExists("testuser"));
    
    // Получаем пользователя по логину
    User retrievedUser = dbManager->getUserByLogin("testuser");
    EXPECT_EQ(retrievedUser.login, testUser.login);
    EXPECT_EQ(retrievedUser.passwordHash, testUser.passwordHash);
    EXPECT_EQ(retrievedUser.salt, testUser.salt);
    EXPECT_EQ(retrievedUser.isAdmin, testUser.isAdmin);
    
    // Проверяем аутентификацию
    EXPECT_TRUE(dbManager->authenticateUser("testuser", testUser.passwordHash));
    
    // Неправильный пароль должен вернуть false
    EXPECT_FALSE(dbManager->authenticateUser("testuser", "wrongpassword"));
}

// Тест экспорта в файл
TEST_F(DatabaseTest, ExportToFileTest) {
    // Добавляем тестовые данные
    House house1, house2;
    
    house1.address = "Экспорт тест 1";
    house1.apartments = 15;
    house1.totalArea = 750.5;
    house1.buildYear = 2005;
    house1.floors = 6;
    
    house2.address = "Экспорт тест 2";
    house2.apartments = 25;
    house2.totalArea = 1250.75;
    house2.buildYear = 2015;
    house2.floors = 8;
    
    ASSERT_TRUE(dbManager->addHouse(house1));
    ASSERT_TRUE(dbManager->addHouse(house2));
    
    // Экспортируем в файл
    std::vector<std::string> fields = {"address", "apartments", "total_area", "build_year", "floors"};
    std::string filename = "test_export.txt";
    
    EXPECT_TRUE(dbManager->exportToFile(filename, fields));
    
    // Проверяем, что файл создан и не пустой
    std::ifstream file(filename);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    file.close();
    
    // Должна быть хотя бы одна строка с данными + заголовок
    EXPECT_GE(lineCount, 2);
    
    // Удаляем тестовый файл
    std::remove(filename.c_str());
}

// Тест получения количества домов
TEST_F(DatabaseTest, GetHouseCountTest) {
    int initialCount = dbManager->getHouseCount();
    
    // Добавляем тестовый дом
    House house;
    house.address = "Дом для подсчета";
    house.apartments = 10;
    house.totalArea = 500.0;
    house.buildYear = 2021;
    house.floors = 5;
    
    ASSERT_TRUE(dbManager->addHouse(house));
    
    // Количество должно увеличиться на 1
    EXPECT_EQ(dbManager->getHouseCount(), initialCount + 1);
}

// Тест невалидных данных
TEST_F(DatabaseTest, InvalidDataTest) {
    // Пустой адрес
    House invalidHouse1;
    invalidHouse1.address = "";
    invalidHouse1.apartments = 10;
    invalidHouse1.totalArea = 500.0;
    invalidHouse1.buildYear = 2020;
    invalidHouse1.floors = 5;
    
    EXPECT_FALSE(dbManager->addHouse(invalidHouse1));
    
    // Неправильный год (будущее)
    House invalidHouse2;
    invalidHouse2.address = "Будущий дом";
    invalidHouse2.apartments = 10;
    invalidHouse2.totalArea = 500.0;
    invalidHouse2.buildYear = 2050; // Будущий год
    invalidHouse2.floors = 5;
    
    EXPECT_FALSE(dbManager->addHouse(invalidHouse2));
    
    // Отрицательное количество квартир
    House invalidHouse3;
    invalidHouse3.address = "Дом с отрицательными квартирами";
    invalidHouse3.apartments = -5;
    invalidHouse3.totalArea = 500.0;
    invalidHouse3.buildYear = 2020;
    invalidHouse3.floors = 5;
    
    EXPECT_FALSE(dbManager->addHouse(invalidHouse3));
}

// Основная функция запуска тестов
int main(int argc, char **argv) {
    // Инициализация Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "=========================================" << std::endl;
    std::cout << "Запуск тестов базы данных HousingFund" << std::endl;
    std::cout << "Для успешного прохождения тестов убедитесь, что:" << std::endl;
    std::cout << "1. PostgreSQL запущен" << std::endl;
    std::cout << "2. База данных 'housing_fund' существует" << std::endl;
    std::cout << "3. Пользователь 'housing_user' имеет доступ" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Запуск всех тестов
    int result = RUN_ALL_TESTS();
    
    std::cout << "=========================================" << std::endl;
    if (result == 0) {
        std::cout << "✅ Все тесты пройдены успешно!" << std::endl;
    } else {
        std::cout << "❌ Некоторые тесты не прошли" << std::endl;
    }
    std::cout << "=========================================" << std::endl;
    
    return result;
}
