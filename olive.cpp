#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

struct Cordinate {
    int16_t x;
    int16_t y;
};

struct RectSize {
    int16_t width;
    int16_t height;
};



struct Canvas {
    RectSize size;
    vector<uint32_t> pixels;

    Canvas(RectSize sz) : size(sz) {
        pixels.resize(size.width * size.height);
    }
};


void fillCanvas(Canvas& canvas, uint32_t color) {
    for (auto& pixel : canvas.pixels) {
        pixel = color;
    }
};


using Errno = int;
Errno save2ppm(const Canvas& canvas, const string& path) {
    using UniqueFile = unique_ptr<FILE, decltype(&fclose)>;

    UniqueFile uf = UniqueFile(fopen(path.c_str(), "wb"), fclose);
    FILE *f = uf.get();
    if (f == nullptr) { return errno; }

    fprintf(
            f,
            "P6\n"
            "%hd %hd 255\n",
            canvas.size.width,
            canvas.size.height
           );
    if (ferror(f)) { return errno; }

    for (auto pixel : canvas.pixels) {
        uint8_t pixel_bytes[] = {
            static_cast<uint8_t>((pixel>>(8*0))&0xFF),
            static_cast<uint8_t>((pixel>>(8*1))&0xFF),
            static_cast<uint8_t>((pixel>>(8*2))&0xFF)
        };

        fwrite(pixel_bytes, sizeof(pixel_bytes), 1, f);
        if (ferror(f)) { return errno; }
    }

    return 0;
}



int main (int argc, char *argv[]) {
    Canvas canvas {RectSize {.width=400, .height=300}};
    fillCanvas(canvas, 0x00FF0000);
    save2ppm(canvas, "test.ppm");
    return 0;
}
