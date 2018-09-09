## Formatting and style
This module has automatic travis check for code style provided by Travis CI.
So it is recommended to check any source code changes locally using `clang-format`.
Just run:
`./clang-format.sh`

You might also integrate `clang-format` into your editor if you haven't already:
- Emacs: http://clang.llvm.org/docs/ClangFormat.html
- SublimeText: https://github.com/rosshemsley/SublimeClangFormat
- Vim: https://github.com/rhysd/vim-clang-format
- Visual Studio Code: Install Microsoft support for C/CPP, (For Windows 10+WSL) add clang-format path in settings.json (if clang-format is installed in WSL:

    //clang module
    "clang-format.executable": "wsl.exe clang-format",
    "editor.formatOnSave": true
- Visual Studio: http://llvm.org/builds/, or use the integrated support in Visual Studio 2017
- Xcode: https://github.com/travisjeffery/ClangFormat-Xcode
