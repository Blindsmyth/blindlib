#!/bin/bash

# UUID Generation Helper
# Generates the next available UUID in the sequence

echo "🔢 Generating next available UUID..."

# Read the last UUID from .cursorrules
LAST_UUID=$(grep -o '5224773f-18de-4231-8f92-b1f22bb953[0-9A-F][0-9A-F]' .cursorrules | tail -1)

if [ -z "$LAST_UUID" ]; then
    echo "❌ No UUIDs found in registry. Starting from 5224773f-18de-4231-8f92-b1f22bb95370"
    LAST_UUID="5224773f-18de-4231-8f92-b1f22bb95370"
fi

# Extract the last two hex digits
LAST_HEX=${LAST_UUID: -2}

# Convert to decimal, increment, convert back to hex
LAST_DEC=$((16#$LAST_HEX))
NEXT_DEC=$((LAST_DEC + 1))
NEXT_HEX=$(printf "%02X" $NEXT_DEC)

# Generate new UUID
NEW_UUID="5224773f-18de-4231-8f92-b1f22bb953$NEXT_HEX"

echo "📋 UUID Information:"
echo "   Last UUID:  $LAST_UUID"
echo "   Next UUID:  $NEW_UUID"
echo ""

# Check if this UUID already exists
if grep -q "$NEW_UUID" .cursorrules; then
    echo "⚠️  WARNING: This UUID already exists in the registry!"
    echo "   Please check the registry manually."
else
    echo "✅ UUID is available for use."
fi

echo ""
echo "📝 To use this UUID:"
echo "   1. Add to .cursorrules: * your_object.axo: $NEW_UUID"
echo "   2. Use in .axo file: uuid=\"$NEW_UUID\""
echo "   3. Run ./check_uuid_conflicts.sh to verify"


