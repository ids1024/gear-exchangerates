NAME = watchtest
PKGNAME = org.example.$(NAME)-1.0.0
SRC = tizen-manifest.xml inc/watchtest.h src/watchtest.c

debug: Debug/$(NAME)

release: Release/$(NAME)

package-debug: Debug/$(PKGNAME).tpk

package-release: Release/$(PKGNAME)-arm.tpk

install-debug: Debug/org.example.$(NAME)-1.0.0-arm.tpk
	tizen install -n $(PKGNAME)-arm.tpk -- $(PWD)/Debug

install-release: Release/org.example.$(NAME)-1.0.0-arm.tpk
	tizen install -n $(PKGNAME)-arm.tpk -- $(PWD)/Release

Debug/org.example.$(NAME)-1.0.0-arm.tpk: Debug/$(NAME)
	tizen package -t tpk -s ids1024 -- $(PWD)/Debug

Release/org.example.$(NAME)-1.0.0-arm.tpk: Release/$(NAME)
	tizen package -S on -t tpk -s ids1024 -- $(PWD)/Release

Debug/$(NAME): $(SRC)
	tizen build-native -r wearable-3.0-device.core -C Debug

Release/$(NAME): $(SRC)
	tizen build-native -r wearable-3.0-device.core -C Release

clean:
	tizen clean

.PHONY: debug release pakage-debug package-release install-debug install-release clean
