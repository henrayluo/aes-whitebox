// Copyright 2019 AES-128 WBC Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aes128_oracle.h"

#include <assert.h>

#include "aes128_oracle_tables.cc"

namespace {

void ShiftRows(uint8_t state[16]) {
  constexpr int shifts[16] = {
     0,  5, 10, 15,
     4,  9, 14,  3,
     8, 13,  2,  7,
    12,  1,  6, 11,
  };

  const uint8_t in[16] = {
    state[ 0], state[ 1], state[ 2], state[ 3],
    state[ 4], state[ 5], state[ 6], state[ 7],
    state[ 8], state[ 9], state[10], state[11],
    state[12], state[13], state[14], state[15],
  };

  for (int i = 0; i < 16; i++)
    state[i] = in[shifts[i]];
}

void Cipher(const uint8_t in[16], uint8_t out[16]) {
  // Copy the input to the state.
  uint8_t state[16] = {
    in[ 0], in[ 1], in[ 2], in[ 3],
    in[ 4], in[ 5], in[ 6], in[ 7],
    in[ 8], in[ 9], in[10], in[11],
    in[12], in[13], in[14], in[15],
  };

  // Perform the necessary number of rounds. The round key is added first.
  // The last round does not perform the MixColumns step.
  for (int r = 0; r < 9; r++) {
    ShiftRows(state);

    // Using T-boxes + Ty(i) Tables (single step):
    for (int j = 0; j < 4; ++j) {
      uint8_t n0, n1, n2, n3;
      uint32_t aa, bb, cc, dd;

      aa = Tyboxes[r][j*4 + 0][state[j*4 + 0]],
      bb = Tyboxes[r][j*4 + 1][state[j*4 + 1]],
      cc = Tyboxes[r][j*4 + 2][state[j*4 + 2]],
      dd = Tyboxes[r][j*4 + 3][state[j*4 + 3]];

      n0 = Xor[r][j*24 +  0][(aa >> 28) & 0xf][(bb >> 28) & 0xf];
      n1 = Xor[r][j*24 +  1][(cc >> 28) & 0xf][(dd >> 28) & 0xf];
      n2 = Xor[r][j*24 +  2][(aa >> 24) & 0xf][(bb >> 24) & 0xf];
      n3 = Xor[r][j*24 +  3][(cc >> 24) & 0xf][(dd >> 24) & 0xf];
      state[j*4 + 0] = (Xor[r][j*24 + 4][n0][n1] << 4) | Xor[r][j*24 + 5][n2][n3];

      n0 = Xor[r][j*24 +  6][(aa >> 20) & 0xf][(bb >> 20) & 0xf];
      n1 = Xor[r][j*24 +  7][(cc >> 20) & 0xf][(dd >> 20) & 0xf];
      n2 = Xor[r][j*24 +  8][(aa >> 16) & 0xf][(bb >> 16) & 0xf];
      n3 = Xor[r][j*24 +  9][(cc >> 16) & 0xf][(dd >> 16) & 0xf];
      state[j*4 + 1] = (Xor[r][j*24 + 10][n0][n1] << 4) | Xor[r][j*24 + 11][n2][n3];

      n0 = Xor[r][j*24 + 12][(aa >> 12) & 0xf][(bb >> 12) & 0xf];
      n1 = Xor[r][j*24 + 13][(cc >> 12) & 0xf][(dd >> 12) & 0xf];
      n2 = Xor[r][j*24 + 14][(aa >>  8) & 0xf][(bb >>  8) & 0xf];
      n3 = Xor[r][j*24 + 15][(cc >>  8) & 0xf][(dd >>  8) & 0xf];
      state[j*4 + 2] = (Xor[r][j*24 + 16][n0][n1] << 4) | Xor[r][j*24 + 17][n2][n3];

      n0 = Xor[r][j*24 + 18][(aa >>  4) & 0xf][(bb >>  4) & 0xf];
      n1 = Xor[r][j*24 + 19][(cc >>  4) & 0xf][(dd >>  4) & 0xf];
      n2 = Xor[r][j*24 + 20][(aa >>  0) & 0xf][(bb >>  0) & 0xf];
      n3 = Xor[r][j*24 + 21][(cc >>  0) & 0xf][(dd >>  0) & 0xf];
      state[j*4 + 3] = (Xor[r][j*24 + 22][n0][n1] << 4) | Xor[r][j*24 + 23][n2][n3];

      aa = MBL[r][j*4 + 0][state[j*4 + 0]];
      bb = MBL[r][j*4 + 1][state[j*4 + 1]];
      cc = MBL[r][j*4 + 2][state[j*4 + 2]];
      dd = MBL[r][j*4 + 3][state[j*4 + 3]];

      n0 = Xor[r][j*24 +  0][(aa >> 28) & 0xf][(bb >> 28) & 0xf];
      n1 = Xor[r][j*24 +  1][(cc >> 28) & 0xf][(dd >> 28) & 0xf];
      n2 = Xor[r][j*24 +  2][(aa >> 24) & 0xf][(bb >> 24) & 0xf];
      n3 = Xor[r][j*24 +  3][(cc >> 24) & 0xf][(dd >> 24) & 0xf];
      state[j*4 + 0] = (Xor[r][j*24 + 4][n0][n1] << 4) | Xor[r][j*24 + 5][n2][n3];

      n0 = Xor[r][j*24 +  6][(aa >> 20) & 0xf][(bb >> 20) & 0xf];
      n1 = Xor[r][j*24 +  7][(cc >> 20) & 0xf][(dd >> 20) & 0xf];
      n2 = Xor[r][j*24 +  8][(aa >> 16) & 0xf][(bb >> 16) & 0xf];
      n3 = Xor[r][j*24 +  9][(cc >> 16) & 0xf][(dd >> 16) & 0xf];
      state[j*4 + 1] = (Xor[r][j*24 + 10][n0][n1] << 4) | Xor[r][j*24 + 11][n2][n3];

      n0 = Xor[r][j*24 + 12][(aa >> 12) & 0xf][(bb >> 12) & 0xf];
      n1 = Xor[r][j*24 + 13][(cc >> 12) & 0xf][(dd >> 12) & 0xf];
      n2 = Xor[r][j*24 + 14][(aa >>  8) & 0xf][(bb >>  8) & 0xf];
      n3 = Xor[r][j*24 + 15][(cc >>  8) & 0xf][(dd >>  8) & 0xf];
      state[j*4 + 2] = (Xor[r][j*24 + 16][n0][n1] << 4) | Xor[r][j*24 + 17][n2][n3];

      n0 = Xor[r][j*24 + 18][(aa >>  4) & 0xf][(bb >>  4) & 0xf];
      n1 = Xor[r][j*24 + 19][(cc >>  4) & 0xf][(dd >>  4) & 0xf];
      n2 = Xor[r][j*24 + 20][(aa >>  0) & 0xf][(bb >>  0) & 0xf];
      n3 = Xor[r][j*24 + 21][(cc >>  0) & 0xf][(dd >>  0) & 0xf];
      state[j*4 + 3] = (Xor[r][j*24 + 22][n0][n1] << 4) | Xor[r][j*24 + 23][n2][n3];
    }
  }

  ShiftRows(state);

  // Using T-boxes:
  for (int i = 0; i < 16; i++)
    out[i] = Tboxes10[i][state[i]];
}

}  // namespace

extern "C" {

void aes128_oracle_encrypt_cfb(const uint8_t iv[16], const uint8_t* m,
    size_t len, uint8_t* c) {
  uint8_t cfb_blk[16];

  assert(len % 16 == 0);

  for (int i = 0; i < 16; i++)
    cfb_blk[i] = iv[i];

  for (size_t j = 0; j < (len / 16); j++) {
    Cipher(cfb_blk, cfb_blk);
    for (size_t i = 0; i < 16; i++) {
      cfb_blk[i] ^= m[j*16 + i];
      c[j*16 + i] = cfb_blk[i];
    }
  }
}

void aes128_oracle_decrypt_cfb(const uint8_t iv[16], const uint8_t* c,
    size_t len, uint8_t* m) {
  uint8_t cfb_blk[16];

  assert(len % 16 == 0);

  for (int i = 0; i < 16; i++)
    cfb_blk[i] = iv[i];

  for (size_t j = 0; j < (len / 16); j++) {
    Cipher(cfb_blk, cfb_blk);
    for (size_t i = 0; i < 16; i++) {
      m[j*16 + i] = cfb_blk[i] ^ c[j*16 + i];
      cfb_blk[i] = c[j*16 + i];
    }
  }
}

}  // extern "C"
