#include <iostream>

// Simple test to verify viewport calculations for Retina displays

int main() {
    // Test case 1: Default window size 400x300
    {
        int width = 400, height = 300;
        std::cout << "Test 1: Default window size\n";
        std::cout << "  Input: " << width << "x" << height << "\n";
        
        #ifdef __APPLE__
        // Simulate Retina scaling
        float scale = 2.0;
        width = (int)(width * scale);
        height = (int)(height * scale);
        std::cout << "  Retina scaled: " << width << "x" << height << "\n";
        #endif
        
        double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
        std::cout << "  Aspect ratio: " << aspectRatio << "\n\n";
    }
    
    // Test case 2: Custom size 800x600
    {
        int width = 800, height = 600;
        std::cout << "Test 2: Custom window size\n";
        std::cout << "  Input: " << width << "x" << height << "\n";
        
        #ifdef __APPLE__
        // Simulate Retina scaling
        float scale = 2.0;
        width = (int)(width * scale);
        height = (int)(height * scale);
        std::cout << "  Retina scaled: " << width << "x" << height << "\n";
        #endif
        
        double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
        std::cout << "  Aspect ratio: " << aspectRatio << "\n\n";
    }
    
    // Test case 3: Minimum size 64x64
    {
        int width = 64, height = 64;
        std::cout << "Test 3: Minimum window size\n";
        std::cout << "  Input: " << width << "x" << height << "\n";
        
        #ifdef __APPLE__
        // Simulate Retina scaling
        float scale = 2.0;
        width = (int)(width * scale);
        height = (int)(height * scale);
        std::cout << "  Retina scaled: " << width << "x" << height << "\n";
        #endif
        
        double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
        std::cout << "  Aspect ratio: " << aspectRatio << "\n\n";
    }
    
    std::cout << "Viewport calculations complete.\n";
    std::cout << "On macOS Retina displays, the framebuffer is 2x the logical window size.\n";
    std::cout << "The viewport must be set to framebuffer dimensions for correct rendering.\n";
    
    return 0;
}