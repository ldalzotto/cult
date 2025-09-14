#include "convert.h"

// Helper function to convert i8 to string
void convert_i8_to_string(i8 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u8 to string
void convert_u8_to_string(u8 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i16 to string
void convert_i16_to_string(i16 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u16 to string
void convert_u16_to_string(u16 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i32 to string
void convert_i32_to_string(i32 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u32 to string
void convert_u32_to_string(u32 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i64 to string
void convert_i64_to_string(i64 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u64 to string
void convert_u64_to_string(u64 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert iptr to string
void convert_iptr_to_string(iptr value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert uptr to string
void convert_uptr_to_string(uptr value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
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
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}
