#pragma once

#include <windows.h>

#include <mutex>
#include <string>
#include <vector>

#include "draw.hpp"

namespace og {

class DrawingPanel : public cmc::DrawSurface {
public:
    explicit DrawingPanel(HWND parent);
    ~DrawingPanel() override;

    HWND handle() const { return hwnd_; }
    void layout(int x, int y, int w, int h);
    void wipe();
    bool used() const { return used_; }

    void clear() override;
    void size(int w, int h) override;
    void color(const std::string& c) override;
    void dot(double x, double y, double r) override;
    void circle(double x, double y, double r) override;
    void line(double x1, double y1, double x2, double y2) override;
    void box(double x, double y, double w, double h) override;
    void blob(double x, double y, double w, double h) override;
    void write(const std::string& text, double x, double y) override;
    void hold_open() override {}

private:
    struct Command {
        std::string name;
        double a = 0;
        double b = 0;
        double c = 0;
        double d = 0;
        std::string text;
    };

    void enqueue(Command command);
    void drain();
    void paint(HDC dc);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    HWND hwnd_ = nullptr;
    std::mutex mutex_;
    std::vector<Command> queue_;
    std::vector<Command> commands_;
    std::string color_ = "black";
    bool used_ = false;
    int width_ = 500;
    int height_ = 400;
};

}