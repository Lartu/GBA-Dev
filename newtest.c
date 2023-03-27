#include "lamadil.h"

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
uint32 color2 = 10;
uint32 color_speed2 = 1;
uint32 color3 = 20;
uint32 color_speed3 = 1;

uint32 ldpl_timer = 0;
uint32 ldpl_timer2 = 0;

inline void start()
{
}

inline void update()
{
    fill_screen(create_color(color, color2, color3));
    
    /*draw_sprite(0, -40 + 28 + (lam_cos(ldpl_timer)>>8), bitmap, 64, 64);
    draw_sprite(64, -40 + 28 + (lam_cos(ldpl_timer + (PI / 4))>>8), bitmap, 64, 64);
    draw_sprite(128, -40 + 28 + (lam_cos(ldpl_timer + (PI / 2))>>8), bitmap, 64, 64);
    draw_sprite(128+64, -40 + 28 + (lam_cos(ldpl_timer + (PI * 3/ 4))>>8), bitmap, 64, 64);


    draw_sprite(0, 28 + (lam_sin(ldpl_timer)>>8), bitmap, 64, 64);
    draw_sprite(64, 28 + (lam_sin(ldpl_timer + (PI / 4))>>8), bitmap, 64, 64);
    draw_sprite(128, 28 + (lam_sin(ldpl_timer + (PI / 2))>>8), bitmap, 64, 64);
    draw_sprite(128+64, 28 + (lam_sin(ldpl_timer + (PI * 3/ 4))>>8), bitmap, 64, 64);

    draw_sprite(0, 68 + (lam_sin(ldpl_timer- (PI))>>8), bitmap, 64, 64);
    draw_sprite(64, 68 + (lam_sin(ldpl_timer - (PI / 4))>>8), bitmap, 64, 64);
    draw_sprite(128, 68 + (lam_sin(ldpl_timer - (PI / 2))>>8), bitmap, 64, 64);
    draw_sprite(128+64, 68 + (lam_sin(ldpl_timer - (PI * 3/ 4))>>8), bitmap, 64, 64);

    draw_sprite(0, 40+68 + (lam_cos(ldpl_timer)>>8), bitmap, 64, 64);
    draw_sprite(64, 40+68 + (lam_cos(ldpl_timer + (PI / 4))>>8), bitmap, 64, 64);
    draw_sprite(128, 40+68 + (lam_cos(ldpl_timer + (PI / 2))>>8), bitmap, 64, 64);
    draw_sprite(128+64, 40+68 + (lam_cos(ldpl_timer + (PI * 3/ 4))>>8), bitmap, 64, 64);*/

    for(uint32 i = 0; i < 240; ++i)
    {
        set_pixel(i, 80+(lam_sin(ldpl_timer + ((2 * PI) / 240) * i)>>8), 0x1FF);
        set_pixel(i, 81+(lam_sin(ldpl_timer + ((2 * PI) / 240) * i)>>8), 0x1FF);
    }

    for(uint32 i = 0; i < 240; ++i)
    {
        uint32 y = 80+(lam_sin(ldpl_timer2 + ((2 * PI) / 240) * i)>>8);
        Color point_color = 0x1FFF - screen_buffer[i + y * screen_width];
        set_pixel(i, y, point_color);
        set_pixel(i, y+1, point_color);
        set_pixel(i, y+2, point_color);
    }

    for(uint32 i = 0; i < 240; ++i)
    {
        uint32 y = 80+(lam_sin(ldpl_timer2 + ldpl_timer + ((2 * PI) / 240) * i)>>8);
        Color point_color = 0x1FFF - screen_buffer[i + y * screen_width];
        Color point_color_bw = create_color(point_color, point_color, point_color);
        set_pixel(i, y, point_color_bw);
    }

    ldpl_timer += PI/16;
    ldpl_timer2 += PI/10;
    
    color += color_speed;
    if(color == 0 || color == 31) color_speed = -color_speed;
    color2 += color_speed2;
    if(color2 == 0 || color2 == 31) color_speed2 = -color_speed2;
    color3 += color_speed3;
    if(color3 == 0 || color3 == 31) color_speed3 = -color_speed3;

    /*for(int iy = 0; iy < 32; ++iy){
        for(int ix = 0; ix < 32; ++ix){
            set_pixel(x+ix, y+iy, create_color(31-screen_buffer[(x+ix) + (y+iy) * screen_width], 31-screen_buffer[(x+ix) + (y+iy) * screen_width], 31-screen_buffer[(x+ix) + (y+iy) * screen_width]));
        }
    }
    
    for(int iy = 0; iy < 32; ++iy){
        for(int ix = 0; ix < 32; ++ix){
            set_pixel(x2+ix, y2+iy, 0x1FFF-screen_buffer[(x2+ix) + (y2+iy) * screen_width]);
        }
    }
    
    for(int iy = 0; iy < 32; ++iy){
        for(int ix = 0; ix < 32; ++ix){
            set_pixel(x3+ix, y3+iy, create_color(screen_buffer[(x3+ix) + (y3+iy) * screen_width], screen_buffer[(x3+ix) + (y3+iy) * screen_width], screen_buffer[(x3+ix) + (y3+iy) * screen_width]));
        }
    }*/

    x += speed_x;
    y += speed_y;
    x2 += speed_x2;
    y2 += speed_y2;
    x3 += speed_x3;
    y3 += speed_y3;

    if(x+speed_x <= 0 || x+speed_x >= 240-32) speed_x = -speed_x;
    if(y+speed_y <= 0 || y+speed_y >= 160-32) speed_y = -speed_y;
    if(x2+speed_x2 <= 0 || x2+speed_x2 >= 240-32) speed_x2 = -speed_x2;
    if(y2+speed_y2 <= 0 || y2+speed_y2 >= 160-32) speed_y2 = -speed_y2;
    
    if(x3+speed_x3 <= 0 || x3+speed_x3 >= 240-32) speed_x3 = -speed_x3;
    if(y3+speed_y3 <= 0 || y3+speed_y3 >= 160-32) speed_y3 = -speed_y3;

}