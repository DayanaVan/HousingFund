-- База данных жилого фонда для Linux/C++/PostgreSQL
-- Версия 2.1

-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    salt VARCHAR(64) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица домов
CREATE TABLE IF NOT EXISTS houses (
    id SERIAL PRIMARY KEY,
    address VARCHAR(200) NOT NULL,
    apartments INT NOT NULL CHECK (apartments > 0),
    total_area DECIMAL(10,2) NOT NULL CHECK (total_area > 0),
    build_year INT NOT NULL CHECK (build_year >= 1500 AND build_year <= EXTRACT(YEAR FROM CURRENT_DATE)),
    floors INT NOT NULL CHECK (floors > 0 AND floors <= 100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Индексы для ускорения поиска
CREATE INDEX IF NOT EXISTS idx_houses_address ON houses(address);
CREATE INDEX IF NOT EXISTS idx_houses_year ON houses(build_year);
CREATE INDEX IF NOT EXISTS idx_houses_apartments ON houses(apartments);
CREATE INDEX IF NOT EXISTS idx_houses_area ON houses(total_area);
CREATE INDEX IF NOT EXISTS idx_houses_floors ON houses(floors);
CREATE INDEX IF NOT EXISTS idx_users_login ON users(login);

-- Индекс для поиска дубликатов по адресу (case-insensitive)
CREATE INDEX IF NOT EXISTS idx_houses_address_lower ON houses(LOWER(address));

-- Функция для нормализации адреса в БД
CREATE OR REPLACE FUNCTION normalize_address(text)
RETURNS text AS $$
BEGIN
    RETURN lower(regexp_replace(regexp_replace($1, 
        '\s+', ' ', 'g'),  -- множественные пробелы
        '[.,]', '', 'g'));  -- точки и запятые
END;
$$ LANGUAGE plpgsql IMMUTABLE;

-- Индекс на нормализованный адрес
CREATE INDEX IF NOT EXISTS idx_houses_address_normalized ON houses(normalize_address(address));

-- Триггер для автоматического обновления времени
CREATE OR REPLACE FUNCTION update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.created_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Вставляем начальные данные
INSERT INTO users (login, password_hash, salt) VALUES 
('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'admin_salt_123'),
('user', '04f8996da763b7a969b1028ee3007569eaf3a635486ddab211d512c85b9df8fb', 'user_salt_456')
ON CONFLICT (login) DO NOTHING;

INSERT INTO houses (address, apartments, total_area, build_year, floors) VALUES
('ул. Ленина, д. 10', 24, 1560.50, 1985, 5),
('пр. Мира, д. 42', 48, 3200.00, 2005, 9),
('ул. Пушкина, д. 15', 12, 850.75, 1960, 3),
('ул. Советская, д. 7', 36, 2450.25, 1995, 7),
('ул. Гагарина, д. 33', 60, 4200.00, 2010, 12),
('ул. Кирова, д. 18', 18, 1200.00, 1972, 4),
('пр. Ленинградский, д. 89', 96, 7800.50, 2018, 16),
('ул. Садовая, д. 5', 6, 450.25, 1955, 2),
('ул. Молодежная, д. 22', 30, 2100.00, 2000, 6),
('ул. Заречная, д. 41', 42, 3150.75, 1990, 8)
ON CONFLICT DO NOTHING;

-- Права доступа
GRANT ALL PRIVILEGES ON TABLE users TO housing_user;
GRANT ALL PRIVILEGES ON TABLE houses TO housing_user;
GRANT USAGE, SELECT ON SEQUENCE users_id_seq TO housing_user;
GRANT USAGE, SELECT ON SEQUENCE houses_id_seq TO housing_user;

-- Статистика
SELECT 'База данных инициализирована успешно!' as message;
SELECT COUNT(*) as total_users FROM users;
SELECT COUNT(*) as total_houses FROM houses;
