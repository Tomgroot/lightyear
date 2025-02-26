# Lightyear CRC

In the compiled binary `crc_compute`, a 32 CRC is calculated and appended to the
message "hiiamtom". It simulates multiple transmission of the message with the appended
CRC. First a correct transmission, second a single bit flipped transmission, third a transmission
with a burst of errors where all bits are flipped, fourth a transmission with half of bits flipped, 
and last a correct transmission again. The results are printed in the console.

# Usage

- Build: the source code can be found in `crc_compute.cpp`. Compile `crc_compute` using `g++ crc_compute.cpp -o crc_compute`.
- Run: a precompiled binary can be found at `crc_compute`.
- Test: the `main` function of `crc_compute` contains 5 test situations of which the result will be echoed to the console.

# CRC algorithm selection & justification

To decide what CRC algorithm to use, we need to consider three important factors:

- Error-detection capability
- Computational cost
- Implementation complexity

The polynomial of the CRC influences the three factors. The polynomial length should satisfy a very robust and should be strictly reliable in the harsh electrical environment 
and strict reliability requirements in automotive applications.

## Error detection capability

When we have a polynomial of length `n`, CRC always detects single error burst 
not longer than `n` bits and the fraction of all longer error bursts is approximately `(1 - 2^(-n))`.

In a worse case scenario the Bit Error Rate (BER) is 10^–04
(average is 10^-05 according to https://support.transcelestial.com/support/solutions/articles/51000324695-bit-error-rates-what-is-ber-and-what-is-a-good-ber-)
Lets calculate an approximation of the errors per seconds: 10^-4 * 921600 = 92.16 errors second

Approximation based on intuition, so has to be confirmed but:
- Lets assume the probability of errors with length (bits) of 17 - 32 bits is very roughly 20%
- Lets assume the probability of errors with length (bits) of 33 - 64 bits is very roughly 5%

92.16 * 0.2 = 18.432 errors longer than 16 bit per second \
92.16 * 0.05 = 4.6 errors longer than 32 bit per second

16 bit CRC = `1-2^(-16)` = 99.9984% get detected of errors longer than 16 bit = 0.0016% undetected = 0.000294912 undetected errors per second \
so it takes on average 56.5 minutes for an undetected error \
32 bit CRC = `1-2^(-32)` = 99.999999976% get detected of errors longer than 32 bit = `2.4 * 10^(-10) = 1.104 * 10^(-9)` undetected errors per second \
so it takes on average 28.7 years for an undetected error

Therefore, in the harsh electrical environment and strict reliability requirements in automotive applications, 
the 32 bit CRC is more reliable over the 16 bit variant for detecting data corruption during transmission.
Using a 64 bit CRC would probably not be needed due to 32 bit being reliably enough in worst case BER.

## Computational costs

Let's calculate how fast the CRC calculation should at least be. Messages are send 921.600 bits per seconds.
Each character has 8 bits + 1 stop bit + 1 parity bit. A payload of 8 bytes message = 10 * 8 = 80 bits.
921.600 bits per second / 80 = 11520 messages per second.
1000 ms / 11520 = 0.09ms.
So the CRC should be calculated within at least 0.09ms for a single message.

Longer polynomials require more computations per bit. A 32 bit microcontroller can handle the 32 CRC algorithms.
Aside from the CRC length, a table look up can also be done instead of a bitwise CRC computation to improve speeds especially
given the high baud rate and the less speedy microprocessors. However, adding a table would mean it uses extra (flash) 
memory as well. For a 32 CRC this takes up 256 * 32 bit = 1KB which is fair enough considering the hardware constraints 
of most microcontrollers. 

## Implementation complexity

Strong specific polynomials are defined in industry standard CRCs. https://en.wikipedia.org/wiki/Cyclic_redundancy_check. 
Using such standardized polynomial ensures compatibility with other systems. 

Polynomials such as the AUTOSAR polynomial https://www.autosar.org/fileadmin/standards/R22-11/CP/AUTOSAR_SWS_CRCLibrary.pdf
are used in automotive applications, however for UART this polynomial might not be the best choice due to Autosar being
optimised for different communication buses.

For UART the Castagnoli due to it being a highly robust polynomial against noise.
Device A and B should share the polynomial. Because of it's use in short messages and its optimizations for burst
errors Castagnoli would be a good fit.

## Conclusion
Given the strict reliability requirements of automotive applications, a CRC-32C (Castagnoli) including a lookup table approach
is a solid choice as it balances strong error detection, computational efficiency, and practical implementation complexity.

A time calculation is also added to get a small insight in how fast the CRC calculation is on a high-end
computer with a powerfull CPU. The average of calculating a CRC with a LUT is around 0.0002 ms.

However, as this conclusion is based on assumptions (and with proper hardware) it would be ideal to support this with 
automotive testing and more CRC simulation. 
