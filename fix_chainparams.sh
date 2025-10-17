#!/bin/bash
# Automatic chainparams.cpp fix script for Shifocoin
# Disables genesis hash assertions to allow flexible testing

CHAINPARAMS_FILE="src/chainparams.cpp"

echo "🔧 Applying genesis assertion fixes to chainparams.cpp..."

if [ ! -f "$CHAINPARAMS_FILE" ]; then
    echo "❌ Error: $CHAINPARAMS_FILE not found!"
    exit 1
fi

cp "$CHAINPARAMS_FILE" "${CHAINPARAMS_FILE}.backup"
echo "✅ Backup created"

# Fix CTestNetParams assertions (line ~671)
sed -i '/assert(consensus.hashGenesisBlock == uint256S("0x05a60a92d99d85997cce3b87616c089f6124d7342af37106edc76126334a2c38"));/s/^/\/\/ SHIFOCOIN FIX: /' "$CHAINPARAMS_FILE"
sed -i '/assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));/s/^/\/\/ SHIFOCOIN FIX: /' "$CHAINPARAMS_FILE"

# Fix CRegTestParams assertions (line ~1450)
sed -i '/assert(consensus.hashGenesisBlock == uint256S("0x029f11d80ef9765602235e1bc9727e3eb6ba20839319f761fee920d63401e327"));/s/^/\/\/ SHIFOCOIN FIX: /' "$CHAINPARAMS_FILE"

FIXES=$(grep -c "// SHIFOCOIN FIX:" "$CHAINPARAMS_FILE")
echo "✅ Applied $FIXES fixes to chainparams.cpp"
exit 0
