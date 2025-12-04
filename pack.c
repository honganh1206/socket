#include <stdint.h>
#include <stdio.h>

/* Pack float to unsigned integer */
uint32_t htonf(float f) {
    uint32_t p;
    uint32_t sign;

    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }

    /* Construct a custom 32-bit packed value.
     * First convert the float to integer by truncating its fractional part e.g., 3.7f to 3
     * then keep only the lower 15 bits of that integer, ensuring the magnitude field never exceeds 15 bits
     * then shift that 15-bit magnitude to bits 30 to 16 while reserving bits 15-0

	31 | 30........16 | 15........0
	 S |   magnitude   |   unused

     * On the right side of OR, we place the sign bit at bit 31 (MSB)
     * then finally combine the sign and the magnitude to form a single packed 32-bit value
     * */
    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31);

    /*Determine the fraction part?
     * Compute the fractional part, and scale that fractional part to 16-bit fixed point range 
     * (so the value stays close to the rounded value with increment of 1/65536)
     * then cast it to type uint32
     * and keep only the lower 16 bits to ensure the result fits EXACTLY into a 16-bit fractional field
     * then perform OR bitwise operation?*/
    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff;

    return p;
}

float ntohf(uint32_t p) {
    /*Shift the integer 15-bit magnitude to bits 14 to 0, leaving the sign bit at bit 15,
     * then mask away the sign bit (become 0)
     * the final result is the integer magnitude only*/
    float f = ((p>>16)&0x7fff);

    /*Add the fractional part */
    f += (p & 0xffff) / 65536.0f;

    /*Set sign bit
     * by read the sign bit stored at bit 31
     * and if it's 1, negate the reconstructed float?*/
    if (((p>>31)&0x1) == 0x1) { f = -f; }

    return f;
}

/*While this is fast and compact, it's not an efficient use of space and the range is limited */
int main(void) {
    float f = 3.1415926, f2;
    uint32_t netf;

    netf = htonf(f);  // convert to "network" form
    f2 = ntohf(netf); // convert back to test

    printf("Original: %f\n", f);        // 3.141593
    printf(" Network: 0x%08X\n", netf); // 0x0003243F
    printf("Unpacked: %f\n", f2);       // 3.141586

    return 0;
}
