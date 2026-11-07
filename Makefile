# Build on macOS (Xcode CLT) or via the GitHub Actions workflow in .github/workflows/build.yml
SDK     := $(shell xcrun --sdk iphoneos --show-sdk-path)
OUT     := build/XantaMenu.dylib
SRC     := $(wildcard src/*.cpp) $(wildcard src/*.mm)

.PHONY: all clean
all: $(OUT)

$(OUT): $(SRC) $(wildcard src/*.hpp)
	@mkdir -p build
	xcrun --sdk iphoneos clang++ -arch arm64 -isysroot $(SDK) -miphoneos-version-min=14.0 \
	  -std=gnu++20 -fobjc-arc -O2 -fvisibility=hidden -dynamiclib \
	  -install_name @rpath/XantaMenu.dylib \
	  -framework UIKit -framework Foundation -framework QuartzCore \
	  $(SRC) -o $@

clean:
	rm -rf build
