// Felzenszwalb EDT + Signed Distance Field
// - Input mask: uint8_t array (0 = background, >0 = foreground)
// - Output sdf: float array (signed distances in pixels)
// - Complexity: O(width*height)

#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdint>

namespace BlackberryEditor {

    static inline float INF_F() { return 1e20f; }
    
    // 1D squared-distance transform (Felzenszwalb).
    // f: input array (float), where f[i] == 0 for feature pixels and INF otherwise.
    // d: output array of squared distances.
    // length: number of elements.
    // Uses integer arithmetic for indices but returns float squared distances.
    void edt_1d(const float *f, float *d, int length) {
        // Arrays for parabolas
        std::vector<int> v(length);
        std::vector<float> z(length + 1);
    
        int k = 0;
        v[0] = 0;
        z[0] = -INF_F();
        z[1] = +INF_F();
    
        auto square = [](float x) { return x * x; };
    
        // compute lower envelope
        for (int q = 1; q < length; ++q) {
            float s;
            while (true) {
                int p = v[k];
                // intersection x-coordinate between parabolas from p and q
                // ( (q^2 - p^2) + (f[q] - f[p]) ) / (2(q - p))
                float num = ( (q * (float)q - p * (float)p) + (f[q] - f[p]) );
                float den = 2.0f * (q - p);
                s = num / den;
                if (s <= z[k]) {
                    // pop last parabola
                    k--;
                    if (k < 0) { k = 0; break; }
                } else break;
            }
            ++k;
            v[k] = q;
            z[k] = s;
            z[k+1] = +INF_F();
        }
    
        // sample lower envelope
        int ptr = 0;
        for (int q = 0; q < length; ++q) {
            while (z[ptr+1] < (float)q) ptr++;
            int p = v[ptr];
            float diff = q - p;
            d[q] = (diff * diff) + f[p];
        }
    }
    
    // 2D EDT wrapper: compute squared distances for whole image.
    // src is row-major with width*height floats (expected f values: 0 for feature, INF otherwise)
    // tmp is a workspace array of size width*height
    // out is destination array of squared distances (size width*height)
    void edt_2d(const float *src, float *tmp, float *out, int width, int height) {
        // 1) transform along columns (y) -> tmp
        std::vector<float> colBuf(std::max(width, height));
        std::vector<float> colOut(std::max(width, height));
    
        // For each column: build f(y) from src and apply 1D EDT
        for (int x = 0; x < width; ++x) {
            // build column f(y)
            for (int y = 0; y < height; ++y) {
                colBuf[y] = src[y * width + x];
            }
            edt_1d(colBuf.data(), colOut.data(), height);
            for (int y = 0; y < height; ++y) {
                tmp[y * width + x] = colOut[y];
            }
        }
    
        // 2) transform along rows (x) -> out
        std::vector<float> rowBuf(width);
        std::vector<float> rowOut(width);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                rowBuf[x] = tmp[y * width + x];
            }
            edt_1d(rowBuf.data(), rowOut.data(), width);
            for (int x = 0; x < width; ++x) {
                out[y * width + x] = rowOut[x];
            }
        }
    }
    
    // Build signed distance field from binary mask (uint8_t: 0 or >0)
    // mask: input array [width*height], 0 => background, >0 => foreground
    // sdfOut: output float array [width*height] (signed distances in pixels)
    // If normalize = true, distances are divided by maxDist and clamped to [-1,1]
    void BuildSignedDistanceField(const uint8_t *mask, int width, int height, float *sdfOut, bool normalize = false, float maxDist = 128.0f) {
        const int N = width * height;
    
        // Prepare arrays for EDT inputs/outputs
        std::vector<float> f_fore(N), f_back(N), tmp(N), out_fore(N), out_back(N);
    
        // For edt, feature pixels must be f[i] = 0, others = INF
        // We want:
        // - out_fore  = squared distance to nearest foreground (mask==1)
        // - out_back  = squared distance to nearest background (mask==0)
    
        for (int i = 0; i < N; ++i) {
            if (mask[i] > 0) {
                // foreground pixel => feature for foreground EDT
                f_fore[i] = 0.0f;
                f_back[i] = INF_F();
            } else {
                // background pixel => feature for background EDT
                f_fore[i] = INF_F();
                f_back[i] = 0.0f;
            }
        }
    
        // Compute squared distances
        edt_2d(f_fore.data(), tmp.data(), out_fore.data(), width, height);
        edt_2d(f_back.data(), tmp.data(), out_back.data(), width, height);
    
        // Now compute signed distance: inside positive
        // distFore = sqrt(out_fore) : distance to nearest foreground
        // distBack = sqrt(out_back) : distance to nearest background
        // signed = distBack - distFore  (positive inside, negative outside)
    
        float invMax = 1.0f / maxDist;
        for (int i = 0; i < N; ++i) {
            float distFore = std::sqrt(out_fore[i]);
            float distBack = std::sqrt(out_back[i]);
            float signedDist = distBack - distFore; // >0 inside, <0 outside
    
            if (normalize) {
                float v = signedDist * invMax;
                if (v > 1.0f) v = 1.0f;
                if (v < -1.0f) v = -1.0f;
                sdfOut[i] = v;
            } else {
                sdfOut[i] = signedDist;
            }
        }
    }

} // namespace BlackberryEditor