#include "convert.h"

// Helper function to convert i8 to string
char* convert_i8_to_string(i8 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Handle negative numbers
        int is_negative = 0;
        i8 temp = value;
        if (temp < 0) {
            is_negative = 1;
            temp = -temp;
        }

        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Add negative sign if needed
        if (is_negative) {
            temp_buffer[idx++] = '-';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert u8 to string
char* convert_u8_to_string(u8 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        u8 temp = value;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert i16 to string
char* convert_i16_to_string(i16 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Handle negative numbers
        int is_negative = 0;
        i16 temp = value;
        if (temp < 0) {
            is_negative = 1;
            temp = -temp;
        }

        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Add negative sign if needed
        if (is_negative) {
            temp_buffer[idx++] = '-';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert u16 to string
char* convert_u16_to_string(u16 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        u16 temp = value;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert i32 to string
char* convert_i32_to_string(i32 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Handle negative numbers
        int is_negative = 0;
        i32 temp = value;
        if (temp < 0) {
            is_negative = 1;
            temp = -temp;
        }

        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Add negative sign if needed
        if (is_negative) {
            temp_buffer[idx++] = '-';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert u32 to string
char* convert_u32_to_string(u32 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[32];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert digits
        char temp_buf[32];
        int temp_idx = 0;
        u32 temp = value;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert i64 to string
char* convert_i64_to_string(i64 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[64];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Handle negative numbers
        int is_negative = 0;
        i64 temp = value;
        if (temp < 0) {
            is_negative = 1;
            temp = -temp;
        }

        // Convert digits
        char temp_buf[64];
        int temp_idx = 0;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Add negative sign if needed
        if (is_negative) {
            temp_buffer[idx++] = '-';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert u64 to string
char* convert_u64_to_string(u64 value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[64];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert digits
        char temp_buf[64];
        int temp_idx = 0;
        u64 temp = value;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert iptr to string
char* convert_iptr_to_string(iptr value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[64];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Handle negative numbers
        int is_negative = 0;
        iptr temp = value;
        if (temp < 0) {
            is_negative = 1;
            temp = -temp;
        }

        // Convert digits
        char temp_buf[64];
        int temp_idx = 0;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Add negative sign if needed
        if (is_negative) {
            temp_buffer[idx++] = '-';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert uptr to string
char* convert_uptr_to_string(uptr value, stack_alloc* alloc) {
    int idx = 0;
    char temp_buffer[64];

    // Handle special case of 0
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert digits
        char temp_buf[64];
        int temp_idx = 0;
        uptr temp = value;
        while (temp > 0) {
            temp_buf[temp_idx++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}

// Helper function to convert pointer to string
char* convert_pointer_to_string(void* ptr, stack_alloc* alloc) {
    // Convert pointer to hexadecimal string
    uptr value = (uptr)ptr;
    int idx = 0;
    char temp_buffer[32];

    temp_buffer[idx++] = '0';
    temp_buffer[idx++] = 'x';

    // Handle special case of NULL pointer
    if (value == 0) {
        temp_buffer[idx++] = '0';
    } else {
        // Convert to hexadecimal (at least 8 digits for consistency)
        char temp_buf[32];
        int temp_idx = 0;

        // Convert digits
        while (value > 0) {
            int digit = value % 16;
            temp_buf[temp_idx++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            value /= 16;
        }

        // Ensure at least 8 digits (pad with zeros if needed)
        while (temp_idx < 8) {
            temp_buf[temp_idx++] = '0';
        }

        // Reverse the digits
        for (int i = temp_idx - 1; i >= 0; i--) {
            temp_buffer[idx++] = temp_buf[i];
        }
    }

    // Allocate memory and copy
    char* result = (char*)sa_alloc(alloc, idx + 1);
    for (int i = 0; i < idx; i++) {
        result[i] = temp_buffer[i];
    }
    result[idx] = '\0';

    return result;
}
