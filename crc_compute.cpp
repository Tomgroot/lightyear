//
// Created by Tom Groot on 25/02/2025.
//

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>

// Castagnoli polynomial
const uint32_t polynomial = 0x1EDC6F41;

static uint32_t LUT[256];

// inspired by https://bitbucket.org/spandeygit/crc32v/src/master/tablegen_8bit.c
void initLUT() {

    for (int j = 0; j < 256; j++) {
        uint32_t crc = 0x00000000;
        uint8_t b = j;
        crc = b << 24;
        for (int k = 0; k < 8; k++) {
            // if the most significant bit is 1, XOR with the polynomial
            if ((crc & 0x80000000) != 0) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
        LUT[j] = crc;
    }
}

uint32_t computeCRC(uint8_t bytes[8]) {
    uint32_t crc = 0x00000000;

    for (int j = 0; j < 8; j++) {
        uint8_t b = bytes[j];
        // Making use of the fact that crc(a+b) = crc(a) + crc(b) =
        // crc(0xabcdefgh) = crc(0xab000000) + crc(0x00cdefgh)
        //                 = crc(0xab000000) + (0xcdefgh<<8)
        b ^= (crc >> 24) & 0xFF;
        crc = (crc << 8) ^ LUT[b];
    }

    return crc;
}

bool isValid(uint8_t buffer[12]) {
    // Unpacking the message and test whether the calculated CRC is the same.
    uint8_t message[8];
    memcpy(message, buffer, 8);
    uint32_t crc;
    memcpy(&crc, buffer + 8, 4);

    uint32_t computedCRC = computeCRC(message);
    return computedCRC == crc;
}

int main() {
    // Can be done at start up only.
    initLUT();

    uint8_t message[8] = { 'h', 'i', 'i', 'a', 'm', 't', 'o', 'm' };
    // calculating the time it takes to compute the CRC
    auto start = std::chrono::high_resolution_clock::now();
    uint32_t crc = computeCRC(message);
    auto stop = std::chrono::high_resolution_clock::now();
    auto diff = stop - start;
    std::cout << "crc: 0x" << std::hex << crc << std::endl;
    std::cout << "calculated in: " << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl << std::endl;

    uint8_t buffer[8 + 4];
    memcpy(buffer, message, 8);
    memcpy(buffer + 8, &crc, 4);

    // In a real world scenario also check whether the length of the received package is what we expect it to be.
    // UART handles the parity and stop bits and hides them from our implementation.

    std::cout << "correct transmission incoming..." << std::endl;
    // Without any errors...
    std::cout << "is valid: " << isValid(buffer) << std::endl << std::endl;

    uint8_t oneBitFlipBuffer[12];
    memcpy(oneBitFlipBuffer, buffer, 12);
    std::cout << "one bit flipped transmission incoming..." << std::endl;
    // If we flip a bit it should detect it's invalid...
    oneBitFlipBuffer[0] ^= 0xF;
    std::cout << "is valid: " << isValid(oneBitFlipBuffer) << std::endl << std::endl;

    uint8_t allBitsFlipBuffer[12];
    memcpy(allBitsFlipBuffer, buffer, 12);
    std::cout << "all bits flipped transmission incoming..." << std::endl;
    // Flip everything in the message
    for (int i = 0; i < 12; i++) {
        allBitsFlipBuffer[i] = allBitsFlipBuffer[i] ^ 0xFF;
    }
    std::cout << "is valid: " << isValid(allBitsFlipBuffer) << std::endl << std::endl;

    uint8_t halfBitsFlipBuffer[12];
    memcpy(halfBitsFlipBuffer, buffer, 12);
    std::cout << "half bits flipped transmission incoming..." << std::endl;
    // flip half
    for (int i = 3; i < 9; i++) {
        halfBitsFlipBuffer[i] = halfBitsFlipBuffer[i] ^ 0xFF;
    }
    std::cout << "is valid: " << isValid(allBitsFlipBuffer) << std::endl << std::endl;

    uint8_t correctBuffer[12];
    memcpy(correctBuffer, buffer, 12);
    std::cout << "again correct transmission incoming..." << std::endl;
    std::cout << "is valid: " << isValid(correctBuffer) << std::endl << std::endl;
}