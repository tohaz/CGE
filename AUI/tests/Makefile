CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20
INCLUDE_PATH = ../
CPPFLAGS = -I$(INCLUDE_PATH)
LDFLAGS = -Llib -L../Custom/
LIBS = -lCGLib -lX11 -lX11-xcb -lxcb -lxcb-cursor

# --- ДОБАВЛЕНО: Путь к файлу библиотеки ---
# Предположим, она называется libCGLib.a и лежит в ../Custom/
CGLIB_PATH = ../Custom/libCGLib.a 

SRC_DIR = ./
BUILD_DIR = ./bin/

SRCS = $(wildcard $(SRC_DIR)*.cpp)
EXES = $(patsubst $(SRC_DIR)%.cpp,$(BUILD_DIR)%,$(SRCS))

all: $(EXES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Правило для вызова make в родительской директории
# .PHONY здесь важен, чтобы команда запускалась, но само правило 
# не мешало проверке даты файла библиотеки ниже
.PHONY: build_external_libs
build_external_libs:
	$(MAKE) -C ..

# --- ИЗМЕНЕНО: Добавлена зависимость $(CGLIB_PATH) ---
$(BUILD_DIR)%: $(SRC_DIR)%.cpp $(CGLIB_PATH) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

# Опционально: если вы хотите, чтобы при запуске этого Makefile 
# сначала проверялась сборка библиотеки в родительской папке
$(CGLIB_PATH): build_external_libs
	@: # Пустая команда, просто чтобы сослаться на цель выше

.PHONY: clean all
clean:
	rm -rf $(BUILD_DIR)