# Shamigo

**Shamigo** is a steganographic utility that combines [Shamir's Secret Sharing Scheme](https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing) with 1-bit Least Significant Bit (LSB) image steganography.

## Features

- **Distribute** a secret BMP image by splitting it into `n` shadows and hiding them in `n` cover BMP images using LSB steganography.
- **Recover** the original secret BMP image using at least `k` out of the `n` shadow-containing cover BMP images.
- Secure and robust via threshold-based reconstruction using Shamir’s Secret Sharing.



## Usage

```bash
./shamigo [--d | --r] --secret <file> --k <num> [--n <num>] [--dir <directory>]
```

### Required Parameters

| Flag        | Description                                                                 |
|-------------|-----------------------------------------------------------------------------|
| `--d`       | **Distribute mode**: hide the secret image into `n` cover images.           |
| `--r`       | **Recover mode**: reconstruct the secret image from stego images.           |
| `--secret`  | Path to the secret image (in **distribute**) or output file name (in **recover**). |
| `--k`       | Minimum number of shares (2–10) required to recover the image.              |

### Optional Parameters

| Flag        | Description                                                                 |
|-------------|-----------------------------------------------------------------------------|
| `--n`       | Number of shares to generate (must be ≥ `k`) in distribute mode. Defaults to the number of images in the directory if omitted. Does not change anything in recover mode. |
| `--dir`     | Directory of cover images. Defaults to current directory if missing.                  |

---

## Examples

### Distribute a secret

```bash
./shamigo --d --secret secret.bmp --k 3 --n 5 --dir ./covers
```

- Hides `secret.bmp` into 5 cover images in `./covers`
- Any 3 of them are sufficient to recover the secret

### Recover the secret

```bash
./shamigo --r --secret output.bmp --k 3 --dir ./covers
```

- Recovers the original image as `output.bmp` using any 3 valid stego images in `./covers`


## Notes

- The number of cover images in the directory must be at least `n` in distribute mode.
- The `--n` parameter is not required in recover mode.
- Only indexed mode 8bpp color depth BMP files are supported.
- The implementation uses 1-bit LSB steganography; image quality remains largely unaffected.


## Error Handling

- If neither `--d` nor `--r` is specified, or both are used at once, the program exits with an error.
- If `--secret` or `--k` is missing, or `k` is invalid, execution halts with a descriptive message.
- If `n` is set but is less than `k`, an error is raised.


## Build & Run

To compile:

```bash
make clean all
```

Then execute as shown in the examples above.



## Authors

* Juan Ignacio Fernández Dinardo - 62466
* Gianfranco Magliotti - 61172

