import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

# 565 to RGB tuple
def rgb565_to_rgb888(c):
    if c == 0:
        return (0, 0, 0)
    r = ((c >> 11) & 0x1F) * 255 // 31
    g = ((c >> 5) & 0x3F) * 255 // 63
    b = (c & 0x1F) * 255 // 31
    return (r, g, b)

C_BLACK   = 0x0000
C_WHITE   = 0xFFFF
C_RED     = 0xF800
C_GREEN   = 0x07E0
C_BLUE    = 0x001F
C_CYAN    = 0x07FF
C_MAGENTA = 0xF81F
C_YELLOW  = 0xFFE0
C_ORANGE  = 0xFD20
C_GRAY    = 0x8410
C_SPOTIFY = 0x1EAC

ICON_TEMP = [
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_RED, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_RED, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, C_RED, C_RED, C_RED, C_RED, C_RED, 0, 0,
    0, C_RED, C_RED, C_RED, C_RED, C_RED, 0, 0,
    0, 0, C_RED, C_RED, C_RED, 0, 0, 0
]

ICON_SUN = [
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0,
    C_ORANGE, 0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0, C_ORANGE,
    0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_ORANGE, 0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0, C_ORANGE,
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0
]

ICON_BTC = [
    0, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_WHITE, C_WHITE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_WHITE, C_WHITE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_ORANGE,
    0, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0
]

ICON_YOUTUBE = [
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0,
    C_RED,   C_RED,   C_WHITE, C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0
]

ICON_PLANE = [
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN,
    0, 0, C_WHITE, C_CYAN, C_CYAN, C_WHITE, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    0, 0, C_CYAN, 0, 0, C_CYAN, 0, 0
]

ICON_LIGHTNING = [
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, C_YELLOW, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_YELLOW, 0,
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, 0, C_YELLOW,    0,        0,        0,        0, 0
]

ICON_SPOTIFY = [
    0,         0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,         0,
    0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,
    C_SPOTIFY, 0,         0,         0,         0,         0,         C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, 0,         0,         0,         0,         C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY,
    0,         C_SPOTIFY, C_SPOTIFY, 0,         0,         C_SPOTIFY, C_SPOTIFY, 0,
    0,         0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,         0
]

ICON_STOPWATCH = [
    0,       0,       C_CYAN,  C_WHITE, C_WHITE, 0,       0,       0,
    0,       0,       0,       C_CYAN,  C_CYAN,  0,       0,       0,
    0,       C_CYAN,  C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_CYAN,  0,
    C_CYAN,  C_WHITE, 0,       0,       0,       0,       C_WHITE, C_CYAN,
    C_CYAN,  C_WHITE, 0,       C_CYAN,  C_CYAN,  0,       C_WHITE, C_CYAN,
    C_CYAN,  C_WHITE, 0,       0,       C_CYAN,  0,       C_WHITE, C_CYAN,
    0,       C_CYAN,  C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_CYAN,  0,
    0,       0,       0,       0,       0,       0,       0,       0
]

ICON_HOURGLASS = [
    C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE,
    0,        C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE, 0,
    0,        0,        C_ORANGE, C_YELLOW, C_YELLOW, C_ORANGE, 0,        0,
    0,        0,        0,        C_ORANGE, C_ORANGE, 0,        0,        0,
    0,        0,        C_ORANGE, 0,        0,        C_ORANGE, 0,        0,
    0,        C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE, 0,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE
]

ICON_PACMAN = [
    0,        0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,
    0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_YELLOW, C_YELLOW, 0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW,
    C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,        0,
    C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,        0,        0,
    C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,        0,
    0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    0,        0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0
]

FONT_3X5 = [
    [0x07, 0x05, 0x05, 0x05, 0x07], # 0
    [0x02, 0x06, 0x02, 0x02, 0x07], # 1
    [0x07, 0x01, 0x07, 0x04, 0x07], # 2
    [0x07, 0x01, 0x07, 0x01, 0x07], # 3
    [0x05, 0x05, 0x07, 0x01, 0x01], # 4
    [0x07, 0x04, 0x07, 0x01, 0x07], # 5
    [0x07, 0x04, 0x07, 0x05, 0x07], # 6
    [0x07, 0x01, 0x02, 0x02, 0x02], # 7
    [0x07, 0x05, 0x07, 0x05, 0x07], # 8
    [0x07, 0x05, 0x07, 0x01, 0x07], # 9
]

FONT_ALPHA_3X5 = [
    [0x02, 0x05, 0x07, 0x05, 0x05], # A
    [0x06, 0x05, 0x06, 0x05, 0x06], # B
    [0x07, 0x04, 0x04, 0x04, 0x07], # C
    [0x06, 0x05, 0x05, 0x05, 0x06], # D
    [0x07, 0x04, 0x07, 0x04, 0x07], # E
    [0x07, 0x04, 0x06, 0x04, 0x04], # F
    [0x07, 0x04, 0x05, 0x05, 0x07], # G
    [0x05, 0x05, 0x07, 0x05, 0x05], # H
    [0x07, 0x02, 0x02, 0x02, 0x07], # I
    [0x01, 0x01, 0x01, 0x05, 0x02], # J
    [0x05, 0x06, 0x04, 0x06, 0x05], # K
    [0x04, 0x04, 0x04, 0x04, 0x07], # L
    [0x05, 0x07, 0x05, 0x05, 0x05], # M
    [0x05, 0x06, 0x05, 0x03, 0x05], # N
    [0x07, 0x05, 0x05, 0x05, 0x07], # O
    [0x07, 0x05, 0x07, 0x04, 0x04], # P
    [0x07, 0x05, 0x05, 0x07, 0x01], # Q
    [0x06, 0x05, 0x06, 0x06, 0x05], # R
    [0x03, 0x04, 0x07, 0x01, 0x06], # S
    [0x07, 0x02, 0x02, 0x02, 0x02], # T
    [0x05, 0x05, 0x05, 0x05, 0x07], # U
    [0x05, 0x05, 0x05, 0x05, 0x02], # V
    [0x05, 0x05, 0x05, 0x07, 0x05], # W
    [0x05, 0x05, 0x02, 0x05, 0x05], # X
    [0x05, 0x05, 0x02, 0x02, 0x02], # Y
    [0x07, 0x01, 0x02, 0x04, 0x07], # Z
]

class MatrixScreen:
    def __init__(self):
        self.grid = [[0 for _ in range(32)] for _ in range(8)]
    
    def clear(self):
        self.grid = [[0 for _ in range(32)] for _ in range(8)]
        
    def draw_pixel(self, x, y, color):
        if 0 <= x < 32 and 0 <= y < 8:
            self.grid[y][x] = color

    def draw_icon(self, x, y, bitmap):
        for r in range(8):
            for c in range(8):
                val = bitmap[r * 8 + c]
                if val != 0:
                    self.draw_pixel(x + c, y + r, val)

    def draw_calendar_icon(self, x, y, day):
        cal_base = [
            0,       C_RED,   C_WHITE, C_RED,   C_RED,   C_WHITE, C_RED,   0,
            C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
            C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
            C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
            C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
            C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
            C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
            0,       C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0
        ]
        self.draw_icon(x, y, cal_base)
        if day < 10:
            for r in range(5):
                row = FONT_3X5[day][r]
                if row & 4: self.draw_pixel(x + 2, y + 2 + r, 0x0001) # Near black
                if row & 2: self.draw_pixel(x + 3, y + 2 + r, 0x0001)
                if row & 1: self.draw_pixel(x + 4, y + 2 + r, 0x0001)
        else:
            tens = day // 10
            units = day % 10
            for r in range(5):
                row_t = FONT_3X5[tens][r]
                if row_t & 4: self.draw_pixel(x + 1, y + 2 + r, 0x0001)
                if row_t & 2: self.draw_pixel(x + 2, y + 2 + r, 0x0001)
                if row_t & 1: self.draw_pixel(x + 3, y + 2 + r, 0x0001)
                row_u = FONT_3X5[units][r]
                if row_u & 4: self.draw_pixel(x + 4, y + 2 + r, 0x0001)
                if row_u & 2: self.draw_pixel(x + 5, y + 2 + r, 0x0001)
                if row_u & 1: self.draw_pixel(x + 6, y + 2 + r, 0x0001)

    def draw_small_char(self, x, y, c, color):
        if c == ' ':
            return 2
        if c == ':':
            self.draw_pixel(x, y + 1, color)
            self.draw_pixel(x, y + 3, color)
            return 2
        if c == '.':
            self.draw_pixel(x, y + 4, color)
            return 2
        if c == '!':
            self.draw_pixel(x, y, color)
            self.draw_pixel(x, y + 1, color)
            self.draw_pixel(x, y + 2, color)
            self.draw_pixel(x, y + 4, color)
            return 2
        if c in ("'", '^', '°'):
            self.draw_pixel(x, y, color)
            self.draw_pixel(x + 1, y, color)
            self.draw_pixel(x, y + 1, color)
            self.draw_pixel(x + 1, y + 1, color)
            return 3
        if c == '-':
            self.draw_pixel(x, y + 2, color)
            self.draw_pixel(x + 1, y + 2, color)
            self.draw_pixel(x + 2, y + 2, color)
            return 4
        if c == '%':
            self.draw_pixel(x, y, color)
            self.draw_pixel(x + 2, y, color)
            self.draw_pixel(x + 2, y + 1, color)
            self.draw_pixel(x + 1, y + 2, color)
            self.draw_pixel(x, y + 3, color)
            self.draw_pixel(x, y + 4, color)
            self.draw_pixel(x + 2, y + 4, color)
            return 4
        
        row_data = None
        if '0' <= c <= '9':
            row_data = FONT_3X5[ord(c) - ord('0')]
        elif 'A' <= c <= 'Z':
            row_data = FONT_ALPHA_3X5[ord(c) - ord('A')]
        elif 'a' <= c <= 'z':
            row_data = FONT_ALPHA_3X5[ord(c) - ord('a')]

        if not row_data:
            return 3

        for r in range(5):
            b = row_data[r]
            if b & 4: self.draw_pixel(x, y + r, color)
            if b & 2: self.draw_pixel(x + 1, y + r, color)
            if b & 1: self.draw_pixel(x + 2, y + r, color)
        return 4

    def draw_small_text(self, x, y, text, color):
        cur_x = x
        for c in text:
            cur_x += self.draw_small_char(cur_x, y, c, color)
        return cur_x - x

    def draw_digit_3x5(self, x, y, digit, color):
        for r in range(5):
            b = FONT_3X5[digit][r]
            if b & 4: self.draw_pixel(x, y + r, color)
            if b & 2: self.draw_pixel(x + 1, y + r, color)
            if b & 1: self.draw_pixel(x + 2, y + r, color)

    def draw_time_3x5(self, x, y, h, m, blink, col_time, col_colon):
        h1 = (h // 10) % 10
        h2 = h % 10
        m1 = (m // 10) % 10
        m2 = m % 10
        cur_x = x
        self.draw_digit_3x5(cur_x, y, h1, col_time); cur_x += 4
        self.draw_digit_3x5(cur_x, y, h2, col_time); cur_x += 4
        if blink:
            self.draw_pixel(cur_x, y + 1, col_colon)
            self.draw_pixel(cur_x, y + 3, col_colon)
        cur_x += 2
        self.draw_digit_3x5(cur_x, y, m1, col_time); cur_x += 4
        self.draw_digit_3x5(cur_x, y, m2, col_time)

    def draw_weekday_bar(self, active_day_idx, active_col, inactive_col, weekend_col, inactive_weekend_col, is_mon_start, start_x, y):
        for i in range(7):
            is_wknd = (i == 5 or i == 6) if is_mon_start else (i == 0 or i == 6)
            if i == active_day_idx:
                col = active_col
            else:
                col = weekend_col if (is_wknd and weekend_col != 0) else inactive_col
            bx = start_x + i * 3
            self.draw_pixel(bx, y, col)
            self.draw_pixel(bx + 1, y, col)

def render_device_image(matrix, title_tag="", subtitle=""):
    # High resolution hardware rendering
    # Canvas size: 760 x 240
    W, H = 760, 240
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Outer ambient soft shadow
    for i in range(12, 0, -2):
        alpha = int(18 * (1.0 - i / 12.0))
        draw.rounded_rectangle([20 - i, 20 - i, W - 20 + i, H - 20 + i], radius=24 + i, fill=(0, 0, 0, alpha))

    # TC001 Physical Outer Enclosure (Matte Dark Slate / Industrial Polymer)
    draw.rounded_rectangle([20, 20, W - 20, H - 20], radius=20, fill=(24, 25, 29, 255), outline=(45, 48, 55, 255), width=2)
    
    # Enclosure subtle top edge highlight (beveled sheen)
    draw.rounded_rectangle([21, 21, W - 21, 25], radius=4, fill=(65, 70, 80, 160))

    # Inner Smoked Acrylic Display Bezel
    # Active 32x8 matrix area: 32 cols x 18px = 576px, 8 rows x 18px = 144px
    grid_w = 32 * 18
    grid_h = 8 * 18
    mat_x = (W - grid_w) // 2 # 92
    mat_y = (H - grid_h) // 2 # 48

    # Acrylic recess frame
    draw.rounded_rectangle([mat_x - 12, mat_y - 10, mat_x + grid_w + 12, mat_y + grid_h + 10], 
                           radius=10, fill=(10, 11, 14, 255), outline=(30, 32, 38, 255), width=2)

    # Subtle top status badge / label
    if title_tag:
        draw.text((mat_x - 8, mat_y - 28), title_tag.upper(), fill=(140, 148, 165, 255))
    if subtitle:
        draw.text((mat_x + grid_w - 180, mat_y - 28), subtitle.upper(), fill=(90, 98, 115, 255))

    # Layer for LED glows
    glow_img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_img)

    # First pass: Draw unlit LED sockets & collect active glows
    cell_size = 18
    radius = 6 # diameter 12
    for r in range(8):
        for c in range(32):
            cx = mat_x + c * cell_size + cell_size // 2
            cy = mat_y + r * cell_size + cell_size // 2
            val = matrix.grid[r][c]

            # Unlit dark diode socket
            draw.ellipse([cx - radius, cy - radius, cx + radius, cy + radius],
                         fill=(18, 20, 24, 255), outline=(26, 29, 36, 255), width=1)

            if val != 0:
                rgb = rgb565_to_rgb888(val)
                # If it's the near-black calendar font digit (0x0001)
                if val == 0x0001:
                    rgb = (15, 17, 20)
                # Outer glow circle
                glow_draw.ellipse([cx - radius - 6, cy - radius - 6, cx + radius + 6, cy + radius + 6],
                                  fill=(rgb[0], rgb[1], rgb[2], 120))
                glow_draw.ellipse([cx - radius - 3, cy - radius - 3, cx + radius + 3, cy + radius + 3],
                                  fill=(rgb[0], rgb[1], rgb[2], 180))

    # Blur glow layer
    glow_blurred = glow_img.filter(ImageFilter.GaussianBlur(radius=3))
    img.alpha_composite(glow_blurred)

    # Second pass: Draw vibrant active LED dies and core hot-spots
    draw = ImageDraw.Draw(img)
    for r in range(8):
        for c in range(32):
            cx = mat_x + c * cell_size + cell_size // 2
            cy = mat_y + r * cell_size + cell_size // 2
            val = matrix.grid[r][c]
            if val != 0:
                rgb = rgb565_to_rgb888(val)
                if val == 0x0001:
                    rgb = (18, 20, 24)
                # Main illuminated die
                draw.ellipse([cx - radius, cy - radius, cx + radius, cy + radius],
                             fill=(rgb[0], rgb[1], rgb[2], 255))
                # Core intense phosphor emission center (hotspot)
                if val != 0x0001:
                    wh_r = min(255, rgb[0] + 80)
                    wh_g = min(255, rgb[1] + 80)
                    wh_b = min(255, rgb[2] + 80)
                    draw.ellipse([cx - 2, cy - 2, cx + 2, cy + 2], fill=(wh_r, wh_g, wh_b, 230))

    return img

def main():
    os.makedirs("docs/media", exist_ok=True)

    # 1. Screen 0: Clock & Calendar
    m0 = MatrixScreen()
    m0.draw_calendar_icon(0, 0, 15) # Day 15
    m0.draw_time_3x5(10, 1, 14, 35, True, C_WHITE, C_WHITE)
    # Weekday bar: Thursday active (idx 3 with Mon-start), cyan active, dim inactive workdays, red weekend
    m0.draw_weekday_bar(3, C_CYAN, 0x2104, C_RED, 0, True, 10, 7)
    img0 = render_device_image(m0, "SCREEN 0 // CLOCK & CALENDAR", "14:35 UTC+2 | OCT 15")
    img0.save("docs/media/screen_0_clock.png")
    print("Saved screen_0_clock.png")

    # 2. Screen 1: Indoor Environment (SHT30)
    m1 = MatrixScreen()
    m1.draw_icon(0, 0, ICON_TEMP)
    m1.draw_small_text(9, 1, "23.4°C", C_YELLOW)
    img1 = render_device_image(m1, "SCREEN 1 // INDOOR TELEMETRY", "SHT30 PRECISION SENSOR")
    img1.save("docs/media/screen_1_indoor.png")
    print("Saved screen_1_indoor.png")

    # 3. Screen 2: Outdoor Weather (Open-Meteo)
    m2 = MatrixScreen()
    m2.draw_icon(0, 0, ICON_SUN)
    m2.draw_small_text(9, 1, "19°C", C_CYAN)
    img2 = render_device_image(m2, "SCREEN 2 // OUTDOOR WEATHER", "OPEN-METEO SYNCED")
    img2.save("docs/media/screen_2_outdoor.png")
    print("Saved screen_2_outdoor.png")

    # 4. Screen 3: Financial Markets & Crypto
    m3 = MatrixScreen()
    m3.draw_icon(0, 0, ICON_BTC)
    m3.draw_small_text(9, 1, "84.5K", C_GREEN)
    img3 = render_device_image(m3, "SCREEN 3 // CRYPTO & STOCKS", "BITCOIN / USD LIVE")
    img3.save("docs/media/screen_3_crypto.png")
    print("Saved screen_3_crypto.png")

    # 5. Screen 4: YouTube Subscriber Counter
    m4 = MatrixScreen()
    m4.draw_icon(0, 0, ICON_YOUTUBE)
    m4.draw_small_text(9, 1, "125K", C_WHITE)
    img4 = render_device_image(m4, "SCREEN 4 // YOUTUBE METRICS", "LIVE SUBSCRIBER TELEMETRY")
    img4.save("docs/media/screen_4_youtube.png")
    print("Saved screen_4_youtube.png")

    # 6. Screen 5: Live Airspace Radar (ADS-B)
    m5 = MatrixScreen()
    m5.draw_icon(0, 0, ICON_PLANE)
    m5.draw_small_text(9, 1, "RYR45", C_CYAN)
    img5 = render_device_image(m5, "SCREEN 5 // AIRSPACE RADAR", "OPENCLIMB FLIGHT TRACKER")
    img5.save("docs/media/screen_5_airspace.png")
    print("Saved screen_5_airspace.png")

    # 7. Screen 6: Battery & Energy Telemetry
    m6 = MatrixScreen()
    m6.draw_icon(0, 0, ICON_LIGHTNING)
    m6.draw_small_text(9, 1, "94%", C_GREEN)
    img6 = render_device_image(m6, "SCREEN 6 // BATTERY TELEMETRY", "4.18V LI-PO DUAL GAUGE")
    img6.save("docs/media/screen_6_battery.png")
    print("Saved screen_6_battery.png")

    # 8. Screen 7: Stopwatch & Countdown Timer
    m_timer = MatrixScreen()
    m_timer.draw_icon(0, 0, ICON_HOURGLASS)
    # Time: 04:25
    m_timer.draw_digit_3x5(9, 1, 0, C_WHITE)
    m_timer.draw_digit_3x5(13, 1, 4, C_WHITE)
    m_timer.draw_pixel(17, 2, C_WHITE)
    m_timer.draw_pixel(17, 4, C_WHITE)
    m_timer.draw_digit_3x5(19, 1, 2, C_WHITE)
    m_timer.draw_digit_3x5(23, 1, 5, C_WHITE)
    # Play green triangle badge
    m_timer.draw_pixel(28, 1, C_GREEN)
    m_timer.draw_pixel(28, 2, C_GREEN)
    m_timer.draw_pixel(28, 3, C_GREEN)
    m_timer.draw_pixel(28, 4, C_GREEN)
    m_timer.draw_pixel(28, 5, C_GREEN)
    m_timer.draw_pixel(29, 2, C_GREEN)
    m_timer.draw_pixel(29, 3, C_GREEN)
    m_timer.draw_pixel(29, 4, C_GREEN)
    m_timer.draw_pixel(30, 3, C_GREEN)
    # Bottom progress bar cyan (265s / 300s -> ~20 px)
    p_len = int(23 * (265.0 / 300.0))
    for px in range(9, 9 + p_len):
        m_timer.draw_pixel(px, 7, C_CYAN)
    img_timer = render_device_image(m_timer, "SCREEN 7 // TIMER & STOPWATCH", "PRECISION CHRONOMETER")
    img_timer.save("docs/media/screen_7_timer.png")
    print("Saved screen_7_timer.png")

    # 9. Screen 8: Ambient Screensaver (Matrix Rain / Pac-Man / Fire / Life)
    m_ss = MatrixScreen()
    m_ss.draw_pixel(1, 4, C_WHITE)   # Energizer
    m_ss.draw_pixel(4, 4, C_ORANGE)
    m_ss.draw_pixel(7, 4, C_ORANGE)
    m_ss.draw_pixel(10, 4, C_ORANGE)
    pac_bitmap = [0x1E, 0x3E, 0x3C, 0x30, 0x3C, 0x1E]
    for r in range(6):
        row = pac_bitmap[r]
        for c in range(6):
            if row & (1 << (5 - c)):
                m_ss.draw_pixel(13 + c, 1 + r, C_YELLOW)
    ghost_body = [0x1E, 0x3F, 0x3F, 0x3F, 0x3F, 0x2D]
    for r in range(6):
        row = ghost_body[r]
        for c in range(6):
            if row & (1 << (5 - c)):
                px = 23 + c
                py = 1 + r
                if r == 1 and (c == 1 or c == 4):
                    m_ss.draw_pixel(px, py, C_WHITE)
                elif r == 2 and (c == 2 or c == 5):
                    m_ss.draw_pixel(px, py, C_BLUE)
                else:
                    m_ss.draw_pixel(px, py, C_RED)
    img_ss = render_device_image(m_ss, "SCREEN 8 // AMBIENT SCREENSAVER", "MATRIX \u2022 FIRE \u2022 PAC-MAN \u2022 LIFE")
    img_ss.save("docs/media/screen_8_screensaver.png")
    print("Saved screen_8_screensaver.png")

    # 10. Screen Custom: Spotify / Notification
    m7 = MatrixScreen()
    m7.draw_icon(0, 0, ICON_SPOTIFY)
    m7.draw_small_text(9, 1, "VERIDIS", C_SPOTIFY)
    # Bottom progress bar at y=7, 65% width
    p_len = int(23 * 0.65)
    for px in range(9, 9 + p_len):
        m7.draw_pixel(px, 7, C_SPOTIFY)
    img7 = render_device_image(m7, "CUSTOM APP // SPOTIFY NOW PLAYING", "DYNAMIC NOTIFICATION API")
    img7.save("docs/media/screen_custom_spotify.png")
    print("Saved screen_custom_spotify.png")

    # 11. Multi-Screen Showcase Grid (10 screens in 5x2 layout)
    canvas_w = 5 * 760 + 50
    canvas_h = 2 * 240 + 50
    grid_img = Image.new("RGBA", (canvas_w, canvas_h), (12, 13, 16, 255))
    screens = [img0, img1, img2, img3, img4, img5, img6, img_timer, img_ss, img7]
    for idx, s_img in enumerate(screens):
        row = idx // 5
        col = idx % 5
        x = 20 + col * (760 + 5)
        y = 20 + row * (240 + 5)
        grid_img.alpha_composite(s_img, (x, y))
    
    # Scale down composite to 1500px width for fast loading in GitHub README
    comp_w = 1500
    comp_h = int(canvas_h * (comp_w / canvas_w))
    showcase = grid_img.resize((comp_w, comp_h), Image.Resampling.LANCZOS)
    showcase.save("docs/media/screens_showcase.png")
    print("Saved screens_showcase.png")

if __name__ == "__main__":
    main()
