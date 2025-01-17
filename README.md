# 🪙 Token counter and 📌prompt creator
### Overwiew
This repository is *extremely poorly* made piece of software. Create one text file prompts from codebases and then count tokens in it. For local repositories. Use `main64.exe ‹directory_name›` (C++ source of executable is `cpp.cpp`) for creating `prompt.txt` file. Then use `main.py` (requires OpenAI’s package `tiktoken`) to count tokens. 

### Goal
Blazingly fast count tokens in *large* codebases like [Linux Kernel](https://github.com/torvalds/linux) and create large text prompts for it. For fun.

### How it’s made
All code is generated by [DeepSeek](https://chat.deepseek.com/). With insignificant edits of Python code by `CTRL+C` and `CTRL+V` from [OpenAI’s Cookbook](https://cookbook.openai.com/examples/how_to_count_tokens_with_tiktoken) and DeepSeek’s tips. C++ code is multithreaded. The C++ part creates large `prompt.txt` file with file tree and content of all files and subdirectories in directory. The Python part just uses OpenAI’s tokenizer.

### Build C++ command used for create `main64.exe`
```bash
x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o main64.exe cpp.cpp
```
# Example
### [Linux Kernel](https://github.com/torvalds/linux) number of tokens as of Jan. 2025

```
Number of tokens: 456 479 607
```

Number of lines in `prompt.txt`: `40 227 546`

`prompt.txt` file size: `1.42 GB`

