#############################################################
#
# onda app
#
#############################################################
ONDA_VERSION = 1.0
ONDA_OVERRIDE_SRCDIR = $(TOPDIR)/onda
ONDA_INSTALL_TARGET = YES
ONDA_DEPENDENCIES = linux26

define ONDA_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" -C $(LINUX26_DIR) M=$(@D) modules
endef

define ONDA_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/onda.ko $(TARGET_DIR)/lib/modules/
endef

$(eval $(call GENTARGETS))
