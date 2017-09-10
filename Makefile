include config.mk

PACKAGE = $(PKGNAME)-$(VERSION)-$(ARCH).tpk

all: package-debug

debug: Debug/$(NAME)

release: Release/$(NAME)

package-debug: Debug/$(PACKAGE)

package-release: Release/$(PACKAGE)

install-debug: Debug/$(PACKAGE)
	tizen install -n $(PACKAGE) -- ./Debug

install-release: Release/$(PACKAGE)
	tizen install -n $(PACKAGE) -- ./Release

Debug/$(PACKAGE): Debug/$(NAME)
	tizen package -t tpk -s $(SECURITY_PROFILE) -- ./Debug

Release/$(PACKAGE): Release/$(NAME)
	tizen package -S on -t tpk -s $(SECURITY_PROFILE) -- ./Release

Debug/$(NAME): $(SRC)
	tizen build-native -r $(ROOTSTRAP) -C Debug

Release/$(NAME): $(SRC)
	tizen build-native -r $(ROOTSTRAP) -C Release

clean:
	tizen clean

.PHONY: debug release pakage-debug package-release install-debug install-release clean
