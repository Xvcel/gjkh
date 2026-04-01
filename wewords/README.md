# WeWords — Vocabulary Flashcard App

Oxford 5000 (4,993 үг) + HSK 1-6 (4,998 үг) Монгол орчуулгатай flashcard апп.

## ⬇️ Download (.exe)

1. Дээрх **Actions** tab дарна
2. Хамгийн сүүлийн build дарна
3. **WeWords-Windows** artifact татна → ZIP задлаад `WeWords.exe` ажиллуулна

## 🔨 Build from source

```bash
# Qt6 шаардлагатай
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2022_64"
cmake --build build --config Release
cd build/Release && windeployqt WeWords.exe
```

## ⌨️ Товчлол

| Товч | Үйлдэл |
|------|--------|
| `Space` | Карт хөрвүүлэх |
| `K` | Known — сурсан |
| `H` | Hard — хүнд |
| `← →` | Өмнөх / дараагийн |
| `Esc` | Гарах |
