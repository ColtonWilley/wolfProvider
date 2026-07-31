/* user_settings.h
 *
 * Copyright (C) 2006-2026 wolfSSL Inc.
 *
 * This file is part of wolfProvider.
 *
 * wolfProvider is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfProvider is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfProvider. If not, see <http://www.gnu.org/licenses/>.
 */

/* wolfSSL configuration for wolfProvider on Windows / Visual Studio, x64.
 *
 * Derived from wolfSSL v5.9.2-stable, wolfssl/options.h, as produced by the
 * configure invocation in scripts/utils-wolfssl.sh:
 *
 *     ./configure --enable-all-crypto --with-eccminsz=192
 *                 --with-max-ecc-bits=1024 --enable-opensslcoexist
 *                 --enable-sha
 *                 CFLAGS="-DWC_RSA_NO_PADDING -DWOLFSSL_PUBLIC_MP
 *                         -DHAVE_PUBLIC_FFDHE -DHAVE_FFDHE_6144
 *                         -DHAVE_FFDHE_8192 -DWOLFSSL_PSS_LONG_SALT
 *                         -DWOLFSSL_PSS_SALT_LEN_DISCOVER -DRSA_MIN_SIZE=1024
 *                         -DWOLFSSL_OLD_OID_SUM -DWC_RNG_SEED_CB"
 *
 * Authored 2026-07-29.
 *
 * THIS FILE IS MEANT TO BE READ AND EDITED.  Four things to know first.
 *
 * 1. THE TWO-COPY RULE.  This file configures wolfSSL as well as wolfProvider.
 *    It must be copied from wolfProvider\IDE\WINVS to wolfssl\IDE\WIN, where
 *    wolfSSL's own project finds it (wolfssl.vcxproj include dirs "./;./IDE/WIN",
 *    consumed at wolfssl/wolfcrypt/settings.h:359-360).  IF YOU CHANGE ONE COPY
 *    YOU MUST CHANGE BOTH COPIES.  The two copies diverging does not produce a
 *    build error -- it produces an ABI mismatch between wolfssl.dll and
 *    libwolfprov.dll.
 *
 * 2. options.h IS THE AUTHORITY ON THE CONTENT.  This file is the resolved
 *    Linux options.h body, minus exactly four host-probe macros (documented at
 *    the head of the first block below), plus the Windows guard and the include
 *    guard.  Nothing has been added.  Roughly half of what wolfProvider depends
 *    on arrives as raw -D CFLAGS or as autoconf probe results, so the resolved
 *    file -- not the configure line -- is what has to be reproduced.
 *
 * 3. SOURCE ORDER AND DUPLICATES ARE PRESERVED.  The order below is options.h's
 *    and 20 of the 183 #define lines are repeats of an earlier macro
 *    (HAVE_TLS_EXTENSIONS appears 7 times, WOLFSSL_AES_DIRECT 6, and so on),
 *    exactly as configure emitted them.  They are harmless, and keeping them
 *    makes a line-for-line comparison against a future options.h possible.
 *    167 distinct macros, 183 #undef/#define pairs.
 *
 * 4. AFTER ANY EDIT, RUN BOTH CHECKS.  On the Linux host, the macro-set
 *    comparison against the options.h control (vm/windows/check-user-settings.sh,
 *    gate G2 of vm/windows/SPEC-windows-port-changes.md section 8.6).  In the
 *    guest, the MSVC assertion probe (vm/windows/wolfssl-config-assert.c, gate
 *    G3), which build-windows.ps1 runs unconditionally against both copies.
 *    The host check cannot see the Windows branches; the guest check is the one
 *    that stands between a wrong configuration and a shipped DLL.
 */

#ifndef _WIN_USER_SETTINGS_H_
#define _WIN_USER_SETTINGS_H_

/* Verify this is Windows */
#ifndef _WIN32
#error This user_settings.h header is only designed for Windows
#endif

/* ==========================================================================
 * Toolchain and host configuration
 * ==========================================================================
 */

/* FOUR MACROS ARE DELIBERATELY ABSENT FROM THIS FILE.
 *
 * They are autoconf host-probe results that the Linux options.h carries.  Each
 * is dropped for a measured consequence under MSVC, not merely because it has
 * no meaning there.  None of the four produces a compile or link error if it
 * survives, which is exactly why they are called out here.  Do not restore
 * them.
 *
 *   WOLFSSL_HAVE_ATOMIC_H  (options.h:23-24)
 *   HAVE_C___ATOMIC        (options.h:32-33)
 *       Together these select wc_port.h:551, which does #include <stdatomic.h>
 *       and typedefs wolfSSL_Atomic_Int as atomic_int.  That branch is tested
 *       BEFORE the MSVC branch at wc_port.h:572 (InterlockedOrAcquire /
 *       InterlockedExchange, wolfSSL_Atomic_Int == volatile long).  Leaving
 *       them in changes the type, and with it the layout of every struct that
 *       embeds a reference count.  An ABI change, silent at build time.
 *
 *   HAVE___UINT128_T       (options.h:542-543)
 *       types.h:268-281 -- with no __SIZEOF_INT128__ (MSVC has none) wolfSSL
 *       emits "typedef unsigned long uint128_t __attribute__((mode(TI)));",
 *       a hard MSVC error.  It also buys nothing on Windows: sp_int.h:316-320
 *       excludes the 128-bit sp_int_word on _WIN64 regardless.
 *
 *   HAVE_GETPID            (options.h:545-546)
 *       random.h:416-418 adds "pid_t pid;" to struct WC_RNG and random.c:1746
 *       calls getpid().  Neither name exists in the UCRT (MSVC spells it
 *       _getpid(), declared in <process.h>).  Dropping it changes
 *       sizeof(WC_RNG) relative to the Linux build; that is harmless only
 *       because wolfSSL and wolfProvider compile against this same file.
 *
 * HAVE_LIMITS_H and WOLFSSL_HAVE_ASSERT_H below look like host probes too, but
 * MSVC has both <limits.h> and <assert.h>.  They stay.
 */

#undef  HAVE_LIMITS_H
#define HAVE_LIMITS_H 1

#undef  WOLFSSL_HAVE_ASSERT_H
#define WOLFSSL_HAVE_ASSERT_H

/* From --enable-all-crypto.  Enables wolfSSH-oriented algorithm coverage; kept
 * for fidelity with the Linux configuration, which is the authority here. */

#undef  WOLFSSL_WOLFSSH
#define WOLFSSL_WOLFSSH

#undef  HAVE_THREAD_LS
#define HAVE_THREAD_LS

#undef  NO_DO178
#define NO_DO178

/* DO NOT REMOVE.  The highest-consequence line in this file.
 *
 * settings.h:3527 gates the SP_INT_BITS block on
 *     WOLFSSL_X86_64_BUILD || WOLFSSL_AARCH64_BUILD || OPENSSL_EXTRA
 * With the gate open, settings.h:3542 resolves SP_INT_BITS to MIN_FFDHE_BITS,
 * i.e. 8192 here by way of HAVE_FFDHE_8192 further down.  With the gate shut,
 * control falls through to sp_int.h:497-498 (#elif defined(WOLFSSL_SP_4096))
 * and SP_INT_BITS silently becomes 4096; dh.h:144 then derives DH_MAX_SIZE
 * from it and FFDHE-8192 is broken at run time.
 *
 * Two things make that failure silent.  Nothing derives this macro from
 * _M_X64 -- autoconf sets it from host_cpu alone, so a hand-written config is
 * the only place it can come from on MSVC.  And the guard meant to catch its
 * absence, settings.h:3514-3518, is dead code: SP_INT_BITS is not yet defined
 * when that #if is evaluated.
 *
 * Never satisfy the settings.h:3527 gate with OPENSSL_EXTRA instead.  It would
 * mask a missing WOLFSSL_X86_64_BUILD and it is on the never-set list
 * (SPEC section 8.5). */

#undef  WOLFSSL_X86_64_BUILD
#define WOLFSSL_X86_64_BUILD

/* ==========================================================================
 * Algorithm and feature enables
 *
 * Ordering below is options.h's, which follows the configure script rather
 * than any taxonomy.  It is preserved on purpose (see the file header).
 * ==========================================================================
 */

#undef  WOLFSSL_ASN_TEMPLATE
#define WOLFSSL_ASN_TEMPLATE

#undef  HAVE_ECC_CDH
#define HAVE_ECC_CDH

#undef  HAVE_ECC_KOBLITZ
#define HAVE_ECC_KOBLITZ

#undef  HAVE_ECC_SECPR2
#define HAVE_ECC_SECPR2

#undef  HAVE_ECC_SECPR3
#define HAVE_ECC_SECPR3

#undef  WOLFSSL_DES_ECB
#define WOLFSSL_DES_ECB

#undef  HAVE_AES_DECRYPT
#define HAVE_AES_DECRYPT

#undef  HAVE_AES_ECB
#define HAVE_AES_ECB

#undef  WOLFSSL_ALT_NAMES
#define WOLFSSL_ALT_NAMES

#undef  HAVE_FFDHE_2048
#define HAVE_FFDHE_2048

#undef  HAVE_FFDHE_3072
#define HAVE_FFDHE_3072

#undef  WOLFSSL_ASN_ALL
#define WOLFSSL_ASN_ALL

#undef  WOLFSSL_DH_EXTRA
#define WOLFSSL_DH_EXTRA

#undef  WOLFSSL_ECDSA_DETERMINISTIC_K_VARIANT
#define WOLFSSL_ECDSA_DETERMINISTIC_K_VARIANT

#undef  WOLFSSL_HAVE_ISSUER_NAMES
#define WOLFSSL_HAVE_ISSUER_NAMES

#undef  WC_KDF_NIST_SP_800_56C
#define WC_KDF_NIST_SP_800_56C

#undef  WC_RNG_BANK_SUPPORT
#define WC_RNG_BANK_SUPPORT

/* --- --enable-opensslcoexist: five macros, all of them required ------------
 * OPENSSL_COEXIST is a suppression switch: it removes wolfSSL's OpenSSL
 * compatibility macro aliases across the ~30 headers under wolfssl/openssl/ so
 * that OpenSSL's and wolfSSL's headers can coexist in one translation unit.
 * Every wolfProvider source file includes both, so this is essential.
 * Not one of these five is visible on the configure command line.
 * NO_OLD_RNGNAME is deliberately not set. */

#undef  NO_OLD_WC_NAMES
#define NO_OLD_WC_NAMES

#undef  NO_OLD_SSL_NAMES
#define NO_OLD_SSL_NAMES

#undef  NO_OLD_SHA_NAMES
#define NO_OLD_SHA_NAMES

#undef  NO_OLD_MD5_NAME
#define NO_OLD_MD5_NAME

#undef  OPENSSL_COEXIST
#define OPENSSL_COEXIST

/* --- Side-channel hardening and threading --------------------------------- */

#undef  ERROR_QUEUE_PER_THREAD
#define ERROR_QUEUE_PER_THREAD

#undef  TFM_TIMING_RESISTANT
#define TFM_TIMING_RESISTANT

#undef  ECC_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT

#undef  WC_RSA_BLINDING
#define WC_RSA_BLINDING

#undef  ATOMIC_USER
#define ATOMIC_USER

#undef  HAVE_PK_CALLBACKS
#define HAVE_PK_CALLBACKS

/* --- AES modes ------------------------------------------------------------ */

#undef  HAVE_AES_ECB
#define HAVE_AES_ECB

#undef  WOLFSSL_AES_CBC_LENGTH_CHECKS
#define WOLFSSL_AES_CBC_LENGTH_CHECKS

#undef  HAVE_AESCCM
#define HAVE_AESCCM

#undef  WOLFSSL_AES_EAX
#define WOLFSSL_AES_EAX

#undef  WOLFSSL_AES_OFB
#define WOLFSSL_AES_OFB

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  WOLFSSL_AES_CFB
#define WOLFSSL_AES_CFB

#undef  WOLFSSL_ARMASM_NO_HW_CRYPTO
#define WOLFSSL_ARMASM_NO_HW_CRYPTO

/* --- Intel acceleration.  ABI-relevant -- read before touching. -----------
 * WOLFSSL_AESNI and USE_INTEL_SPEEDUP require wolfSSL's MASM sources.
 * wolfSSL's own vcxproj already carries the ml64.exe custom build rules for
 * them and does not exclude them from DLL Release|x64.
 *
 * WOLFSSL_GENERAL_ALIGNMENT is 16 here, and settings.h:3013 derives that from
 * WOLFSSL_AESNI ALONE -- not from USE_INTEL_SPEEDUP.  Alignment drives
 * XGEN_ALIGN on struct members, so it must match between the wolfSSL build and
 * the wolfProvider build or the result is an ABI break, not a link error.  Both
 * builds read this one file, which is what makes them match; never add a
 * per-project override.
 *
 * --- USE_INTEL_SPEEDUP IS DELIBERATELY OMITTED ON WINDOWS ------------------
 *
 * This is the ONE place this file knowingly diverges from the Linux options.h
 * beyond the four host-probe drops, and it is not a preference -- wolfSSL 5.9.2
 * cannot build with it under MSVC.
 *
 * Measured in wolfssl-source: wolfSSL ships FIFTEEN GAS (.S) assembly files for
 * x86-64 but only SIX MASM (.asm) files.  Nine have no Windows counterpart:
 *
 *     fe_x25519_asm      wc_mlkem_asm      sha3_asm
 *     sha256_asm         sha512_asm        wc_mldsa_asm
 *     aes_gcm_x86_asm    sm3_asm           sp_sm2_x86_64_asm
 *
 * --enable-intelasm (which is what -DUSE_INTEL_SPEEDUP means, configure.ac:4131)
 * turns on the C code paths that CALL into those, via automake conditionals such
 * as BUILD_CURVE25519_INTELASM (src/include.am:1885).  On MSVC the C compiles
 * fine and the DLL then fails to link:
 *
 *     ge_operations.obj : LNK2001 unresolved external symbol fe_sq / ge_add /
 *                         fe_invert_nct / sc_muladd ...
 *     wc_mlkem_poly.obj : LNK2001 unresolved external symbol
 *                         mlkem_keygen_avx2 / sha3_blocksx4_avx2 ...
 *
 * Omitting it is exactly --disable-intelasm, a supported wolfSSL configuration.
 * What is KEPT, and why this costs less than it appears:
 *
 *   - WOLFSSL_AESNI stays.  configure.ac:4142 makes AESNI independent of
 *     intelasm, aes_asm.asm DOES exist, and this is what pins alignment at 16.
 *   - WOLFSSL_SP_X86_64_ASM stays.  sp_x86_64_asm.asm exists, and this is the
 *     assembly that matters most for RSA/ECC/DH.
 *   - No algorithm is lost.  Curve25519, Ed25519, ML-KEM, ML-DSA and SHA-2/3 all
 *     still build -- from their C implementations instead of AVX2 assembly.
 *
 * So the cost is throughput on those specific primitives, not capability.
 *
 * Gate G2 reports USE_INTEL_SPEEDUP and its derived HAVE_INTEL_AVX1 /
 * HAVE_INTEL_AVX2 as control-only; all three are on the G2 allowlist for this
 * reason and this reason only.  If wolfSSL ever ships the missing MASM files,
 * restore the macro, drop those three allowlist entries, and re-run G2 and G3.
 *
 * #undef  USE_INTEL_SPEEDUP
 * #define USE_INTEL_SPEEDUP
 */

#undef  WOLFSSL_AESNI
#define WOLFSSL_AESNI

#undef  WOLFSSL_USE_ALIGN
#define WOLFSSL_USE_ALIGN

/* --- Ciphers and digests -------------------------------------------------- */

#undef  HAVE_CAMELLIA
#define HAVE_CAMELLIA

#undef  WOLFSSL_MD2
#define WOLFSSL_MD2

#undef  HAVE_NULL_CIPHER
#define HAVE_NULL_CIPHER

#undef  WOLFSSL_RIPEMD
#define WOLFSSL_RIPEMD

#undef  HAVE_BLAKE2B
#define HAVE_BLAKE2B

#undef  HAVE_BLAKE2S
#define HAVE_BLAKE2S

#undef  WOLFSSL_SHA224
#define WOLFSSL_SHA224

#undef  WOLFSSL_SHA512
#define WOLFSSL_SHA512

#undef  WOLFSSL_SHA384
#define WOLFSSL_SHA384

#undef  SESSION_CERTS
#define SESSION_CERTS

#undef  WOLFSSL_SEP
#define WOLFSSL_SEP

#undef  KEEP_PEER_CERT
#define KEEP_PEER_CERT

/* --- KDFs.  HAVE_HKDF and !NO_KDF are part of the floor set by the 14
 * wolfProvider sources that carry code at conditional depth 0; wp_krb5kdf.c
 * has no top-level #if at all.  Trimming them does not shrink the build, it
 * breaks it. */

#undef  HAVE_HKDF
#define HAVE_HKDF

#undef  HAVE_X963_KDF
#define HAVE_X963_KDF

/* --- ECC ------------------------------------------------------------------ */

#undef  HAVE_ECC
#define HAVE_ECC

#undef  ECC_SHAMIR
#define ECC_SHAMIR

/* Value-bearing; wolfProvider requires 192.  This is also the reason
 * WOLFSSL_HARDEN_TLS must never be set: settings.h:3218-3225 raises a hard
 * #error for ECC keys below 224 bits. */

#undef  ECC_MIN_KEY_SZ
#define ECC_MIN_KEY_SZ 192

#undef  HAVE_ECC_BRAINPOOL
#define HAVE_ECC_BRAINPOOL

#undef  FP_ECC
#define FP_ECC

#undef  HAVE_ECC_ENCRYPT
#define HAVE_ECC_ENCRYPT

#undef  WOLFCRYPT_HAVE_ECCSI
#define WOLFCRYPT_HAVE_ECCSI

#undef  WOLFSSL_PUBLIC_MP
#define WOLFSSL_PUBLIC_MP

#undef  WOLFCRYPT_HAVE_SAKKE
#define WOLFCRYPT_HAVE_SAKKE

/* --- RSA, encodings, MACs ------------------------------------------------- */

#undef  NO_OLD_TLS
#define NO_OLD_TLS

#undef  WC_RSA_PSS
#define WC_RSA_PSS

#undef  WOLFSSL_PSS_LONG_SALT
#define WOLFSSL_PSS_LONG_SALT

#undef  HAVE_ANON
#define HAVE_ANON

#undef  WOLFSSL_ASN_PRINT
#define WOLFSSL_ASN_PRINT

#undef  WOLFSSL_BASE64_ENCODE
#define WOLFSSL_BASE64_ENCODE

#undef  WOLFSSL_BASE16
#define WOLFSSL_BASE16

#undef  WOLFSSL_SIPHASH
#define WOLFSSL_SIPHASH

#undef  HAVE_CMAC_KDF
#define HAVE_CMAC_KDF

#undef  WOLFSSL_CMAC
#define WOLFSSL_CMAC

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  WOLFSSL_AES_XTS
#define WOLFSSL_AES_XTS

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

/* --- Curve25519 / Curve448 / Ed25519 / Ed448 ------------------------------ */

#undef  WOLFSSL_CUSTOM_CURVES
#define WOLFSSL_CUSTOM_CURVES

#undef  HAVE_CURVE448
#define HAVE_CURVE448

#undef  HAVE_ED448
#define HAVE_ED448

#undef  WOLFSSL_ED448_STREAMING_VERIFY
#define WOLFSSL_ED448_STREAMING_VERIFY

#undef  WC_SRTP_KDF
#define WC_SRTP_KDF

#undef  HAVE_AES_ECB
#define HAVE_AES_ECB

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

/* --- SHA-3, ML-KEM, ChaCha/Poly, DRBG ------------------------------------- */

#undef  WOLFSSL_SHA3
#define WOLFSSL_SHA3

#undef  WOLFSSL_SHAKE128
#define WOLFSSL_SHAKE128

#undef  WOLFSSL_SHAKE256
#define WOLFSSL_SHAKE256

#undef  WOLFSSL_HAVE_MLKEM
#define WOLFSSL_HAVE_MLKEM

#undef  WOLFSSL_TLS_NO_MLKEM_STANDALONE
#define WOLFSSL_TLS_NO_MLKEM_STANDALONE

#undef  WOLFSSL_PQC_HYBRIDS
#define WOLFSSL_PQC_HYBRIDS

#undef  HAVE_POLY1305
#define HAVE_POLY1305

#undef  HAVE_CHACHA
#define HAVE_CHACHA

#undef  HAVE_XCHACHA
#define HAVE_XCHACHA

#undef  HAVE_HASHDRBG
#define HAVE_HASHDRBG

#undef  WOLFSSL_DRBG_SHA512
#define WOLFSSL_DRBG_SHA512

/* --- TLS extensions, OCSP, CRL.  Present because the Linux build has them;
 * wolfProvider itself is a crypto provider and does not use the TLS layer. */

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_CERTIFICATE_STATUS_REQUEST
#define HAVE_CERTIFICATE_STATUS_REQUEST

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_CERTIFICATE_STATUS_REQUEST_V2
#define HAVE_CERTIFICATE_STATUS_REQUEST_V2

#undef  HAVE_CRL
#define HAVE_CRL

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SNI
#define HAVE_SNI

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  HAVE_FFDHE_2048
#define HAVE_FFDHE_2048

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  WOLFSSL_TLS13
#define WOLFSSL_TLS13

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_EXTENDED_MASTER
#define HAVE_EXTENDED_MASTER

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SNI
#define HAVE_SNI

#undef  HAVE_MAX_FRAGMENT
#define HAVE_MAX_FRAGMENT

#undef  HAVE_TRUNCATED_HMAC
#define HAVE_TRUNCATED_HMAC

#undef  HAVE_ALPN
#define HAVE_ALPN

#undef  HAVE_TRUSTED_CA
#define HAVE_TRUSTED_CA

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  WOLFCRYPT_HAVE_SRP
#define WOLFCRYPT_HAVE_SRP

#undef  ASN_BER_TO_DER
#define ASN_BER_TO_DER

#undef  HAVE_ENCRYPT_THEN_MAC
#define HAVE_ENCRYPT_THEN_MAC

#undef  WOLFSSL_ENCRYPTED_KEYS
#define WOLFSSL_ENCRYPTED_KEYS

#undef  HAVE_SCRYPT
#define HAVE_SCRYPT

/* --- Single-precision (SP) math, x86-64 assembly -------------------------
 * WOLFSSL_SP_X86_64, WOLFSSL_SP_X86_64_ASM, WOLFSSL_SP_ASM and
 * WOLFSSL_SP_LARGE_CODE look like x86 noise and are not: they are the macros a
 * hand-editor is most likely to delete, and they select the assembly
 * implementations.  Leave them. */

#undef  WOLFSSL_HAVE_SP_RSA
#define WOLFSSL_HAVE_SP_RSA

#undef  WOLFSSL_HAVE_SP_DH
#define WOLFSSL_HAVE_SP_DH

#undef  WOLFSSL_SP_4096
#define WOLFSSL_SP_4096

#undef  WOLFSSL_SP_LARGE_CODE
#define WOLFSSL_SP_LARGE_CODE

#undef  WOLFSSL_HAVE_SP_ECC
#define WOLFSSL_HAVE_SP_ECC

#undef  HAVE_ECC384
#define HAVE_ECC384

#undef  WOLFSSL_SP_384
#define WOLFSSL_SP_384

#undef  HAVE_ECC521
#define HAVE_ECC521

#undef  WOLFSSL_SP_521
#define WOLFSSL_SP_521

#undef  WOLFSSL_SP_1024
#define WOLFSSL_SP_1024

#undef  WOLFSSL_SP_MATH_ALL
#define WOLFSSL_SP_MATH_ALL

#undef  WOLFSSL_SP_X86_64
#define WOLFSSL_SP_X86_64

#undef  WOLFSSL_SP_ASM
#define WOLFSSL_SP_ASM

#undef  WOLFSSL_SP_X86_64_ASM
#define WOLFSSL_SP_X86_64_ASM

#undef  WOLF_CRYPTO_CB
#define WOLF_CRYPTO_CB

#undef  WC_NO_ASYNC_THREADING
#define WC_NO_ASYNC_THREADING

#undef  HAVE_AES_KEYWRAP
#define HAVE_AES_KEYWRAP

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  WOLFSSL_HASH_FLAGS
#define WOLFSSL_HASH_FLAGS

#undef  HAVE_DH_DEFAULT_PARAMS
#define HAVE_DH_DEFAULT_PARAMS

/* Value-bearing pair.  Both are asserted by value by the MSVC configuration
 * probe (vm/windows/wolfssl-config-assert.c). */

#undef  RSA_MAX_SIZE
#define RSA_MAX_SIZE 4096

#undef  MAX_ECC_BITS
#define MAX_ECC_BITS 1024

/* --- Certificate generation, key generation, misc ------------------------- */

#undef  HAVE_CURVE25519
#define HAVE_CURVE25519

#undef  HAVE_ED25519
#define HAVE_ED25519

#undef  WOLFSSL_SYS_CA_CERTS
#define WOLFSSL_SYS_CA_CERTS

#undef  WOLFSSL_KEY_GEN
#define WOLFSSL_KEY_GEN

#undef  WOLFSSL_CERT_REQ
#define WOLFSSL_CERT_REQ

#undef  WOLFSSL_CERT_GEN
#define WOLFSSL_CERT_GEN

#undef  WOLFSSL_CERT_EXT
#define WOLFSSL_CERT_EXT

#undef  HAVE_OCSP
#define HAVE_OCSP

#undef  HAVE_OPENSSL_CMD
#define HAVE_OPENSSL_CMD

#undef  WOLFSSL_ED25519_STREAMING_VERIFY
#define WOLFSSL_ED25519_STREAMING_VERIFY

#undef  WOLFSSL_AES_SIV
#define WOLFSSL_AES_SIV

#undef  WOLFSSL_AES_COUNTER
#define WOLFSSL_AES_COUNTER

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  HAVE_PKCS7
#define HAVE_PKCS7

#undef  NO_DES3_TLS_SUITES
#define NO_DES3_TLS_SUITES

#undef  GCM_TABLE_4BIT
#define GCM_TABLE_4BIT

#undef  HAVE_AESGCM
#define HAVE_AESGCM

#undef  WOLFSSL_AESGCM_STREAM
#define WOLFSSL_AESGCM_STREAM

#undef  WOLFSSL_AESXTS_STREAM
#define WOLFSSL_AESXTS_STREAM

#undef  WOLFSSL_PUBLIC_MP
#define WOLFSSL_PUBLIC_MP

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SERVER_RENEGOTIATION_INFO
#define HAVE_SERVER_RENEGOTIATION_INFO

#undef  HAVE_COMP_KEY
#define HAVE_COMP_KEY

#undef  WOLFSSL_ALLOW_RC4
#define WOLFSSL_ALLOW_RC4

#undef  WOLFSSL_TLS_OCSP_MULTI
#define WOLFSSL_TLS_OCSP_MULTI

#undef  HAVE_WC_INTROSPECTION
#define HAVE_WC_INTROSPECTION

#undef  WC_RSA_NO_PADDING
#define WC_RSA_NO_PADDING

#undef  WOLFSSL_PUBLIC_MP
#define WOLFSSL_PUBLIC_MP

/* --- FFDHE groups.  HAVE_FFDHE_8192 is what makes MIN_FFDHE_BITS 8192, which
 * is in turn what WOLFSSL_X86_64_BUILD (above) turns into SP_INT_BITS 8192.
 * The two are a matched pair; changing either one silently changes DH. */

#undef  HAVE_PUBLIC_FFDHE
#define HAVE_PUBLIC_FFDHE

#undef  HAVE_FFDHE_6144
#define HAVE_FFDHE_6144

#undef  HAVE_FFDHE_8192
#define HAVE_FFDHE_8192

#undef  WOLFSSL_PSS_LONG_SALT
#define WOLFSSL_PSS_LONG_SALT

#undef  WOLFSSL_PSS_SALT_LEN_DISCOVER
#define WOLFSSL_PSS_SALT_LEN_DISCOVER

/* Value-bearing; 1024.  wolfSSL 5.9.2 has a first-class --enable-wolfprovider
 * (configure.ac:462-475) that would lower RSA_MIN_SIZE via rsa.h:127-133, but
 * scripts/utils-wolfssl.sh does not use it, so HAVE_WOLFPROVIDER is absent
 * from options.h.  Do NOT define HAVE_WOLFPROVIDER here: the resolved values
 * it would have produced are already present in this file, and defining it
 * would move this file away from the options.h control and fail gate G2. */

#undef  RSA_MIN_SIZE
#define RSA_MIN_SIZE 1024

#undef  WOLFSSL_OLD_OID_SUM
#define WOLFSSL_OLD_OID_SUM

#undef  WC_RNG_SEED_CB
#define WC_RNG_SEED_CB

#endif /* _WIN_USER_SETTINGS_H_ */
