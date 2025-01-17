# 🪙 Token counter and 📌prompt creator
### Overwiew
Create one text file prompts and then count tokens in it. For local repos. Use `main64.exe` (it’s source is `cpp.cpp`) for creating `prompt.txt` file. Then use main.py (requires OpenAI’s package `tiktoken`) to count tokens.

### Build C++ command used for create `main64.exe`
```bash
x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o main64.exe cpp.cpp
```

# Example
### [Linux Kernel](https://github.com/torvalds/linux) number of tokens as of Jan. 2025


```bash
Number of tokens: 456 479 607
```
