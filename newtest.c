#include "lamadil.h"

uint32 state = 0;

void start()
{
}

uint32 speed_x = 4;
uint32 speed_y = 4;
uint32 x = 50;
uint32 y = 50;
uint32 speed_x2 = -4;
uint32 speed_y2 = 2;
uint32 x2 = 50;
uint32 y2 = 50;
uint32 speed_x3 = 3;
uint32 speed_y3 = -3;
uint32 x3 = 50;
uint32 y3 = 50;
uint32 color = 0;
uint32 color_speed = 1;

void update()
{
    fill_screen(create_color(color, color, color));
    color += color_speed;
    if(color == 0 || color == 31) color_speed = -color_speed;

    set_pixel(x+1, y, create_color(31-color, 31-color, 31-color));
    set_pixel(x, y+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x+2, y+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x+1, y+2, create_color(31-color, 31-color, 31-color));
    set_pixel(x+1, y+1, create_color(31, 0, 0));

    set_pixel(x2+1, y2, create_color(31-color, 31-color, 31-color));
    set_pixel(x2,   y2+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x2+2, y2+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x2+1, y2+2, create_color(31-color, 31-color, 31-color));
    set_pixel(x2+1, y2+1, create_color(0, 31, 0));

    set_pixel(x3+1, y3, create_color(31-color, 31-color, 31-color));
    set_pixel(x3,   y3+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x3+2, y3+1, create_color(31-color, 31-color, 31-color));
    set_pixel(x3+1, y3+2, create_color(31-color, 31-color, 31-color));
    set_pixel(x3+1, y3+1, create_color(0, 0, 31));

    for(uint32 iy = 0; iy < 16; ++iy)
    {
        for(uint32 ix = 0; ix < 16; ++ix)
        {
            Color color = create_color(bitmap[ix + iy*16], bitmap[ix + iy*16], bitmap[ix + iy*16]);
            if(color > 0) set_pixel(x+ix, y+iy, color);
        }
    }

    x += speed_x;
    y += speed_y;
    x2 += speed_x2;
    y2 += speed_y2;
    x3 += speed_x3;
    y3 += speed_y3;

    if(x <= 10 || x >= 240-13) speed_x = -speed_x;
    if(y <= 10 || y >= 160-13) speed_y = -speed_y;
    if(x2 <= 10 || x2 >= 240-13) speed_x2 = -speed_x2;
    if(y2 <= 10 || y2 >= 160-13) speed_y2 = -speed_y2;
    if(x3 <= 10 || x3 >= 240-13) speed_x3 = -speed_x3;
    if(y3 <= 10 || y3 >= 160-13) speed_y3 = -speed_y3;

}