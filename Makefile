.PHONY: help build build-debug run clean migrate-up migrate-down migrate-new migrate-status db-shell cmake-generate test test-verbose build-tests

# === Настройки ===
CMAKE_BUILD_TYPE ?= Release
BUILD_DIR := build
DOCKER_COMPOSE := docker compose

# === Справка ===
help:
	@echo "🔧 Доступные команды:"
	@echo "  make cmake-generate  - сгенерировать файлы CMake (cmake -S . -B build)"
	@echo "  make build           - собрать проект (Release)"
	@echo "  make build-debug     - собрать проект (Debug)"
	@echo "  make test            - запустить тесты"
	@echo "  make test-verbose    - запустить тесты с подробным выводом"
	@echo "  make run             - запустить приложение локально"
	@echo "  make clean           - очистить артефакты сборки"
	@echo ""
	@echo "🗄️  Миграции (dbmate):"
	@echo "  make migrate-up      - применить миграции"
	@echo "  make migrate-down    - откатить последнюю миграцию"
	@echo "  make migrate-new имя_миграции - создать новую миграцию"
	@echo "  make migrate-status  - показать статус миграций"
	@echo ""
	@echo "🐳 Docker:"
	@echo "  make docker-build    - собрать образ приложения"
	@echo "  make docker-up       - запустить всё через docker compose"
	@echo "  make docker-down     - остановить сервисы"
	@echo "  make test-docker     - запустить тесты в docker"
	@echo "  make test-docker-up  - запустить тесты в docker c поднятием всех сервисов"
	@echo "  make test-docker-clean  - очистить тестовые артефакты"

# === CMake ===
cmake-generate:
	@echo "🔧 Генерация CMake..."
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

build: cmake-generate
	@echo "🚀 Сборка (Release)..."
	cmake --build $(BUILD_DIR) --config $(CMAKE_BUILD_TYPE) -j$$(nproc)

build-debug: CMAKE_BUILD_TYPE = Debug
build-debug: cmake-generate
	@echo "🐛 Сборка (Debug)..."
	cmake --build $(BUILD_DIR) --config $(CMAKE_BUILD_TYPE) -j$$(nproc)

test:
	@echo "🔧 Генерация CMake для тестов (Debug)..."
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON
	@echo "🔨 Сборка тестов..."
	cmake --build $(BUILD_DIR) --target empl_back_tests -j$$(nproc)
	@echo "🧪 Запуск тестов..."
	cd $(BUILD_DIR) && ctest --output-on-failure

test-verbose: cmake-generate
	@echo "🧪 Запуск тестов (подробно)..."
	cd $(BUILD_DIR) && ctest --verbose

# === Сборка тестов ===
build-tests: cmake-generate
	@echo "🔨 Сборка тестов..."
	cmake --build $(BUILD_DIR) --target empl_back_tests --config $(CMAKE_BUILD_TYPE)

# === Запуск локально ===
run:
	@echo "▶️  Запуск приложения..."
	./$(BUILD_DIR)/empl_back

# === Очистка ===
clean:
	@echo "🧹 Очистка..."
	rm -rf $(BUILD_DIR)
	find . -name "CMakeCache.txt" -delete
	find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true

# === Миграции (dbmate) ===
migrate-up:
	$(DOCKER_COMPOSE) run --rm dbmate-migrate up

migrate-down:
	$(DOCKER_COMPOSE) run --rm dbmate-migrate rollback

migrate-new:
	$(DOCKER_COMPOSE) run --rm \
		--user "$(shell id -u):$(shell id -g)" \
		-v $(PWD)/db:/db \
		dbmate-migrate new $(filter-out $@,$(MAKECMDGOALS))

migrate-status:
	$(DOCKER_COMPOSE) run --rm dbmate-migrate status

# === Docker ===
docker-build:
	$(DOCKER_COMPOSE) build app

docker-up:
	$(DOCKER_COMPOSE) up -d

docker-start:
	$(DOCKER_COMPOSE) start

docker-stop:
	$(DOCKER_COMPOSE) stop

docker-down:
	$(DOCKER_COMPOSE) down

# === Тесты в Docker ===
test-docker:
	@echo "🐳 Запуск тестов в Docker..."
	$(DOCKER_COMPOSE) build app-tests
	$(DOCKER_COMPOSE) run --rm app-tests

test-docker-up:
	@echo "🐳 Запуск тестов с поднятием всех сервисов..."
	$(DOCKER_COMPOSE) up --abort-on-container-exit app-tests

test-docker-clean:
	@echo "🧹 Очистка тестовых артефактов..."
	$(DOCKER_COMPOSE) down -v

# === Отладка БД ===
db-shell:
	$(DOCKER_COMPOSE) exec postgres psql -U emplcpp -d empldb