#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

/*
 * Produces a compact binary representation of the original float.
 *
 * This converts a normal floating-point number into a custom
 * “IEEE-754-style” binary format stored inside an integer.
 *
 * Sequence of what it does:
 *
 * 1. Detect the sign of the number (positive or negative).
 * 2. Convert the number into its normalized form:
 *        1.xxx × 2^shift
 *    It repeatedly divides or multiplies by 2 until the value
 *    sits between 1.0 and 2.0, counting how many steps were taken.
 *
 * 3. Build the significand (fraction):
 *    Keep only the digits after the “1.” part by removing the leading 1
 *    and turning the fractional part into an integer.
 *
 * 4. Build the exponent:
 *    Take the shift value, add the exponent bias, and store it.
 *
 * 5. Combine sign bit, exponent bits, and significand bits into
 *    one final packed integer and return it.
 *
 */
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;

    // How many bits we use to store the detailed part of the number
    unsigned significandbits = bits - expbits - 1;

    // If the number is zero, all bits are zero — easy case
    if (f == 0.0) return 0;

    // Sign bit: Figure out if the number is negative or positive
    // Also make fnorm a positive number for processing
    if (f < 0) { sign = 1; fnorm = -f; }
    else       { sign = 0; fnorm =  f; }

    // Make fnorm fall between 1.0 and 2.0
    // Count how many times we had to scale it.
    //ELI5: “Shrink or grow the number until it sits nicely between 1.0 and 2.0, and count how many steps you took.”
    // The point: If a leading 1 -> store as significand, if power of two -> Store as exponent
    shift = 0;
    while (fnorm >= 2.0) { fnorm /= 2.0; shift++; }  // number too big → shrink it
    while (fnorm <  1.0) { fnorm *= 2.0; shift--; }  // number too small → grow it

    // Remove the leading 1 — computers assume it's there
    fnorm = fnorm - 1.0;

    // Turn the fractional part into an integer that fits in the significand field
    significand = fnorm * ((1LL << significandbits) + 0.5f);

    // Add the "bias" so the exponent fits into unsigned bits
    exp = shift + ((1 << (expbits - 1)) - 1);

    // Put all pieces together into the final packed number:
    // sign bit, then exponent bits, then significand bits
    return (sign << (bits - 1)) |
           (exp  << (bits - expbits - 1)) |
           significand;
}
/*
 * Restore the bit representation to its original floating-point value.
 *
 * This reverses pack754(): it takes a packed IEEE-754-style
 * integer and turns it back into a normal floating-point value.
 *
 * Sequence of what it does:
 *
 * 1. If the input is zero, return 0.0 immediately.
 *
 * 2. Extract the significand:
 *    - Pull out the fraction bits.
 *    - Convert them back into a fractional number.
 *    - Add back the hidden leading 1 (making it 1.xxx again).
 *
 * 3. Extract the exponent:
 *    - Pull out its bits.
 *    - Subtract the exponent bias.
 *    - Scale the number up/down by powers of two.
 *
 * 4. Apply the sign bit:
 *    - If the highest bit is 1, make the final number negative.
 *
 */
long double unpack754(uint64_t i, unsigned bits, unsigned expbits) {
    long double result;
    long long shift;
    unsigned bias;

    // How many bits store the detailed fraction part
    unsigned significandbits = bits - expbits - 1;

    // If the whole thing is zero, the value is 0.0
    if (i == 0) return 0.0;

    // --- GET THE SIGNIFICAND (the fraction part) ---
    // Turn bits into 1.xxx

    // Extract the bottom significandbits of i
    result = (i & ((1LL << significandbits) - 1));

    // Turn that integer into a fraction again
    result /= (1LL << significandbits);

    // Add back the "hidden" leading 1 that floating-point numbers assume
    result += 1.0f;


    // --- GET THE EXPONENT (the power-of-two scaling) ---
    // Rescale the number back to its true size

    // Compute the bias (center point of exponent values)
    bias = (1 << (expbits - 1)) - 1;

    // Extract the exponent bits and remove the bias
    shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;

    // Apply the exponent by scaling the number up or down
    while (shift > 0)  { result *= 2.0; shift--; }
    while (shift < 0)  { result /= 2.0; shift++; }


    // --- APPLY THE SIGN BIT ---

    // If the top bit is 1 → number is negative
    result *= ((i >> (bits - 1)) & 1) ? -1.0 : 1.0;


    return result;
}

int main(void) {
    float f = 3.1415926, f2;
    double d = 3.14159265358979323, d2;

    uint32_t fi;
    uint64_t di;

    /*Pack float to integer and unpack integer to float */
    fi = pack754_32(f);
    f2 = unpack754_32(fi);

    /*Pack double to integer and unpack integer to double */
    di = pack754_64(d);
    d2 = unpack754_64(di);

    printf("float before : %.7f\n", f);
    printf("float encoded: 0x%08" PRIx32 "\n", fi);
    printf("float after  : %.7f\n\n", f2);

    printf("double before : %.20lf\n", d);
    printf("double encoded: 0x%016" PRIx64 "\n", di);
    printf("double after  : %.20lf\n", d2);

    return 0;
}
