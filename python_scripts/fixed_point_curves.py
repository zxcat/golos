def to_log_v0(x, wordsize=32, ebits=5):
    if x <= 1:
        return x
    # mantissa_bits, mantissa_mask are independent of x
    mantissa_bits = wordsize - ebits
    mantissa_mask = (1 << mantissa_bits) - 1

    msb = x.bit_length() - 1
    mantissa_shift = mantissa_bits - msb
    y = (msb << mantissa_bits) | ((x << mantissa_shift) & mantissa_mask)
    return y

def from_log(y, wordsize=32, ebits=5):
    if y <= 1:
        return y
    # mantissa_bits, leading_1, mantissa_mask are independent of x
    mantissa_bits = wordsize - ebits
    leading_1 = 1 << mantissa_bits
    mantissa_mask = leading_1 - 1

    msb = y >> mantissa_bits
    mantissa_shift = mantissa_bits - msb
    y = (leading_1 | (y & mantissa_mask)) >> mantissa_shift
    return y


def approx_sqrt_v0(x, wordsize=32, ebits=5):
    if x <= 1:
        return x
    # mantissa_bits, leading_1, mantissa_mask are independent of x
    mantissa_bits = wordsize - ebits
    leading_1 = 1 << mantissa_bits
    mantissa_mask = leading_1 - 1

    msb_x = x.bit_length() - 1
    mantissa_shift_x = mantissa_bits - msb_x
    to_log_x = (msb_x << mantissa_bits) | ((x << mantissa_shift_x) & mantissa_mask)

    z = to_log_x >> 1

    msb_z = z >> mantissa_bits
    mantissa_shift_z = mantissa_bits - msb_z
    result = (leading_1 | (z & mantissa_mask)) >> mantissa_shift_z
    return result


def approx_sqrt_v1(x):
    if x <= 1:
        return x
    # mantissa_bits, leading_1, mantissa_mask are independent of x
    msb_x = x.bit_length() - 1
    msb_z = msb_x >> 1
    msb_x_bit = 1 << msb_x
    msb_z_bit = 1 << msb_z
    mantissa_mask = msb_x_bit - 1

    mantissa_x = x & mantissa_mask
    if (msb_x & 1) != 0:
        mantissa_z_hi = msb_z_bit
    else:
        mantissa_z_hi = 0
    mantissa_z_lo = mantissa_x >> (msb_x - msb_z)
    mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1
    result = msb_z_bit | mantissa_z
    return result
