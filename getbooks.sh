#!/bin/bash
# Descarga los Top 100 EBooks de Project Gutenberg (últimos 30 días)
# Usa wget para descargar los archivos .txt.utf-8 con nombres significativos

# Crear carpeta de salida
mkdir -p gutenberg_top100
cd gutenberg_top100

# URL de la página con los Top 100 (últimos 30 días)
TOP_PAGE="https://www.gutenberg.org/browse/scores/top#books-last30"

echo "🚀 Descargando página de Top 100 EBooks (últimos 30 días)..."
curl -s "$TOP_PAGE" > top_page.html

# Extraer los IDs y títulos usando grep y sed
echo "🔍 Extrayendo IDs y títulos de los libros..."
grep -o 'href="/ebooks/[0-9]*"[^>]*>[^<]*</a>' top_page.html | \
  grep -o 'href="/ebooks/[0-9]*" >[^<]*' | \
  sed 's|href="/ebooks/\([0-9]*\)">\(.*\)</a>|\1|\2|' | \
  sed 's/^[[:space:]]*//;s/[[:space:]]*$//' | \
  head -n 100 > book_list.txt

# Verificar que se encontraron libros
if [ ! -s book_list.txt ]; then
    echo "❌ No se encontraron libros. Revisa la estructura de la página."
    exit 1
fi

echo "✅ Se encontraron $(wc -l < book_list.txt) libros."

# Descargar cada libro con wget
echo "📥 Descargando libros..."

while IFS='|' read -r book_id title; do
    # Limpiar título para usarlo en nombre de archivo
    safe_title=$(echo "$title" | sed 's/[^a-zA-Z0-9_\-\. ]//g' | sed 's/  */ /g' | tr ' ' '_' | cut -c1-50)
    
    # URL del archivo UTF-8
    url="https://www.gutenberg.org/ebooks/${book_id}.txt.utf-8"
    
    echo "   📖 Descargando: $title (ID: $book_id)"
    
    # Descargar con wget, esperar 2 segundos entre descargas (para no sobrecargar el servidor)
    wget --quiet --wait=2 --user-agent="Mozilla/5.0" -O "${book_id}_${safe_title}.txt" "$url"
    
    if [ $? -eq 0 ]; then
        echo "      ✅ Descargado: ${book_id}_${safe_title}.txt"
    else
        echo "      ❌ Error descargando: $title"
    fi
    
done < book_list.txt

echo ""
echo "🎉 ¡Descarga completa!"
echo "📁 Archivos guardados en: $(pwd)"
echo "📊 Total descargados: $(ls -1 *.txt 2>/dev/null | wc -l)"
echo "🔗 Página fuente: $TOP_PAGE"

# Limpiar archivo temporal
rm -f top_page.html book_list.txt
