from PIL import Image, ImageDraw
import os


def draw_symbol(draw, size, s):
    stroke = "#2EE6D6"
    stroke_w = max(2, int(8 * s))
    radius = 18 * s
    back = [size * 0.35, size * 0.32, size * 0.65, size * 0.62]
    front = [size * 0.29, size * 0.38, size * 0.59, size * 0.68]

    draw.rounded_rectangle(
        back,
        radius=radius,
        outline=stroke,
        width=stroke_w,
    )
    draw.rounded_rectangle(
        front,
        radius=radius,
        outline=stroke,
        width=stroke_w,
    )

    draw.line(
        [
            (size * 0.43, size * 0.48),
            (size * 0.57, size * 0.58),
        ],
        fill=stroke,
        width=max(2, int(6 * s)),
    )
    draw.line(
        [
            (size * 0.50, size * 0.47),
            (size * 0.57, size * 0.58),
            (size * 0.57, size * 0.47),
        ],
        fill=stroke,
        width=max(2, int(6 * s)),
        joint="curve",
    )


def create_icon(size, scale=4):
    canvas = size * scale
    work = Image.new("RGBA", (canvas, canvas), (0, 0, 0, 0))
    draw = ImageDraw.Draw(work)
    s = canvas / 256.0
    pad = 10 * s
    corner = 54 * s

    draw.rounded_rectangle(
        [pad, pad, canvas - pad, canvas - pad],
        radius=corner,
        fill="#113E6A",
        outline="#D6E0EA",
        width=max(2, int(2.5 * s)),
    )

    draw_symbol(draw, canvas, s)
    return work.resize((size, size), Image.Resampling.LANCZOS)


def main():
    sizes = [16, 24, 32, 48, 64, 128, 256]
    images = []
    for sz in sizes:
        img = create_icon(sz)
        images.append(img)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    ico_path = os.path.join(script_dir, "app.ico")
    png_path = os.path.join(script_dir, "app_256.png")

    icon_sizes = [(img.width, img.height) for img in images]

    img_256 = images[-1].copy()
    img_256.save(
        ico_path,
        format="ICO",
        sizes=icon_sizes,
        append_images=images[:-1],
    )
    print(f"ICO saved to {ico_path}")

    img_256.save(png_path, format="PNG")
    print(f"PNG saved to {png_path}")

    verify = Image.open(ico_path)
    print(f"Verification - ICO embedded sizes: {verify.info.get('sizes', set())}")
    print("Done!")


if __name__ == "__main__":
    main()
