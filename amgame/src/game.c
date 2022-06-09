#include <game.h>

#define FPS 60

#define SIDE 16

static int scr_w, scr_h;
static int x, y;
static int vx = 100, vy = 100;
static int board_x;

static void gameinit() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  scr_w = info.width;
  scr_h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

int readkey() {
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown)
    return event.keycode;

  return AM_KEY_NONE;
}

int kbd_event(int key){
  switch (key)
  {
  case AM_KEY_LEFT:
    board_x -= 3;
    break;
  case AM_KEY_RIGHT:
    board_x += 3;
    break;
  default:
    break;
  }
  return 0;
}

int game_progress(){
  x += vx / FPS;
  y += vy / FPS;
  return 0;
}

int screen_update(){
  draw_tile(0, 0, scr_w, scr_h, 0);
  draw_tile(x, y, 3, 3, 0xffffff);
  draw_tile(board_x, scr_h - 10, 10, 3, 0xffffff);
  return 0;
}

int uptime()
{
  return io_read(AM_TIMER_UPTIME).us;
}

int gameloop() {
  int next_frame = uptime();
  int key;
  while (1)
  {
    putch('.');
    printf("%d %d\n", uptime(), next_frame);
    while (uptime() < next_frame)
      continue; // 等待一帧的到来
    if ((key = readkey()) != AM_KEY_NONE)
      kbd_event(key); // 处理键盘事件
    game_progress();          // 处理一帧游戏逻辑，更新物体的位置等
    screen_update();          // 重新绘制屏幕
    next_frame += 1000000 / FPS; // 计算下一帧的时间
  }
}

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();

  gameinit();
  gameloop();
  return 0;
}
