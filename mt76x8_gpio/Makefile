include $(TOPDIR)/rules.mk

PKG_NAME:=mt76x8_gpio
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Ahmed ARIF <arif193@gmail.com>
PKG_LICENSE:=GPL3

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mt76x8_gpio
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GPIO Control Utility for MT76x8
  DEPENDS:=
endef

define Package/mt76x8_gpio/description
  Utility for controlling GPIO pins on MT76x8 devices.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/mt76x8_gpio/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt76x8_gpio $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mt76x8_gpio))
