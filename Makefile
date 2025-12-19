# Makefile
.PHONY: help build run stop clean test db-shell logs

# Цвета для вывода
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
NC = \033[0m # No Color

help:
	@echo "$(GREEN)Доступные команды для HousingFund:$(NC)"
	@echo ""
	@echo "  $(YELLOW)make build$(NC)    - Собрать Docker образ"
	@echo "  $(YELLOW)make run$(NC)      - Запустить приложение и БД"
	@echo "  $(YELLOW)make stop$(NC)     - Остановить все контейнеры"
	@echo "  $(YELLOW)make clean$(NC)    - Удалить все контейнеры и образы"
	@echo "  $(YELLOW)make test$(NC)     - Запустить тесты"
	@echo "  $(YELLOW)make db-shell$(NC) - Подключиться к shell БД"
	@echo "  $(YELLOW)make logs$(NC)     - Показать логи приложения"
	@echo "  $(YELLOW)make status$(NC)   - Показать статус контейнеров"
	@echo "  $(YELLOW)make rebuild$(NC)  - Пересобрать и запустить заново"

build:
	@echo "$(GREEN)Сборка Docker образа...$(NC)"
	docker-compose build --no-cache

run:
	@echo "$(GREEN)Запуск системы...$(NC)"
	@if [ "$(shell uname)" = "Linux" ]; then \
		echo "$(YELLOW)Для Linux: настройка X11...$(NC)"; \
		xhost +local:docker; \
	fi
	docker-compose up -d
	@echo "$(GREEN)✅ Система запущена!$(NC)"
	@make status

stop:
	@echo "$(YELLOW)Остановка системы...$(NC)"
	docker-compose down

clean:
	@echo "$(RED)Полная очистка...$(NC)"
	docker-compose down -v --rmi all --remove-orphans
	@echo "$(GREEN)✅ Все очищено!$(NC)"

test:
	@echo "$(GREEN)Запуск тестов...$(NC)"
	docker-compose run --rm housing-app /app/HousingFund --test

db-shell:
	@echo "$(GREEN)Подключение к PostgreSQL...$(NC)"
	docker exec -it housing_postgres psql -U housing_user -d housing_fund

logs:
	@echo "$(GREEN)Просмотр логов приложения:$(NC)"
	docker-compose logs -f housing-app

status:
	@echo "$(GREEN)Статус контейнеров:$(NC)"
	@docker-compose ps

rebuild: stop build run
	@echo "$(GREEN)✅ Пересборка завершена!$(NC)"
