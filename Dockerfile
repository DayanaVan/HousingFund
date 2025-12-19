# Dockerfile
FROM ubuntu:22.04 AS builder

# Установка всех зависимостей для сборки
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    git \
    postgresql-server-dev-all \
    libpqxx-dev \
    libssl-dev \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# Копирование исходного кода
WORKDIR /app
COPY . .

# Сборка приложения
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Второй этап - легкий образ для запуска
FROM ubuntu:22.04

# Установка только необходимых зависимостей для запуска
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libpqxx-6.4 \
    libssl3 \
    libqt6core6 \
    libqt6widgets6 \
    libqt6gui6 \
    libgl1-mesa-glx \
    libxcb-xinerama0 \
    ca-certificates \
    fonts-dejavu \
    && rm -rf /var/lib/apt/lists/*

# Создание пользователя для запуска приложения
RUN useradd -m -u 1000 housing && mkdir -p /app && chown housing:housing /app

# Копирование собранного приложения из первого этапа
COPY --from=builder --chown=housing:housing /app/build/HousingFund /app/HousingFund
COPY --chown=housing:housing scripts/setup_database.sql /app/scripts/setup_database.sql

# Копирование библиотек Qt
COPY --from=builder /usr/lib/x86_64-linux-gnu/qt6 /usr/lib/x86_64-linux-gnu/qt6

WORKDIR /app
USER housing

# Установка переменных окружения для подключения к БД
ENV DB_HOST=postgres
ENV DB_PORT=5432
ENV DB_NAME=housing_fund
ENV DB_USER=housing_user
ENV DB_PASSWORD=secure_password
ENV DISPLAY=host.docker.internal:0

# Точка входа
CMD ["/app/HousingFund"]
