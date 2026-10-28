
--------------------------------------------------FILEOGRE--------------------------------------------------

A command line tool for sorting files into subdirectories based on what type they are (Images, Videos, Audio, Documents, ...).

This tool allows users to sort files into predefined categories based on file extension.


-BEFORE-

└── root/
    ├── src/
    │   ├── example.jpg
    │   ├── example2.webp
    │   ├── example3.pdf
    │   ├── sub1/
    │   │   ├── example4.txt
    │   │   └── example5.mpv
    │   └── sub2/
    │       └── example6.wav
    └── dest/


-AFTER-

└── root/
    ├── src/
    │   ├── sub1/
    │   └── sub2/
    └── dest/
        ├── images/
        │   ├── example.jpg
        │   └── example2.webp
        ├── documents/
        │   ├── example3.pdf
        │   └── example4.txt
        ├── videos/
        │   └── example5.mpv
        └── audio/
            └── example6.wav
