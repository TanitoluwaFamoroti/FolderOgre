
# FolderOgre

FolderOgre is a small command-line utility for organizing files in a source folder into destination subfolders based on their type or metadata. It is designed for quick, rule-based sorting without needing a GUI.

The tool currently supports sorting by:
- file extension
- file size
- creation date
- last modification date
- last access date

It moves matching files from a source folder into a destination folder, creating destination directories automatically when needed.

## What it does

FolderOgre scans the files directly inside the source folder and moves them into category folders such as:
- images
- videos
- audio
- documents
- emails
- archives
- executables

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

This produces the executable in the build directory, typically:

```bash
./build/folderogre
```

## Usage

```bash
./build/folderogre <source_folder> <destination_folder> [options...]
```

### General rules
- The first argument is the source folder.
- The second argument is the destination folder.
- The program expects the source to already exist and to be a directory.
- The program moves files from the source folder into the destination folder.
- Files are checked in the source root only; nested directories are not recursively traversed in this version.

## Sort modes

### 1) Sort by file extension

```bash
./build/folderogre ./in ./out -e
```

This groups files by extension using the built-in mapping table.

Example categories:
- .jpg, .jpeg, .png, .webp -> images
- .mp4, .mov, .mkv -> videos
- .mp3, .wav, .flac -> audio
- .pdf, .txt, .docx, .csv -> documents

### 2) Sort by size

```bash
./build/folderogre ./in ./out -s <comparison> <size_in_kb>
```

Comparison values:
- `lt` = less than
- `gt` = greater than or equal to

Examples:
```bash
./build/folderogre ./in ./out -s lt 2048
./build/folderogre ./in ./out -s gt 2048
```

This creates folders such as:
- `Size less than 2048KB`
- `Size greater than 2048KB`

### 3) Sort by creation date

```bash
./build/folderogre ./in ./out -c <comparison> <dd/mm/yyyy>
```

Comparison values:
- `lt` = before the given date
- `eq` = on the given date
- `gt` = after the given date

Example:
```bash
./build/folderogre ./in ./out -c eq 12/04/2024
```

This creates folders like:
- `Created on 12/04/2024`
- `Created before 12/04/2024`
- `Created after 12/04/2024`

### 4) Sort by last modification date

```bash
./build/folderogre ./in ./out -m <comparison> <dd/mm/yyyy>
```

Comparison values:
- `lt`
- `eq`
- `gt`

Example:
```bash
./build/folderogre ./in ./out -m gt 01/01/2025
```

This creates folders such as:
- `Last modified after 01/01/2025`
- `Last modified before 01/01/2025`
- `Last modified on 01/01/2025`

### 5) Sort by last access date

```bash
./build/folderogre ./in ./out -a <comparison> <dd/mm/yyyy>
```

Comparison values:
- `lt`
- `eq`
- `gt`

Example:
```bash
./build/folderogre ./in ./out -a lt 30/06/2024
```

This creates folders such as:
- `Last accessed before 30/06/2024`
- `Last accessed on 30/06/2024`
- `Last accessed after 30/06/2024`

### 6) Help

```bash
./build/folderogre ./in ./out -h
```

Shows the command usage and supported options.

## Additional flags

- `-u` moves files with an unrecognized extension into `out/unknown/`. Use it with extension sorting (`-e`); place it after `-e`:

  ```bash
  ./build/folderogre ./in ./out -e -u
  ```

- `-f` moves hidden files (filenames beginning with `.`) into `out/.hidden/`:

  ```bash
  ./build/folderogre ./in ./out -e -f
  ```

```bash
./build/folderogre ./in ./out -e -u -f
```

## Examples

### Example A: sort by extension

```text
before/
├── in/
│   ├── photo.jpg
│   ├── cover.png
│   ├── notes.txt
│   ├── song.mp3
│   └── clip.mov
└── out/

./build/folderogre ./in ./out -e
```

```text
after/
├── in/
│   └── (empty)
└── out/
    ├── images/
    │   ├── photo.jpg
    │   └── cover.png
    ├── documents/
    │   └── notes.txt
    ├── audio/
    │   └── song.mp3
    └── videos/
        └── clip.mov
```

### Example B: sort by size

```text
before/
├── in/
│   ├── small.txt
│   ├── big-video.mp4
│   └── medium.pdf
└── out/

./build/folderogre ./in ./out -s gt 2048
```

```text
after/
├── in/
│   └── (empty)
└── out/
    └── Size greater than 2048KB/
        ├── big-video.mp4
        └── medium.pdf
```

### Example C: sort by creation date

```text
before/
├── in/
│   ├── report.pdf
│   ├── archive.zip
│   └── old_photo.jpg
└── out/

./build/folderogre ./in ./out -c eq 12/04/2024
```

```text
after/
├── in/
│   └── (empty)
└── out/
    └── Created on 12/04/2024/
        ├── report.pdf
        └── archive.zip
```

## Notes

- Date input format is `dd/mm/yyyy`.
- Size input is interpreted in kilobytes.
- The tool creates destination folders automatically.
- Only direct files in the source folder are considered; it does not walk nested directories in this version.
- The program prints a message if no files were sorted.

## Summary

FolderOgre is a simple organizer for moving files into categorized folders using a few command-line rules. It is best suited for quick cleanup jobs, personal file organization, and lightweight file batching workflows.
