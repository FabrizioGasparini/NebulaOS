#!/usr/bin/env python3
"""Generate a GRUB boot menu background image for NebulaOS."""

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

WIDTH, HEIGHT = 1920, 1080
BG_COLOR = (15, 10, 26)  # #0f0a1a
VIOLET = (124, 58, 237)  # #7c3aed
VIOLET_DIM = (90, 40, 180)
OUT = Path.home() / "nebulaos" / "assets" / "branding" / "nebulaos-grub.png"

FONT_PATHS = [
    "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    "/Library/Fonts/Arial Bold.ttf",
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
]


def find_font(size: int):
    for p in FONT_PATHS:
        if Path(p).exists():
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()


def draw_gradient(img):
    draw = ImageDraw.Draw(img)
    for y in range(HEIGHT):
        ratio = y / HEIGHT
        r = int(BG_COLOR[0] + (5 - BG_COLOR[0]) * ratio)
        g = int(BG_COLOR[1] + (0 - BG_COLOR[1]) * ratio)
        b = int(BG_COLOR[2] + (30 - BG_COLOR[2]) * ratio)
        draw.line([(0, y), (WIDTH, y)], fill=(r, g, b))


def draw_stars(img, count=600):
    draw = ImageDraw.Draw(img)
    rng = random.Random(42)
    for _ in range(count):
        x = rng.randint(0, WIDTH - 1)
        y = rng.randint(0, HEIGHT - 1)
        brightness = rng.randint(80, 255)
        size = rng.choice([1, 1, 1, 2])
        color = (brightness, brightness, brightness)
        if size == 1:
            draw.point((x, y), fill=color)
        else:
            draw.ellipse([x - 1, y - 1, x + 1, y + 1], fill=color)


def draw_logo(img):
    draw = ImageDraw.Draw(img)
    cx, cy = WIDTH // 2, HEIGHT // 2 - 80
    radius = 90

    # Outer glow
    for i in range(30, 0, -1):
        r = radius + i
        alpha = int(15 * (1 - i / 30))
        glow_color = (VIOLET[0], VIOLET[1], VIOLET[2])
        overlay = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
        od = ImageDraw.Draw(overlay)
        od.ellipse(
            [cx - r, cy - r, cx + r, cy + r],
            fill=(*glow_color, alpha),
        )
        img.paste(Image.alpha_composite(img.convert("RGBA"), overlay).convert("RGB"))

    # Main circle
    draw = ImageDraw.Draw(img)
    draw.ellipse(
        [cx - radius, cy - radius, cx + radius, cy + radius],
        fill=VIOLET,
        outline=VIOLET_DIM,
        width=3,
    )

    # Inner ring
    inner_r = radius - 18
    draw.ellipse(
        [cx - inner_r, cy - inner_r, cx + inner_r, cy + inner_r],
        outline=(180, 140, 255),
        width=2,
    )

    # Small center dot
    draw.ellipse([cx - 8, cy - 8, cx + 8, cy + 8], fill=(200, 170, 255))

    # Orbit ring
    orbit_r = radius + 40
    draw.ellipse(
        [cx - orbit_r, cy - orbit_r + 15, cx + orbit_r, cy + orbit_r + 15],
        outline=(VIOLET[0], VIOLET[1], VIOLET[2]),
        width=1,
    )

    # Small dot on orbit
    angle = math.radians(35)
    dot_x = cx + int(orbit_r * math.cos(angle))
    dot_y = cy + int((orbit_r - 15) * math.sin(angle)) + 15
    draw.ellipse([dot_x - 4, dot_y - 4, dot_x + 4, dot_y + 4], fill=(200, 170, 255))


def draw_text(img):
    draw = ImageDraw.Draw(img)
    font_large = find_font(64)
    text = "NebulaOS"
    bbox = draw.textbbox((0, 0), text, font=font_large)
    tw = bbox[2] - bbox[0]
    x = (WIDTH - tw) // 2
    y = HEIGHT - 140
    draw.text((x, y), text, fill=(220, 200, 255), font=font_large)


def main():
    img = Image.new("RGB", (WIDTH, HEIGHT), BG_COLOR)
    draw_gradient(img)
    draw_stars(img)
    draw_logo(img)
    draw_text(img)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    img.save(OUT, "PNG")
    print(f"Saved {OUT} ({img.size[0]}x{img.size[1]})")


if __name__ == "__main__":
    main()
