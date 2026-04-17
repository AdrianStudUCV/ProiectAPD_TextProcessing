import os

def generate_large_file(file_name, target_size_mb):
    # Cuvinte de test (amestec de cuvinte normale si stop words)
    words = "the quick brown fox jumps over the lazy dog and is a very fast animal si de la pentru proiect algoritmi paraleli "
    
    target_size_bytes = target_size_mb * 1024 * 1024
    current_size = 0
    
    print(f"Generez fisierul de {target_size_mb} MB...")
    
    with open(file_name, "w") as f:
        while current_size < target_size_bytes:
            f.write(words)
            current_size += len(words)
            
    print(f"Gata! Fisierul '{file_name}' a fost creat.")
generate_large_file("test_1mb.txt", 1)
generate_large_file("test_10mb.txt", 10)
generate_large_file("test_50mb.txt", 50)
generate_large_file("test_100mb.txt", 100)
generate_large_file("test_200mb.txt", 200)
generate_large_file("test_400mb.txt", 400)
generate_large_file("test_1000mb.txt", 1000)