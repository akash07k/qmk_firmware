ifeq ($(strip $(KEYMAP)), personal)
    # See keymaps/personal/readme.md before changing this keymap-local filtering.
    PERSONAL_RGB_DIR := $(TOP_DIR)/keyboards/keychron/common/rgb

    PERSONAL_SRC_SNAPSHOT := $(filter-out $(PERSONAL_RGB_DIR)/%,$(SRC))
    SRC = $(PERSONAL_SRC_SNAPSHOT)

    PERSONAL_OPT_DEFS_SNAPSHOT := $(filter-out \
        -DKEYCHRON_RGB_ENABLE \
        -DRETAIL_DEMO_ENABLE \
        -DVIA_INSECURE \
        -DRGB_MATRIX_BRIGHTNESS_TURN_OFF_VAL=48, \
        $(OPT_DEFS))
    OPT_DEFS = $(PERSONAL_OPT_DEFS_SNAPSHOT) -DKEYCHRON_RGB_EEPROM_COMPAT_SIZE=518

    PERSONAL_VPATH_SNAPSHOT := $(filter-out $(PERSONAL_RGB_DIR),$(VPATH))
    VPATH = $(PERSONAL_VPATH_SNAPSHOT)
endif
