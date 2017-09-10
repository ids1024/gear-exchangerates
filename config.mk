NAME = watchtest
PKGNAME = org.example.$(NAME)
VERSION = 1.0.0
ARCH = arm
PACKAGE = $(PKGNAME)-$(VERSION)-$(ARCH).tpk
ROOTSTRAP = wearable-2.3.2-device.core
SECURITY_PROFILE = ids1024
SRC = tizen-manifest.xml inc/watchtest.h src/watchtest.c
