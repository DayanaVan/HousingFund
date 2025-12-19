#!/bin/bash
# start-docker.sh
set -e

echo "========================================="
echo "Docker развертывание HousingFund"
echo "========================================="

# Проверка Docker
if ! command -v docker &> /dev/null; then
    echo "❌ Docker не установлен. Установите Docker:"
    echo "Ubuntu/Debian: sudo apt install docker.io docker-compose"
    echo "Mac: https://docs.docker.com/desktop/install/mac-install/"
    echo "Windows: https://docs.docker.com/desktop/install/windows-install/"
    exit 1
fi

# Проверка Docker Compose
if ! command -v docker-compose &> /dev/null; then
    echo "Установка Docker Compose..."
    sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
    sudo chmod +x /usr/local/bin/docker-compose
fi

# Настройка X11 для Linux
if [ "$(uname)" = "Linux" ]; then
    echo "Настройка X11 для GUI..."
    xhost +local:docker
fi

# Сборка образов
echo "Сборка Docker образов..."
docker-compose build --no-cache

# Запуск контейнеров
echo "Запуск системы..."
docker-compose up -d

# Ожидание запуска БД
echo "Ожидание запуска PostgreSQL..."
sleep 10

# Проверка статуса
echo "Проверка статуса..."
if docker ps | grep -q housing_postgres && docker ps | grep -q housing_app; then
    echo "========================================="
    echo "✅ Система успешно запущена!"
    echo ""
    echo "Сервисы:"
    echo "  • PostgreSQL:      localhost:5432"
    echo "  • HousingFund:     GUI приложение"
    echo "  • pgAdmin:         http://localhost:5050"
    echo ""
    echo "Данные для входа:"
    echo "  • pgAdmin:         admin@housing.local / admin123"
    echo "  • HousingFund:     admin / admin"
    echo ""
    echo "Команды управления:"
    echo "  • make stop       - остановить"
    echo "  • make logs       - просмотр логов"
    echo "  • make db-shell   - подключиться к БД"
    echo "  • make clean      - полная очистка"
    echo "========================================="
else
    echo "❌ Ошибка при запуске!"
    echo "Логи:"
    docker-compose logs
    exit 1
fi
