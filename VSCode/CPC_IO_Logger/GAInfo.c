


#include "GAInfo.h"

    uint32_t get_GA_FUNCTION_SELECT(uint32_t rawData) {
        return (rawData & GA_FUNCTION_SELECT_MASK) >> GA_FUNCTION_SELECT_SHIFT;
    }

    uint32_t get_GA_PEN(uint32_t rawData) {
        return (rawData & GA_SELECT_PEN_MASK);
    }

    uint32_t get_GA_COLOUR(uint32_t rawData) {
        return (rawData & GA_SELECT_COLOUR_MASK);
    }

    uint32_t get_GA_MODE(uint32_t rawData) {
        return (rawData & GA_RMR_MODE_MASK);
    }

    uint32_t get_GA_INTERRUPT_CONTROL(uint32_t rawData) {
        return (rawData & GA_RMR_INTERRUPT_CONTROL_MASK) >> GA_RMR_INTERRUPT_CONTROL_MASK_SHIFT;
    }

    uint32_t get_GA_UPPER_ROM_DISABLE(uint32_t rawData) {
        return (rawData & GA_RMR_UPPER_ROM_DISABLE_MASK) >> GA_RMR_UPPER_ROM_DISABLE_MASK_SHIFT;
    }

    uint32_t get_GA_LOWER_ROM_DISABLE(uint32_t rawData) {
        return (rawData & GA_RMR_LOWER_ROM_DISABLE_MASK) >> GA_RMR_LOWER_ROM_DISABLE_MASK_SHIFT;
    }

    uint32_t get_GA_BANK_NUMBER(uint32_t rawData) {
        return (rawData & GA_MMR_BANK_NUMBER_MASK) >> GA_MMR_BANK_NUMBER_MASK_SHIFT;
    }

    uint32_t get_GA_RAM_CONFIG(uint32_t rawData) {
        return (rawData & GA_MMR_RAM_CONFIG_MASK);
    }

    bool is_RMR2 (uint32_t rawData) {
        return (rawData & GA_RMR2_BIT_5_MASK) >> GA_RMR2_BIT_5_MASK_SHIFT;
    }

    uint32_t get_GA_RMR2_ADDR_MODE(uint32_t rawData) {
        return (rawData & GA_RMR2_ADDR_MODE_MASK) >> GA_RMR2_ADDR_MODE_SHIFT;
    }

    uint32_t get_GA_RMR2_ROM_NUM(uint32_t rawData) {
        return (rawData & GA_RMR2_ROM_NUM_MASK);
    }