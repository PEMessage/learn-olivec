#include <algorithm>
#include <cstring>
#include <memory>
#include <iostream>
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

void drawPoint(
        Canvas& canvas,
        const Cordinate& cord,
        Color color
        )
{
    if (!(0 <= cord.x && cord.x < canvas.size.width))  { return; }
    if (!(0 <= cord.y && cord.y < canvas.size.height)) { return; }

    canvas.pixels[cord.y * canvas.size.width + cord.x] = color;
    return;
}



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

    int64_t a_sq = (int64_t)(right_ - left_) * (right_ - left_) / 4;
    int64_t b_sq = (int64_t)(bottom_ - top_) * (bottom_ - top_) / 4;

    for (auto y = top; y <= bottom; ++y) {
        int64_t dy = y - cy;
        for (auto x = left; x <= right; ++x) {
            int64_t dx = x - cx;

            if (dx * dx * b_sq + dy * dy * a_sq <= a_sq * b_sq) {
                canvas.pixels[y * size.width + x] = color;
            }
        }
    }
}

void fillTriangle(
        Canvas& canvas,
        const Cordinate& a,
        const Cordinate& b,
        const Cordinate& c,
        Color color
        )
{

    auto [left_, right_] = std::minmax({a.x, b.x, c.x});
    auto [top_, bottom_] = std::minmax({a.y, b.y, c.y});



    const RectSize& size = canvas.size;


    auto left= clamp<int16_t>(left_, 0, size.width - 1);
    auto right= clamp<int16_t>(right_, 0, size.width - 1);

    auto top=clamp<int16_t>(top_, 0, size.height - 1);
    auto bottom=clamp<int16_t>(bottom_, 0, size.height - 1);


    for (auto y = top; y <= bottom; ++y) {
        for (auto x = left; x <= right; ++x) {

            auto edge1 = (b.y - a.y) * (x - a.x) - (b.x - a.x) * (y - a.y);
            auto edge2 = (c.y - b.y) * (x - b.x) - (c.x - b.x) * (y - b.y);
            auto edge3 = (a.y - c.y) * (x - c.x) - (a.x - c.x) * (y - c.y);

            if (
                    (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) ||
                    (edge1 <= 0 && edge2 <= 0 && edge3 <= 0)
               ) {
                drawPoint(canvas, Cordinate { .y = y, .x = x} , color);
            }
        }
    }
}


void drawCircle(
        Canvas& canvas,
        const Cordinate& a,
        const Cordinate& b,
        Color color
        )
{
    auto [left_, right_] = std::minmax(a.x, b.x);
    auto [top_, bottom_] = std::minmax(a.y, b.y);

    // const RectSize& size = canvas.size;

    int16_t cx = (left_ + right_) / 2;
    int16_t cy = (top_ + bottom_) / 2;

    int16_t double_cx = (left_ + right_);
    int16_t double_cy = (top_ + bottom_);

    int32_t a_sq = (right_ - left_) * (right_ - left_) / 4;
    int32_t b_sq = (bottom_ - top_) * (bottom_ - top_) / 4;

    for (auto x = cx, y = bottom_; x <= right_ ; ++x) {
        int32_t dx = x - cx;
        int32_t dy = ((y - cy) + (y - 1 - cy)) / 2 ;

        if (dx * dx * b_sq + dy * dy * a_sq <=  a_sq * b_sq ) {
            drawPoint(canvas, Cordinate {.y = (int16_t)y               , .x = x                        }, color) ;
            drawPoint(canvas, Cordinate {.y = (int16_t)(double_cy - y) , .x = x                        }, color) ;
            drawPoint(canvas, Cordinate {.y = (int16_t)(double_cy - y) , .x = (int16_t)(double_cx - x) }, color) ;
            drawPoint(canvas, Cordinate {.y = (int16_t)y               , .x = (int16_t)(double_cx - x) }, color) ;
        } else {
            // while(b) {
            //     if(a) break
            // }
            // -->
            // -->
            // while(b && !a) {
            // }
            while(!(dx * dx * b_sq + dy * dy * a_sq <= a_sq * b_sq) && !(y<=cy)) {
                y --;
                // if (y <= cy) break;
                drawPoint(canvas, Cordinate {.y = (int16_t)y               , .x = x                        }, color) ;
                drawPoint(canvas, Cordinate {.y = (int16_t)(double_cy - y) , .x = x                        }, color) ;
                drawPoint(canvas, Cordinate {.y = (int16_t)(double_cy - y) , .x = (int16_t)(double_cx - x) }, color) ;
                drawPoint(canvas, Cordinate {.y = (int16_t)y               , .x = (int16_t)(double_cx - x) }, color) ;
                dy = ((y - cy) + (y - 1 - cy)) / 2 ;
            }
        }
    }
}


void drawLine(
        Canvas& canvas,
        const Cordinate& a,
        const Cordinate& b,
        Color color
        )
{
    // Bresenham
    int16_t x0 = a.x, y0 = a.y;
    int16_t x1 = b.x, y1 = b.y;

    int16_t dx = std::abs(x1 - x0);
    int16_t dy = std::abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (true) {
        drawPoint(canvas, Cordinate { .y = y0, .x = x0  }, color);

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
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

int example_draw_circle() {
    return checkerboard(drawCircle, __func__);
}

int example_draw_line() {
    return checkerboard(drawLine, __func__);
}




int example_line() {
    Canvas canvas {RectSize {.width=400, .height=300}};
    fillCanvas(canvas, GREY);

    drawLine(
            canvas,
            Cordinate{.y = 0,   .x = 200},
            Cordinate{.y = 200, .x = 200},
            RED
            );
    fillTriangle(
            canvas,
            Cordinate{.y = 200, .x = 200},
            Cordinate{.y = 300, .x = 0},
            Cordinate{.y = 300, .x = 400},
            RED
            );

    save2ppm(canvas, string(__func__) + ".ppm");
    return 0;
}

int main (int argc, char *argv[]) {
    if(example_test()) return 0;
    if(example_rect()) return 0;
    if(example_circle()) return 0;
    if(example_draw_circle()) return 0;
    if(example_draw_line()) return 0;
    if(example_line()) return 0;
}
