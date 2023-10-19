include $(TOPDIR)/rules.mk

# Package details
PKG_NAME:=mt76x8-pin-ctrl
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=MEDIATEK mt76xx Pins Control Utility
  MAINTAINER:=Ahmed ARIF <arif193@gmail.com>
endef

define Package/$(PKG_NAME)/description
  Utility to control pinmux on mt76x8.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mt76x8-pin-ctrl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
