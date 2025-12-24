BEGIN;
DO $$
BEGIN
    DROP FUNCTION IF EXISTS add_house(TEXT, INTEGER, DECIMAL, INTEGER, INTEGER) CASCADE;
    DROP FUNCTION IF EXISTS update_house(INTEGER, TEXT, INTEGER, DECIMAL, INTEGER, INTEGER) CASCADE;
    DROP FUNCTION IF EXISTS delete_house(INTEGER) CASCADE;
    DROP FUNCTION IF EXISTS get_all_houses() CASCADE;
    DROP FUNCTION IF EXISTS search_houses(TEXT) CASCADE;
    DROP FUNCTION IF EXISTS get_houses_older_than(INTEGER) CASCADE;
    DROP FUNCTION IF EXISTS house_exists(TEXT) CASCADE;
    DROP FUNCTION IF EXISTS house_exists_except_id(TEXT, INTEGER) CASCADE; 
    DROP FUNCTION IF EXISTS add_user(TEXT, TEXT, TEXT) CASCADE;
    DROP FUNCTION IF EXISTS authenticate_user(TEXT, TEXT) CASCADE;
    DROP FUNCTION IF EXISTS get_user_by_login(TEXT) CASCADE;
    DROP FUNCTION IF EXISTS user_exists(TEXT) CASCADE;
EXCEPTION
    WHEN OTHERS THEN
        NULL;
END $$;
-- Таблицы 
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    login TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS houses (
    id SERIAL PRIMARY KEY,
    address TEXT NOT NULL,
    apartments INT NOT NULL CHECK (apartments > 0),
    total_area DECIMAL(10,2) NOT NULL CHECK (total_area > 0),
    build_year INT NOT NULL CHECK (build_year >= 1500 AND build_year <= EXTRACT(YEAR FROM CURRENT_DATE)),
    floors INT NOT NULL CHECK (floors > 0 AND floors <= 100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Индексы
CREATE INDEX IF NOT EXISTS idx_users_login ON users(login);
CREATE INDEX IF NOT EXISTS idx_houses_address ON houses(address);

-- 1. ДОМА 
CREATE OR REPLACE FUNCTION add_house(
    p_address TEXT,
    p_apartments INTEGER,
    p_total_area DECIMAL(10,2),
    p_build_year INTEGER,
    p_floors INTEGER
) RETURNS INTEGER AS $$
DECLARE
    new_id INTEGER;
BEGIN
    INSERT INTO houses (address, apartments, total_area, build_year, floors)
    VALUES (p_address, p_apartments, p_total_area, p_build_year, p_floors)
    RETURNING id INTO new_id;
    RETURN new_id;
EXCEPTION WHEN OTHERS THEN
    RETURN -1;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION update_house(
    p_id INTEGER,
    p_address TEXT,
    p_apartments INTEGER,
    p_total_area DECIMAL(10,2),
    p_build_year INTEGER,
    p_floors INTEGER
) RETURNS BOOLEAN AS $$
BEGIN
    UPDATE houses 
    SET address = p_address,
        apartments = p_apartments,
        total_area = p_total_area,
        build_year = p_build_year,
        floors = p_floors
    WHERE id = p_id;
    RETURN FOUND;
EXCEPTION WHEN OTHERS THEN
    RETURN FALSE;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION delete_house(p_id INTEGER) 
RETURNS BOOLEAN AS $$
BEGIN
    DELETE FROM houses WHERE id = p_id;
    RETURN FOUND;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_all_houses()
RETURNS TABLE(
    house_id INTEGER,
    house_address TEXT,
    house_apartments INTEGER,
    house_total_area DECIMAL(10,2),
    house_build_year INTEGER,
    house_floors INTEGER,
    house_created_at TIMESTAMP
) AS $$
BEGIN
    RETURN QUERY 
    SELECT id, address, apartments, total_area, build_year, floors, created_at
    FROM houses 
    ORDER BY id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION search_houses(p_search_query TEXT)
RETURNS TABLE(
    house_id INTEGER,
    house_address TEXT,
    house_apartments INTEGER,
    house_total_area DECIMAL(10,2),
    house_build_year INTEGER,
    house_floors INTEGER,
    house_created_at TIMESTAMP
) AS $$
BEGIN
    RETURN QUERY 
    SELECT id, address, apartments, total_area, build_year, floors, created_at
    FROM houses 
    WHERE address ILIKE '%' || p_search_query || '%'
    ORDER BY address;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_houses_older_than(p_years INTEGER)
RETURNS TABLE(
    house_id INTEGER,
    house_address TEXT,
    house_apartments INTEGER,
    house_total_area DECIMAL(10,2),
    house_build_year INTEGER,
    house_floors INTEGER,
    house_age INTEGER
) AS $$
BEGIN
    RETURN QUERY 
    SELECT id, address, apartments, total_area, build_year, floors,
           EXTRACT(YEAR FROM CURRENT_DATE) - build_year as age
    FROM houses 
    WHERE build_year < EXTRACT(YEAR FROM CURRENT_DATE) - p_years
    ORDER BY build_year;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION house_exists(p_address TEXT)
RETURNS BOOLEAN AS $$
DECLARE
    exists_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO exists_count
    FROM houses 
    WHERE address = p_address;
    RETURN exists_count > 0;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION house_exists_except_id(p_address TEXT, p_except_id INTEGER)
RETURNS BOOLEAN AS $$
DECLARE
    exists_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO exists_count
    FROM houses 
    WHERE address = p_address AND id != p_except_id;
    RETURN exists_count > 0;
END;
$$ LANGUAGE plpgsql;

-- 2. ПОЛЬЗОВАТЕЛИ 
CREATE OR REPLACE FUNCTION add_user(
    p_login TEXT,
    p_password_hash TEXT,
    p_salt TEXT
) RETURNS INTEGER AS $$
DECLARE
    new_id INTEGER;
BEGIN
    INSERT INTO users (login, password_hash, salt)
    VALUES (p_login, p_password_hash, p_salt)
    RETURNING id INTO new_id;
    RETURN new_id;
EXCEPTION WHEN OTHERS THEN
    RETURN -1;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_user_by_login(p_login TEXT)
RETURNS TABLE(
    user_id INTEGER,
    user_login TEXT,
    user_password_hash TEXT,
    user_salt TEXT,
    user_created_at TIMESTAMP
) AS $$
BEGIN
    RETURN QUERY 
    SELECT id, login, password_hash, salt, created_at
    FROM users 
    WHERE login = p_login;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION user_exists(p_login TEXT)
RETURNS BOOLEAN AS $$
DECLARE
    exists_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO exists_count
    FROM users 
    WHERE login = p_login;
    RETURN exists_count > 0;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION authenticate_user(
    p_login TEXT,
    p_password_hash TEXT
) RETURNS TABLE(
    user_id INTEGER,
    is_authenticated BOOLEAN,
    user_login TEXT
) AS $$
DECLARE
    v_user_id INTEGER;
    v_authenticated BOOLEAN;
BEGIN
    SELECT id INTO v_user_id
    FROM users 
    WHERE login = p_login AND password_hash = p_password_hash;
    v_authenticated := (v_user_id IS NOT NULL);
    RETURN QUERY 
    SELECT COALESCE(v_user_id, 0), v_authenticated, p_login;
END;
$$ LANGUAGE plpgsql;
COMMIT;
