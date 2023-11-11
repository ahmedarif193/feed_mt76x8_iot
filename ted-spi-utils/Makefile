#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ted-spi-utils
PKG_VERSION:=1.0.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Ahmed ARIF <arif193@gmail.com>
PKG_LICENSE:=GPL3
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ted-spi-utils
	SECTION:=net
	# Select package by default
	DEFAULT:=y
	CATEGORY:=Network
	TITLE:=Some other dummy application.
	URL:=https://www.example.com
	SUBMENU:=Utils
	DEPENDS:= +libstdcpp
endef

define Package/ted-spi-utils/description
	Some example Programm called ted-spi-utils
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/ted-spi-utils/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ted-spi-utils $(1)/usr/bin/
endef


$(eval $(call BuildPackage,ted-spi-utils))
