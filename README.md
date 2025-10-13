# Shifocoin 1.0.0

<img align="right" width="120" height="80" src="doc/imgs/logo.png">

## What is Shifocoin?

**Shifocoin** is a privacy-focused cryptocurrency forked from [Zcash](https://z.cash/), offering **optional privacy** for users who want control over their transaction visibility.

Built on the proven Zcash codebase with Zerocash protocol, Shifocoin provides:
- **Optional Shielded Transactions** - Privacy by choice
- **Transparent Transactions** - For regulatory compliance
- **Total Supply:** 144 million SHFO
- **Custom Distribution:** 14.59% Founder + 7% Mining Pool + 78.41% Miners
- **Network Port:** 9033
- **Ticker Symbol:** SHFO

### Privacy by Choice, Freedom by Design

Unlike Monero's mandatory privacy, Shifocoin gives users the freedom to choose between transparent and shielded transactions based on their needs.

---

## The `shifocoind` Full Node

This repository hosts the `shifocoind` software, a Shifocoin consensus node implementation based on Zcash's `zcashd`. It downloads and stores the entire history of Shifocoin transactions.

The code is derived from [Zcash](https://github.com/zcash/zcash), which itself forked from [Bitcoin Core v0.11.2](https://github.com/bitcoin/bitcoin).

---

## :lock: Security Warnings

**Shifocoin is experimental and a work in progress.** Use it at your own risk.

- Keep your private keys secure
- Never share your seed phrases
- Use official wallet software only
- Verify all downloads and checksums

---

## Key Differences from Zcash

| Feature | Zcash | Shifocoin |
|---------|-------|-----------|
| **Total Supply** | 21 million ZEC | 144 million SHFO |
| **Ticker** | ZEC | SHFO |
| **Network Magic** | 0x24e92764 | 0x53484F4F (SHFO) |
| **Default Port** | 8233 | 9033 |
| **Distribution** | 80% miners, 20% dev | 78.41% miners, 14.59% founder, 7% pool |
| **Address Prefix** | t1/t3, zs | Sf/Sh, zs |

---

## Getting Started

### Prerequisites

- Linux (Debian/Ubuntu recommended)
- 4GB+ RAM
- 20GB+ free disk space
- Stable internet connection

### Building from Source

Build Shifocoin with most dependencies from source:

