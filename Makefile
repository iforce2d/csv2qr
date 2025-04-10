MKDIR := mkdir -p
SRC_DIR := .
OBJ_DIR := ./obj
SRC_FILES := $(wildcard ./*.cpp wildcard)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS := -Wall
LDFLAGS := -lZXing -lhpdf -lz -lpng

all: csv2qr

csv2qr: $(OBJ_FILES)
	g++ -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@ $(MKDIR) $(@D)
	g++ $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)

