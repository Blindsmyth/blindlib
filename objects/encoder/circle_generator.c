#include <stdio.h>
#include <math.h>

int main() {
    int radius = 15; // Typical radius for our dials (adjust as needed)
    
    printf("// Pre-calculated circle coordinates for radius %d\n", radius);
    printf("// Format: {x_offset, y_offset} from center\n");
    printf("const int8_t circle_r%d[][2] = {\n", radius);
    
    // Generate circle coordinates for every 2 degrees (180 points total)
    for (int angle_deg = 0; angle_deg < 360; angle_deg += 2) {
        double angle_rad = angle_deg * M_PI / 180.0;
        int x = (int)round(cos(angle_rad) * radius);
        int y = (int)round(sin(angle_rad) * radius);
        
        printf("    {%3d, %3d}", x, y);
        if (angle_deg < 358) printf(",");
        printf("  // %d°\n", angle_deg);
    }
    
    printf("};\n\n");
    
    // Also generate indicator coordinates for different positions
    printf("// Pre-calculated indicator endpoints for radius %d\n", radius);
    printf("// 270° sweep from 135° to 45° (avoiding bottom gap)\n");
    printf("const int8_t indicator_r%d[][2] = {\n", radius);
    
    for (int i = 0; i <= 127; i++) { // 128 positions (0-127)
        float normalized = (float)i / 127.0f;
        float angle_deg = 135.0f + (normalized * 270.0f);
        if (angle_deg >= 360.0f) angle_deg -= 360.0f;
        double angle_rad = angle_deg * M_PI / 180.0;
        
        int x = (int)round(cos(angle_rad) * radius);
        int y = (int)round(sin(angle_rad) * radius);
        
        printf("    {%3d, %3d}", x, y);
        if (i < 127) printf(",");
        printf("  // pos %d (%.1f°)\n", i, angle_deg);
    }
    
    printf("};\n");
    
    return 0;
} 