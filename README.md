# Build
Para compilar se puede utilizar el comando make. En caso de no tener este programa se puede compilar cada archivo de extensión .c con gcc.

``` gcc archivo.c -o ejecutable -lm -pthread```

# Compresor serial
- Para usar este programa se envía por parámetro el directorio con los archivos de texto (.txt) de la siguiente manera

```./compressor directorio```

# Compresor con hilos
- Para usar este programa se envía por parámetro el directorio con los archivos de texto (.txt) de la siguiente manera

```./compressor_threads directorio```

# Descompresor serial
- Para usar este programa se envía por parámetro el archivo comprimido (.huff) de la siguiente manera

```./decompressor archivo.huff```

# Descompresor con hilos
- Para usar este programa se envía por parámetro el archivo comprimido (.huff) de la siguiente manera

```./decompressor_threads archivo.huff```

# Top 100 libros de los últimos 30 días
- En la carpeta top100 se encuentran los 100 libros de los últimos 30 días en formato .txt, el nombre de cada uno representa el código que asigna la página web https://www.gutenberg.org/browse/scores/top#books-last30.
- Si se desea descargar de nuevo se puede ejecutar el programa get-books.py