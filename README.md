**!! This repo is a subrepo to [xD-DOS](https://github.com/jasonchristiandev/xD-DOS.git)!**

# Extended Drive - Disk Operating System (xD-DOS)

xD-DOS is an operating system inspired by **DOS**. xD-DOS is merely a fun project and **SHOULD NOT** be used for serious terms.

* **Inspired, not rebuilt:** xD-DOS does not aim to be a remake of DOS, but only inspired by DOS.
* **Modular:** Designed with every component of the OS being separate applications instead of one big kernel.

## To-Do List

* ~~Configure limine~~
* ~~Set up kernel entry point~~
* ~~Setup PMM, VMM, and VMA~~
* ~~Setup malloc and free~~
* ~~Write to framebuffer~~
* ~~PSF graphics~~
* ~~Interrupt handling~~
* ~~Basic input~~
* ~~GDT~~
* Rewrite vibe-coded parts to better understand what I'm doing

## Vibe-Coded Parts
* psf.c/h
* syscallhandler.c/h
* graphics.c/h

## Getting Started

### Prerequisites
* make
* binutils
* binutils-mingw-w64 (for bootloader)
* gnu-efi (includes)
* qemu-system-x86_64 (optional for emulating)
* edk2-ovmf (optional for emulating)

### Installation / Compilation
1. **Clone the repository**:
```bash
git clone --recursive https://github.com/jasonchristiandev/xD-DOS.git
```

2. **Build the OS**:
```bash
cd xD-DOS
make -j$(nproc) # or
make -j$(nproc) VERBOSE=1
```

3. **Run the OS using qemu-system-x86_64 (optional)**:
```bash
cp /usr/share/OVMF/OVMF.fd ./OVMF.fd # one time setup, path may differ
make -j$(nproc) run
```

## License

xD-DOS is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more detail.

## Contributing

Contributions are encouraged, due to my lack of experience in OS dev :p

### How to Contribute

There are many ways to contribute, such as but not limited to:

- Submit bugs and feature requests
- Review commits/source code changes
