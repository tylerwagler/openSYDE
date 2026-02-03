# Plan: Replace OpenSSL with Qt Native Cryptography

**Status**: NOT RECOMMENDED
**Date Created**: 2026-01-16
**Project**: openSYDE Codebase Simplification

---

## Executive Summary

This document analyzes the feasibility of replacing OpenSSL with Qt-native cryptographic functions. **Conclusion: Qt cannot replace OpenSSL** for the cryptographic operations used in openSYDE.

Qt provides hashing (QCryptographicHash) and X.509 certificate parsing (QSslCertificate), but **does not provide RSA or ECDSA signing/verification** capabilities, which are core requirements for openSYDE's security features.

---

## Current OpenSSL Usage Analysis

### Files Using OpenSSL

| File | Purpose | OpenSSL APIs Used |
|------|---------|-------------------|
| [C_OscSecurityRsa.cpp](opensyde_tool/libs/opensyde_core/security/C_OscSecurityRsa.cpp) | RSA signature signing/verification | `RSA_private_encrypt`, `RSA_public_decrypt`, `EVP_PKEY`, `PKCS8_PRIV_KEY_INFO` |
| [C_OscSecurityEcdsa.cpp](opensyde_tool/libs/opensyde_core/security/C_OscSecurityEcdsa.cpp) | ECDSA signature signing/verification | `ECDSA_do_sign`, `ECDSA_do_verify`, `ECDSA_SIG`, `EVP_MD_CTX` (SHA-256) |
| [C_OscSecurityPem.cpp](opensyde_tool/libs/opensyde_core/security/C_OscSecurityPem.cpp) | PEM file parsing, RSA key extraction | `X509`, `EVP_PKEY`, `BN_bn2bin`, `d2i_X509`, `i2d_X509` |
| [C_OscSecurityPemBase.cpp](opensyde_tool/libs/opensyde_core/security/C_OscSecurityPemBase.cpp) | X.509 certificate handling | `X509_get_serialNumber`, `X509_get_key_usage`, `X509_print` |
| [C_OscSecurityPemSecUpdate.cpp](opensyde_tool/libs/opensyde_core/security/C_OscSecurityPemSecUpdate.cpp) | Secure update key handling | `PEM_read_bio_PrivateKey`, `EVP_PKEY` |

### OpenSSL Functions Summary

**Cryptographic Operations:**
- `RSA_private_encrypt()` - RSA signature creation
- `RSA_public_decrypt()` - RSA signature verification
- `ECDSA_do_sign()` - ECDSA secp256r1 signature creation
- `ECDSA_do_verify()` - ECDSA signature verification
- `EVP_MD_CTX` - SHA-256 digest computation

**Key Management:**
- `EVP_PKEY` - Generic key container
- `EVP_PKCS82PKEY()` - PKCS#8 to EVP_PKEY conversion
- `EVP_PKEY_get1_RSA()` - Extract RSA key from EVP_PKEY
- `EVP_PKEY_get_bn_param()` - Extract key parameters (modulus, exponent)
- `EVP_PKEY_get_octet_string_param()` - Extract raw key data

**X.509 Certificate Handling:**
- `X509_new()`, `d2i_X509()`, `i2d_X509()` - Create/parse/serialize certificates
- `X509_get0_pubkey()` - Extract public key
- `X509_get_serialNumber()` - Get certificate serial number
- `X509_get_key_usage()` - Get key usage flags
- `X509_get_extension_flags()` - Get extension flags
- `X509_print()` - Print certificate info

**Big Number Operations:**
- `BN_new()`, `BN_bn2bin()`, `BN_num_bytes()`, `BN_clear_free()` - Arbitrary precision integers

**PEM/DER Encoding:**
- `PEM_read_bio_PrivateKey()` - Read PEM-encoded private key
- `d2i_PKCS8_PRIV_KEY_INFO()` - Parse PKCS#8 data
- `i2d_PKCS8_PRIV_KEY_INFO()` - Serialize PKCS#8 data

### Library Size
- `libcrypto.a`: 6.5 MB
- `libssl.a`: 0.9 MB
- Header files: ~260 files in `opensyde_tool/libs/openssl/include/`

---

## Qt Cryptographic Capabilities

### What Qt Provides

| Capability | Qt Class | Status |
|------------|----------|--------|
| SHA-256 hashing | [QCryptographicHash](https://doc.qt.io/qt-6/qcryptographichash.html) | ✅ Available |
| SHA-384, SHA-512 | QCryptographicHash | ✅ Available |
| MD5, SHA-1 | QCryptographicHash | ✅ Available |
| X.509 certificate parsing | [QSslCertificate](https://doc.qt.io/qt-6/qsslcertificate.html) | ✅ Available |
| Public key extraction | QSslKey | ✅ Available |
| Certificate serial number | QSslCertificate::serialNumber() | ✅ Available |
| Certificate validity dates | QSslCertificate::effectiveDate()/expiryDate() | ✅ Available |
| RSA signing | - | ❌ **NOT AVAILABLE** |
| RSA verification | - | ❌ **NOT AVAILABLE** |
| ECDSA signing | - | ❌ **NOT AVAILABLE** |
| ECDSA verification | - | ❌ **NOT AVAILABLE** |
| Big number operations | - | ❌ **NOT AVAILABLE** |
| PKCS#8 key handling | - | ❌ **NOT AVAILABLE** |

### Qt's Cryptographic Limitations

Per the [Qt Forum discussion on ECDSA](https://forum.qt.io/topic/111541/is-there-something-in-qt-for-ecdsa-signing-and-public-key-recovery):
> Qt does not have native built-in support for ECDSA signing and public key recovery.

**QCryptographicHash** is explicitly limited to hashing only - no signing or encryption.

**QSslCertificate/QSslKey** are designed for SSL/TLS connection establishment, not standalone cryptographic operations.

---

## Feature Gap Analysis

### Can Be Replaced with Qt

| OpenSSL Function | Qt Equivalent | Notes |
|-----------------|---------------|-------|
| SHA-256 via EVP_MD_CTX | `QCryptographicHash::Sha256` | Direct replacement possible |
| X509 certificate parsing | `QSslCertificate` | Partial - some fields accessible |
| Certificate serial number | `QSslCertificate::serialNumber()` | Direct replacement |
| Certificate dates | `QSslCertificate::effectiveDate()` | Direct replacement |
| Public key extraction | `QSslCertificate::publicKey()` | Returns QSslKey |

### CANNOT Be Replaced with Qt

| OpenSSL Function | Purpose | Qt Alternative |
|-----------------|---------|----------------|
| `RSA_private_encrypt()` | Create RSA signature | ❌ None |
| `RSA_public_decrypt()` | Verify RSA signature | ❌ None |
| `ECDSA_do_sign()` | Create ECDSA signature | ❌ None |
| `ECDSA_do_verify()` | Verify ECDSA signature | ❌ None |
| `EVP_PKEY_get_bn_param()` | Extract key parameters | ❌ None |
| `BN_*` functions | Big number math | ❌ None |
| `d2i_PKCS8_PRIV_KEY_INFO()` | Parse private keys | ❌ None |

---

## Alternative Solutions

### Option 1: Keep OpenSSL (RECOMMENDED)

**Pros:**
- No migration effort
- Full cryptographic functionality
- Industry-standard, well-audited library
- Already working and tested in openSYDE

**Cons:**
- External dependency (~7.5 MB static libraries)
- Need to track OpenSSL updates

**Verdict:** ✅ Best option - OpenSSL provides functionality Qt cannot

### Option 2: QCA (Qt Cryptographic Architecture)

**What is QCA:** A KDE project providing cryptographic functionality for Qt applications.
**GitHub:** https://github.com/KDE/qca

**Pros:**
- Qt-friendly API
- Supports RSA, DSA, ECDSA
- Plugin-based architecture

**Cons:**
- Another external dependency (replaces OpenSSL with QCA)
- QCA often uses OpenSSL as a backend anyway
- Less widely used than OpenSSL
- Additional integration work required

**Verdict:** ❌ Not recommended - adds complexity without eliminating OpenSSL dependency

### Option 3: Crypto++ Library

**Pros:**
- Pure C++ implementation
- No external dependencies
- Comprehensive cryptographic support

**Cons:**
- Large library
- Different API style
- Significant migration effort
- Less common in Qt projects

**Verdict:** ❌ Not recommended - similar effort to keeping OpenSSL

### Option 4: Windows CNG / Platform-Specific APIs

**Pros:**
- Native Windows API
- No external library needed

**Cons:**
- Windows-only (breaks cross-platform potential)
- Complex API
- Major rewrite required

**Verdict:** ❌ Not recommended - platform lock-in

---

## Partial Migration Assessment

Could we partially migrate (use Qt where possible, keep OpenSSL for signing)?

### Feasible Partial Replacements

**SHA-256 Hashing:**
```cpp
// Before (OpenSSL):
EVP_MD_CTX* ctx = EVP_MD_CTX_new();
EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
EVP_DigestUpdate(ctx, data, len);
EVP_DigestFinal_ex(ctx, digest, &digest_len);

// After (Qt):
QCryptographicHash hash(QCryptographicHash::Sha256);
hash.addData(data, len);
QByteArray digest = hash.result();
```

**Certificate Parsing (basic info):**
```cpp
// Before (OpenSSL):
X509* cert = d2i_X509(...);
ASN1_INTEGER* serial = X509_get_serialNumber(cert);

// After (Qt):
QSslCertificate cert(data, QSsl::Der);
QByteArray serial = cert.serialNumber();
```

### NOT Feasible (Must Keep OpenSSL)

- RSA signature creation/verification
- ECDSA signature creation/verification
- Key parameter extraction (modulus, exponent)
- PKCS#8 private key handling
- Big number operations

### Assessment

A partial migration would:
- Reduce OpenSSL usage by ~10-15%
- Add complexity with two crypto libraries
- Not eliminate the OpenSSL dependency
- Provide minimal benefit for the effort

**Conclusion:** Partial migration is **not recommended**.

---

## Recommendation

**Keep OpenSSL as-is.**

### Rationale

1. **Qt cannot replace OpenSSL's signing functionality**
   - No RSA signing/verification
   - No ECDSA signing/verification
   - This is fundamental to openSYDE's security features

2. **OpenSSL is the industry standard**
   - Well-audited, widely used
   - Regular security updates
   - Excellent documentation

3. **Alternatives don't eliminate the dependency**
   - QCA typically uses OpenSSL backend
   - Other libraries require similar integration effort

4. **Migration effort vs. benefit**
   - High effort for partial migration
   - Zero benefit since OpenSSL still required

### Action Items

1. **Keep OpenSSL** - no migration needed
2. **Update OpenSSL periodically** - track security releases
3. **Document OpenSSL usage** - for future maintainers
4. **Monitor Qt roadmap** - in case Qt adds signing capabilities in future

---

## Summary Table

| Requirement | Qt Support | Recommendation |
|-------------|------------|----------------|
| SHA-256 hashing | ✅ Yes | Could use Qt, but no real benefit |
| X.509 parsing | ✅ Partial | Could use Qt for basic info |
| RSA signing | ❌ No | **Must keep OpenSSL** |
| RSA verification | ❌ No | **Must keep OpenSSL** |
| ECDSA signing | ❌ No | **Must keep OpenSSL** |
| ECDSA verification | ❌ No | **Must keep OpenSSL** |
| Key extraction | ❌ No | **Must keep OpenSSL** |

**Final Recommendation:** Keep OpenSSL. Qt does not provide the cryptographic signing capabilities required by openSYDE.

---

## References

- [QCryptographicHash Documentation](https://doc.qt.io/qt-6/qcryptographichash.html) - Hashing only, no signing
- [QSslCertificate Documentation](https://doc.qt.io/qt-6/qsslcertificate.html) - X.509 certificate class
- [Qt Forum: ECDSA signing support](https://forum.qt.io/topic/111541/is-there-something-in-qt-for-ecdsa-signing-and-public-key-recovery) - Confirms Qt lacks signing
- [QCA (Qt Cryptographic Architecture)](https://github.com/KDE/qca) - Alternative if Qt-style API preferred

---

**Document Version**: 1.0
**Last Updated**: 2026-01-16
