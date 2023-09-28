#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libi2c-amaork
PKG_VERSION:=1.0.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=John Doe <john.doe@example.com>
PKG_LICENSE:=CC0-1.0

include $(INCLUDE_DIR)/package.mk

define Package/libi2c-amaork
	SECTION:=net
	# Select package by default
	#DEFAULT:=y
	CATEGORY:=Network
	TITLE:=Some other dummy application.
	# Feature FOO also needs libsodium:
	DEPENDS:=+librt +CONFIG_EXAMPLE2_ENABLE_FOO:libsodium
	URL:=https://www.example.com
	SUBMENU:=VPN
endef

define Package/libi2c-amaork/description
	Some example Programm called libi2c-amaork
endef

ifeq ($(CONFIG_EXAMPLE2_ENABLE_FOO),y)
MAKE_FLAGS += -DFOO
endif

ifeq ($(CONFIG_EXAMPLE2_ENABLE_BAR),y)
MAKE_FLAGS += -DBAR
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
# Nothing to do here for us.
# By default libi2c-amaork/src/Makefile will be used.
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/include/i2c $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/libi2c-amaork.{a,so*} $(1)/usr/lib/
endef

define Package/libi2c-amaork/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libi2c-amaork.{a,so*} $(1)/usr/lib
endef

$(eval $(call BuildPackage,libi2c-amaork))
