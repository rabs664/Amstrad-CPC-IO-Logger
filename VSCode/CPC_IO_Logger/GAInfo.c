


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

    bool is_RMR2 (uint32_t rawData) {
        return (rawData & GA_RMR2_BIT_5_MASK) >> GA_RMR2_BIT_5_MASK_SHIFT;
    }