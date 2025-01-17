import tiktoken

def count_tokens(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    encoding = tiktoken.encoding_for_model('gpt-4o')  # Используется в GPT-4
    tokens = encoding.encode(content)
    return len(tokens)

token_count = count_tokens("prompt.txt")
print(f"Number of tokens: {token_count}")