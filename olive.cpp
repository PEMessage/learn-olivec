#include <algorithm>
#include <cstring>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

struct Cordinate {
    int16_t y;
    int16_t x;
};

struct RectSize {
    int16_t width;
    int16_t height;
};

using Errno = int;
using Color = uint32_t;


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



void fillRect(
        Canvas& canvas,
        const Cordinate& a,
        const Cordinate& b,
        Color color
        )
{

    auto [left_, right_] = std::minmax(a.x, b.x);
    auto [top_, bottom_] = std::minmax(a.y, b.y);

    const RectSize& size = canvas.size;


    auto left= clamp<int16_t>(left_, 0, size.width - 1);
    auto right= clamp<int16_t>(right_, 0, size.width - 1);

    auto top=clamp<int16_t>(top_, 0, size.height - 1);
    auto bottom=clamp<int16_t>(bottom_, 0, size.height - 1);



    for (auto y = top; y <= bottom ; y ++) {
        for (auto x = left; x <= right ; x ++) {
            canvas.pixels[y * size.width + x] = color;
        }
    }

}


void fillCircle(
        Canvas& canvas,
        const Cordinate& a,
        const Cordinate& b,
        Color color
        )
{

    auto [left_, right_] = std::minmax(a.x, b.x);
    auto [top_, bottom_] = std::minmax(a.y, b.y);



    const RectSize& size = canvas.size;


    auto left= clamp<int16_t>(left_, 0, size.width - 1);
    auto right= clamp<int16_t>(right_, 0, size.width - 1);

    auto top=clamp<int16_t>(top_, 0, size.height - 1);
    auto bottom=clamp<int16_t>(bottom_, 0, size.height - 1);

    int16_t cx = (left_ + right_) / 2;
    int16_t cy = (top_ + bottom_) / 2;

    int32_t a_sq = (right_ - left_) * (right_ - left_) / 4;
    int32_t b_sq = (bottom_ - top_) * (bottom_ - top_) / 4;

    for (auto y = top; y <= bottom; ++y) {
        int32_t dy = y - cy;
        for (auto x = left; x <= right; ++x) {
            int32_t dx = x - cx;

            if (dx * dx * b_sq + dy * dy * a_sq <= a_sq * b_sq) {
                canvas.pixels[y * size.width + x] = color;
            }
        }
    }
}

static_assert(is_same_v<decltype(fillRect), decltype(fillCircle)>, "Not same type");
using FillFunction = std::function<decltype(fillRect)>;


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


const Color GREY = 0x001a1a1a;
const Color RED =  0x000000FF;


int example_test() {
    Canvas canvas {RectSize {.width=400, .height=300}};
    fillCanvas(canvas, GREY);


    save2ppm(canvas, std::string(__func__) + ".ppm");
    return 0;

}


int checkerboard(FillFunction func, const string& name) {
    #define ROW 10
    #define COL 10

    Canvas canvas {RectSize {.width=400, .height=300}};
    fillCanvas(canvas, GREY);

    for (int r = 0; r < ROW; r++) {
        for (int c = 0; c < COL; c++) {
            if ((c + r) % 2 == 0) { continue; }

            Cordinate left_top {
                .y = static_cast<int16_t>(canvas.size.height / ROW * r),
                .x = static_cast<int16_t>(canvas.size.width / COL  * c)
            };

            Cordinate right_bottom {
                .y = static_cast<int16_t>(canvas.size.height / ROW * (r+1) - 1),
                .x = static_cast<int16_t>(canvas.size.width / COL  * (c+1) - 1)
            };
            func(canvas, left_top, right_bottom, RED);
        }
    }

    save2ppm(canvas, name + ".ppm");
    return 0;
}

int example_rect() {
    return checkerboard(fillRect, __func__);
}

int example_circle() {
    return checkerboard(fillCircle, __func__);
}


int main (int argc, char *argv[]) {
    if(example_test()) return 0;
    if(example_rect()) return 0;
    if(example_circle()) return 0;
}
