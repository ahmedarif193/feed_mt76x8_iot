include $(TOPDIR)/rules.mk

PKG_NAME:=sooall-pwm
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

# Kernel module definition
define KernelPackage/sooall-pwm
  SUBMENU:=Other modules
  TITLE:=Sooall PWM Driver
  FILES:=$(PKG_BUILD_DIR)/mt7628-pwm.ko
  AUTOLOAD:=$(call AutoLoad,30,mt7628-pwm)
endef

define KernelPackage/sooall-pwm/description
  Kernel module for controlling Sooall PWM
endef

# Userspace application definition
define Package/sooall-pwm-app
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Sooall PWM Userspace Utility
endef

define Package/sooall-pwm-app/description
  Userspace utility for controlling Sooall PWM
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

define Package/sooall-pwm-app/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sooall_pwm_app $(1)/usr/bin/
endef

$(eval $(call KernelPackage,sooall-pwm))
$(eval $(call BuildPackage,sooall-pwm-app))
