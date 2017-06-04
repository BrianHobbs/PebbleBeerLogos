#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_line_layer;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;
// logos to be displayed
static uint32_t s_logos[] = {RESOURCE_ID_LOGO_BREWDOG,
                             RESOURCE_ID_LOGO_MAGICROCK_1,
                             RESOURCE_ID_LOGO_MAGICROCK_2,
                             RESOURCE_ID_LOGO_GUINNESS,
                             RESOURCE_ID_LOGO_YEASTIEBOYS,
                             RESOURCE_ID_LOGO_FLYINGDOG
                            };
static int range;
static int logo_ticker = 15;

// update the logo every 15 minutes
static void update_logo() {
  if ( logo_ticker == 15 ) {
    logo_ticker = 0;
    gbitmap_destroy(s_logo_bitmap);
    s_logo_bitmap = gbitmap_create_with_resource(s_logos[ rand() % range]);
    bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  } else {
    logo_ticker++;
  }
}

static void update_time(struct tm *tick_time) {
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date(struct tm *tick_time) {
  static char s_buffer[8];
  // short month format
  strftime(s_buffer, sizeof(s_buffer), "%b %d", tick_time);
  text_layer_set_text(s_date_layer, s_buffer);
}

static void updateUI() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_time(tick_time);
  update_logo();
  update_date(tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  updateUI();
}

static void line_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorClear);
  graphics_draw_line(ctx, GPoint(0,0), GPoint(bounds.size.w, 0));
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // logo
  s_logo_layer = bitmap_layer_create(GRect((bounds.size.w - 60), 10, 50, 50));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));
  
  // Time
  int time_offset = 58;
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_REGULAR_48));
  s_time_layer = text_layer_create(GRect(0, (bounds.size.h - time_offset), bounds.size.w, 48));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Line
  int line_offset = time_offset + 1;
  s_line_layer = layer_create(GRect(10, (bounds.size.h - line_offset), (bounds.size.w - 20) , 1));
  layer_set_update_proc(s_line_layer, line_layer_update_proc);
  layer_add_child(window_layer, s_line_layer);
  
  // Date
  int date_offset = line_offset + 27;
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_REGULAR_24));
  s_date_layer = text_layer_create(GRect(10, (bounds.size.h - date_offset), ((bounds.size.w - 10) * 0.75), 24));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // init display
  updateUI();
  
  // Register Tick timer
   tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
  gbitmap_destroy(s_logo_bitmap);

  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  tick_timer_service_unsubscribe();
  bitmap_layer_destroy(s_logo_layer);
  layer_destroy(s_line_layer);
  
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init() {
  // random logo display setup
  srand((unsigned)time(NULL));
  range = sizeof(s_logos) / sizeof(s_logos[0]);
  
  // create main Window element
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  // set handlers to manage window elements
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show the window on the watch (animated = true)
  window_stack_push(s_main_window, true);

}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

