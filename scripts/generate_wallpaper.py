#!/usr/bin/env python3
"""Generate NebulaOS default wallpaper - dark violet/purple nebula effect."""

from PIL import Image, ImageDraw, ImageFilter
import random
import math
import os

def create_nebula_wallpaper(width=3840, height=2160, output_path=None):
    """Create a dark nebula-themed wallpaper with purple/violet hues."""
    
    if output_path is None:
        output_path = os.path.expanduser("~/nebulaos/assets/wallpapers/nebula-default.png")
    
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # Create base image with deep space gradient
    img = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(img)
    
    # Deep space gradient - dark purple to near-black
    for y in range(height):
        ratio = y / height
        r = int(15 * (1 - ratio) + 8 * ratio)
        g = int(10 * (1 - ratio) + 5 * ratio)
        b = int(42 * (1 - ratio) + 20 * ratio)
        draw.line([(0, y), (width, y)], fill=(r, g, b))
    
    # Create nebula clouds effect
    nebula_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    nebula_draw = ImageDraw.Draw(nebula_layer)
    
    nebula_colors = [
        (124, 58, 237, 30),
        (168, 85, 247, 25),
        (139, 92, 246, 20),
        (91, 33, 182, 35),
        (79, 70, 229, 15),
        (192, 132, 252, 18),
    ]
    
    random.seed(42)
    
    for _ in range(15):
        cx = random.randint(0, width)
        cy = random.randint(0, height)
        radius = random.randint(200, 800)
        color = random.choice(nebula_colors)
        
        for r in range(radius, 0, -3):
            alpha = int(color[3] * (r / radius) ** 0.5)
            current_color = (color[0], color[1], color[2], alpha)
            nebula_draw.ellipse(
                [cx - r, cy - r, cx + r, cy + r],
                fill=current_color
            )
    
    # Bright central nebula core
    core_x, core_y = width // 2, height // 2
    for r in range(400, 0, -2):
        alpha = int(40 * (r / 400) ** 0.3)
        nebula_draw.ellipse(
            [core_x - r, core_y - r, core_x + r, core_y + r],
            fill=(168, 85, 247, alpha)
        )
    
    nebula_layer = nebula_layer.filter(ImageFilter.GaussianBlur(radius=80))
    img.paste(Image.alpha_composite(img.convert("RGBA"), nebula_layer).convert("RGB"))
    
    # Add stars
    star_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    star_draw = ImageDraw.Draw(star_layer)
    
    for _ in range(600):
        sx = random.randint(0, width)
        sy = random.randint(0, height)
        brightness = random.randint(150, 255)
        size = random.choice([1, 1, 1, 1, 2, 2, 3])
        alpha = random.randint(100, 255)
        
        if size == 1:
            star_draw.point((sx, sy), fill=(brightness, brightness, brightness, alpha))
        else:
            star_draw.ellipse(
                [sx - size, sy - size, sx + size, sy + size],
                fill=(brightness, brightness, brightness, alpha)
            )
    
    # Brighter stars with glow
    for _ in range(20):
        sx = random.randint(0, width)
        sy = random.randint(0, height)
        for r in range(6, 0, -1):
            alpha = int(200 * (r / 6))
            star_draw.ellipse(
                [sx - r, sy - r, sx + r, sy + r],
                fill=(220, 210, 255, alpha)
            )
    
    star_layer = star_layer.filter(ImageFilter.GaussianBlur(radius=1))
    img.paste(Image.alpha_composite(img.convert("RGBA"), star_layer).convert("RGB"))
    
    # Vignette effect
    vignette = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    vignette_draw = ImageDraw.Draw(vignette)
    
    for r in range(max(width, height), 0, -2):
        ratio = r / max(width, height)
        alpha = int(120 * (1 - ratio) ** 2)
        vignette_draw.ellipse(
            [width//2 - r, height//2 - r, width//2 + r, height//2 + r],
            fill=(0, 0, 0, alpha)
        )
    
    img.paste(Image.alpha_composite(img.convert("RGBA"), vignette).convert("RGB"))
    
    # Subtle light rays from center
    ray_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    ray_draw = ImageDraw.Draw(ray_layer)
    
    for angle in range(0, 360, 30):
        rad = math.radians(angle + random.randint(-5, 5))
        length = random.randint(400, 900)
        x2 = core_x + int(length * math.cos(rad))
        y2 = core_y + int(length * math.sin(rad))
        
        for w in range(15, 0, -1):
            alpha = int(8 * (w / 15))
            ray_draw.line(
                [(core_x, core_y), (x2, y2)],
                fill=(168, 130, 247, alpha),
                width=w
            )
    
    ray_layer = ray_layer.filter(ImageFilter.GaussianBlur(radius=30))
    img.paste(Image.alpha_composite(img.convert("RGBA"), ray_layer).convert("RGB"))
    
    img.save(output_path, "PNG", quality=95)
    print(f"Wallpaper saved to: {output_path}")
    print(f"Size: {width}x{height}")
    return output_path

if __name__ == "__main__":
    create_nebula_wallpaper(3840, 2160)
    create_nebula_wallpaper(1920, 1080,
        os.path.expanduser("~/nebulaos/assets/wallpapers/nebula-default-1080p.png"))
    create_nebula_wallpaper(2560, 1440,
        os.path.expanduser("~/nebulaos/assets/wallpapers/nebula-default-1440p.png"))
