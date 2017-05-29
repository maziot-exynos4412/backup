#define WHITE   0x00ffffff
#define BLACK   0x00000000
#define RED     0x00ff0000
#define GREEN   0x0000ff00
#define BLUE    0x000000ff

int lcd_init(void);
void lcd_draw_point(int x, int y, unsigned int color);
void lcd_draw_line(int y, unsigned int color);
void lcd_draw_line_colorful(unsigned char *src, unsigned char* dst, unsigned int width);
void lcd_clear_screen(unsigned int color);
