#pragma once

#include "graphics.hpp"

class MouseCursor {
    public:
        MouseCursor(PixelWriter* writer, PixelColor erase_color,
            Vector2D<int> initial_position);
        void MoveRelatives(Vector2D<int> displacement);

    private:
        PixelWriter* pixel_writer = nullptr;
        PixelColor erase_color_;
        Vector2D<int> position_;
}