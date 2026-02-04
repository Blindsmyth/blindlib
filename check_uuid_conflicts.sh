#!/bin/bash

# UUID Conflict Detection Script
# Checks for duplicate UUIDs across all .axo files

echo "🔍 Checking for UUID conflicts..."

# Find all .axo files in both blindlib and Axolonatics
BLINDLIB_FILES=$(find /Users/simon/Dropbox/blindlib -name "*.axo" -type f)
AXOLONATICS_FILES=$(find /Users/simon/Dropbox/Axolonatics -name "*.axo" -type f 2>/dev/null)

# Combine all files
ALL_FILES="$BLINDLIB_FILES $AXOLONATICS_FILES"

# Extract UUIDs and check for duplicates
echo "📋 Extracting UUIDs from all .axo files..."

# Create temporary file to store UUIDs
TEMP_FILE=$(mktemp)

for file in $ALL_FILES; do
    if [ -f "$file" ]; then
        # Extract UUID from file
        UUID=$(grep -o 'uuid="[^"]*"' "$file" | head -1 | sed 's/uuid="//;s/"//')
        if [ ! -z "$UUID" ]; then
            echo "$UUID|$file" >> "$TEMP_FILE"
        fi
    fi
done

# Check for duplicates
echo "🔍 Checking for duplicate UUIDs..."
DUPLICATES=$(sort "$TEMP_FILE" | cut -d'|' -f1 | uniq -d)

if [ -z "$DUPLICATES" ]; then
    echo "✅ No UUID conflicts found!"
else
    echo "❌ UUID CONFLICTS DETECTED:"
    echo ""
    for uuid in $DUPLICATES; do
        echo "🔴 UUID: $uuid"
        echo "   Found in:"
        grep "^$uuid|" "$TEMP_FILE" | cut -d'|' -f2 | sed 's/^/     /'
        echo ""
    done
fi

# Clean up
rm "$TEMP_FILE"

echo "📊 Summary:"
TOTAL_UUIDS=$(sort "$TEMP_FILE" 2>/dev/null | wc -l || echo "0")
echo "   Total UUIDs checked: $TOTAL_UUIDS"
echo "   Conflicts found: $(echo "$DUPLICATES" | wc -w)"


