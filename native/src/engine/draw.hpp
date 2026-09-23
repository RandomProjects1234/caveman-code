#pragma once

#include <string>

namespace cmc {

bool parse_color_rgb(const std::string& text, int& r, int& g, int& b);

struct DrawSurface {
    virtual ~DrawSurface() = default;
    virtual void clear() {}
    virtual void size(int w, int h) {
        (void)w;
        (void)h;
    }
    virtual void color(const std::string& c) { (void)c; }
    virtual void dot(double x, double y, double r) {
        (void)x;
        (void)y;
        (void)r;
    }
    virtual void circle(double x, double y, double r) {
        (void)x;
        (void)y;
        (void)r;
    }
    virtual void line(double x1, double y1, double x2, double y2) {
        (void)x1;
        (void)y1;
        (void)x2;
        (void)y2;
    }
    virtual void box(double x, double y, double w, double h) {
        (void)x;
        (void)y;
        (void)w;
        (void)h;
    }
    virtual void blob(double x, double y, double w, double h) {
        (void)x;
        (void)y;
        (void)w;
        (void)h;
    }
    virtual void write(const std::string& text, double x, double y) {
        (void)text;
        (void)x;
        (void)y;
    }
    virtual bool used() const { return false; }
    virtual void hold_open() {}
    virtual void pump() {}
};

class NullDraw : public DrawSurface {
public:
    explicit NullDraw(const std::string& title = "") { (void)title; }
};

#ifdef _WIN32

class Win32Draw : public DrawSurface {
public:
    struct Impl;

    explicit Win32Draw(const std::string& title);
    ~Win32Draw() override;

    void clear() override;
    void size(int w, int h) override;
    void color(const std::string& c) override;
    void dot(double x, double y, double r) override;
    void circle(double x, double y, double r) override;
    void line(double x1, double y1, double x2, double y2) override;
    void box(double x, double y, double w, double h) override;
    void blob(double x, double y, double w, double h) override;
    void write(const std::string& text, double x, double y) override;
    bool used() const override;
    void hold_open() override;
    void pump() override;

private:
    void ensure();
    void repaint();

    Impl* impl_;
};

#else

using Win32Draw = NullDraw;

#endif

}
