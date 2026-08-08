#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SRC="$ROOT/package/kernel/rtl8192cd-rtl8197f/src"
CC_BIN=${HOSTCC:-${CC:-gcc}}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

"$CC_BIN" -std=gnu11 -Wall -Wextra -I"$SRC" \
  -c "$SRC/sha256.c" -o "$TMP/sha256.o"

for symbol in sha256_vector hmac_sha256_vector hmac_sha256 sha256_prf_bits sha256_prf; do
  nm -g --defined-only "$TMP/sha256.o" | grep -Eq "[[:space:]]T[[:space:]]+$symbol$" || {
    echo "missing local SHA-256 symbol: $symbol" >&2
    exit 1
  }
done

cat > "$TMP/test.c" <<'C_EOF'
#include <stddef.h>
#include <stdio.h>

int sha256_vector(size_t num_elem, const unsigned char *addr[],
                  const size_t *len, unsigned char *mac);
void sha256_prf(const unsigned char *key, size_t key_len, const char *label,
                const unsigned char *data, size_t data_len,
                unsigned char *buf, size_t buf_len);

static void dump(const unsigned char *p, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        printf("%02x", p[i]);
    putchar('\n');
}

int main(void)
{
    const unsigned char *vec[1] = {(const unsigned char *)"abc"};
    const size_t len[1] = {3};
    const unsigned char key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char data[5] = {0x10, 0x20, 0x30, 0x40, 0x50};
    unsigned char hash[32];
    unsigned char prf[48];

    if (sha256_vector(1, vec, len, hash))
        return 2;
    sha256_prf(key, sizeof(key), "RTL8197F-v42.3",
               data, sizeof(data), prf, sizeof(prf));
    dump(hash, sizeof(hash));
    dump(prf, sizeof(prf));
    return 0;
}
C_EOF

"$CC_BIN" -std=gnu11 "$TMP/test.c" "$TMP/sha256.o" -o "$TMP/test"
mapfile -t result < <("$TMP/test")
[[ ${result[0]:-} == ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad ]]
[[ ${result[1]:-} == 6c14d28de0853f6293446650cf5a7b819378f580783004b63936fb417e6e2bba96362beff98232da5ef29090c6f491c5 ]]

echo 'PASS: rtl8192cd local SHA-256/PRF symbols and known-answer vectors'
