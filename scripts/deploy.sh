#!/bin/bash
set -e

echo "========================================="
echo "Установка HousingFund на Ubuntu/Linux"
echo "========================================="

# Проверка прав
if [ "$EUID" -eq 0 ]; then
    echo "❌ Не запускайте скрипт от root! Используйте обычного пользователя."
    exit 1
fi

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Функции для вывода
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 1. Обновление системы
print_info "Обновление пакетов системы..."
sudo apt update
sudo apt upgrade -y

# 2. Установка зависимостей
print_info "Установка компилятора и инструментов..."
sudo apt install -y build-essential cmake g++ gcc git wget curl

# 3. Установка PostgreSQL
print_info "Установка PostgreSQL..."
sudo apt install -y postgresql postgresql-contrib libpq-dev
sudo systemctl start postgresql
sudo systemctl enable postgresql

# 4. Установка Qt6
print_info "Установка Qt6..."

if lsb_release -a 2>/dev/null | grep -q "24.04"; then
    print_warn "Обнаружена Ubuntu 24.04, использую новые имена пакетов..."
    sudo apt install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
        libqt6core6t64 libqt6widgets6t64 libqt6gui6t64
        
    if apt-cache show qttools6-dev > /dev/null 2>&1; then
        sudo apt install -y qttools6-dev
    elif apt-cache show qttools6-dev-tools > /dev/null 2>&1; then
        sudo apt install -y qttools6-dev-tools
    else
        print_warn "qttools6-dev не найден, пропускаем..."
    fi
else
    sudo apt install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
        libqt6core6 libqt6widgets6 libqt6gui6 qttools6-dev
fi

# 5. Установка libpqxx
print_info "Установка libpqxx (C++ библиотека для PostgreSQL)..."
sudo apt install -y libpqxx-dev

# 6. Установка OpenSSL
print_info "Установка OpenSSL для хеширования..."
sudo apt install -y libssl-dev

# 7. Настройка базы данных
print_info "Настройка базы данных PostgreSQL..."

# Создание пользователя БД (если не существует)
sudo -u postgres psql -c "SELECT 1 FROM pg_roles WHERE rolname='housing_user'" | grep -q 1 || {
    sudo -u postgres psql -c "CREATE USER housing_user WITH PASSWORD 'secure_password';"
    print_info "Пользователь housing_user создан"
}

# Создание базы данных (если не существует)
sudo -u postgres psql -c "SELECT 1 FROM pg_database WHERE datname='housing_fund'" | grep -q 1 || {
    sudo -u postgres psql -c "CREATE DATABASE housing_fund OWNER housing_user;"
    print_info "База данных housing_fund создана"
}

# Настройка прав
sudo -u postgres psql -c "ALTER USER housing_user WITH SUPERUSER;"

# 8. Создание таблиц
print_info "Создание таблиц в базе данных..."
if [ -f "scripts/setup_database.sql" ]; then
    sudo -u postgres psql -d housing_fund -f scripts/setup_database.sql
else
    print_warn "Файл scripts/setup_database.sql не найден. Создание стандартных таблиц..."
    
    sudo -u postgres psql -d housing_fund << EOF
    CREATE TABLE IF NOT EXISTS users (
        id SERIAL PRIMARY KEY,
        login VARCHAR(50) UNIQUE NOT NULL,
        password_hash VARCHAR(256) NOT NULL,
        salt VARCHAR(64) NOT NULL,
        is_admin BOOLEAN DEFAULT FALSE,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS houses (
        id SERIAL PRIMARY KEY,
        address VARCHAR(200) NOT NULL,
        apartments INT NOT NULL,
        total_area DECIMAL(10,2) NOT NULL,
        build_year INT NOT NULL,
        floors INT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    INSERT INTO users (login, password_hash, salt, is_admin) VALUES 
    ('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'admin_salt_123', TRUE)
    ON CONFLICT (login) DO NOTHING;
EOF
fi

# 9. Сборка проекта
print_info "Сборка проекта..."
if [ -d "build" ]; then
    print_warn "Удаление старой папки build..."
    rm -rf build
fi

mkdir build
cd build

print_info "Конфигурация CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

print_info "Компиляция..."
make -j$(nproc)

# 10. Проверка сборки
if [ -f "HousingFund" ]; then
    print_info "✅ Сборка завершена успешно!"
    echo ""
    echo "========================================="
    echo "Данные для входа:"
    echo "Логин: admin"
    echo "Пароль: admin"
    echo "========================================="
    echo ""
    echo "Запуск приложения:"
    echo "  ./build/HousingFund"
    echo ""
    echo "Или установка в систему:"
    echo "  sudo make install"
    echo "  HousingFund"
else
    print_error "❌ Сборка не удалась!"
    exit 1
fi

# 11. Опционально: создание desktop-файла
if [ "$1" = "--desktop" ]; then
    print_info "Создание ярлыка на рабочем столе..."
    
    cat > ~/Desktop/HousingFund.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=HousingFund
Comment=Управление жилым фондом
Exec=$(pwd)/HousingFund
Icon=applications-office
Terminal=false
Categories=Office;
EOF
    
    chmod +x ~/Desktop/HousingFund.desktop
    print_info "Ярлык создан на рабочем столе"
fi

print_info "Установка завершена успешно! 🎉"
