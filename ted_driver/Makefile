include $(TOPDIR)/rules.mk

PKG_NAME:=ted-driver
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

# Kernel module definition
define KernelPackage/ted-driver
  SUBMENU:=Other modules
  TITLE:=Sooall PWM Driver
  FILES:=$(PKG_BUILD_DIR)/ted-drv.ko
  AUTOLOAD:=$(call AutoLoad,30,ted-drv)
endef

define KernelPackage/ted-driver/description
  Kernel module for controlling trailling edge dimmer
endef

# Userspace application definition
define Package/ted-driver-app
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Sooall PWM Userspace Utility
endef

define Package/ted-driver-app/description
  Userspace utility for controlling trailling edge dimmer
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Package/ted-driver-app/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sooall_pwm_app $(1)/usr/bin/
endef

$(eval $(call KernelPackage,ted-driver))
$(eval $(call BuildPackage,ted-driver-app))
