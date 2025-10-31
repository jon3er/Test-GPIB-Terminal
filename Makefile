

#	Build Directory
BUILD_DIR_RELEASE 	= bin/Release
BUILD_DIR_DEBUG 	= bin/Debug

#	Object Directory
OBJS_DIR_RELEASE 	= obj/Release
OBJS_DIR_DEBUG 		= obj/Debug

#	Target Directory
TARGET_RELEASE 		= $(BUILD_DIR_RELEASE)/$(TARGET)
TARGET_DEBUG 		= $(BUILD_DIR_DEBUG)/$(TARGET)

#	Compiler
CXX 				= g++

#	Compiler flags
CXXFLAGS 			= -Wall -I. `wx-config --cxxflags`

#	Linker Flags
LDFLAGS 			= -L. -lftd2xx -Wl,-rpath=. `wx-config --libs`

#	Debug Flags
CXXFLAGS_DEBUG 		= -g -DDEBUG

#	executable name
TARGET 				= Test_GPIB_Terminal

#	source files
SRCS 				= Main.cpp fkt_GPIB.cpp fkt_d2xx.cpp

#	object files
OBJS 				= $(SRCS:.cpp=.o)

OBJS_DEBUG 			=$(patsubst %.cpp,$(OBJS_DIR_DEBUG)/%.o,$(SRCS))
OBJS_RELEASE 		=$(patsubst %.cpp,$(OBJS_DIR_RELEASE)/%.o,$(SRCS))

#	---Rules----

#	Default target
all: 		Release

#	Release target
Release:	$(TARGET_RELEASE)

#	Debug target
Debug: 		$(TARGET_DEBUG)

#	Release Build
#		Release Linking
$(TARGET_RELEASE): $(OBJS_RELEASE)
	@echo "Linking Release executable: $@"
	@mkdir -p $(BUILD_DIR_RELEASE)
	$(CXX) -o $@ $^ $(LDFLAGS)
#		Release Compiling
$(BUILD_DIR_RELEASE)%.o: %.cpp
	@echo "Compiling Release: $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@


#	Debug build
#		Debug Linking
$(TARGET_DEBUG): $(OBJS_DEBUG)
	@echo "Linking Debug executable: $@"
	@mkdir -p $(BUILD_DIR_DEBUG)
	$(CXX) -o $@ $^ $(LDFLAGS)
#		Debug Compiling
$(BUILD_DIR_DEBUG)%.o: %.cpp
	@echo "Compiling Debug: $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DEBUG) $< -o $@

#	Clean target
clean:
	rm -f bin obj

#	Phony targets
.PHONY: all Release Debug clean
