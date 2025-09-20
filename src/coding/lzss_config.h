#ifndef LZSS_CONFIG_H
#define LZSS_CONFIG_H

#include "primitive.h"

typedef u8 lzss_match_size_t;
typedef u16 lzss_window_size_t;

/**
 * @struct lzss_config
 * @brief Configuration parameters for LZSS compression.
 *
 * This structure holds the parameters that control the behavior of the LZSS compression algorithm,
 * including minimum and maximum match sizes and the maximum window size for searching matches.
 */
typedef struct {
    lzss_match_size_t match_size_min;    /**< Minimum size of a match to be considered for compression (in bytes). */
    lzss_match_size_t match_size_max;    /**< Maximum size of a match that can be encoded (in bytes). */
    lzss_window_size_t window_size_max;  /**< Maximum size of the sliding window for searching matches (in bytes). */
} lzss_config;

#endif /* LZSS_CONFIG_H */
