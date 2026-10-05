#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SD.h>

#define LOAD_SD_LIBRARY
#include <JPEGDecoder.h>

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define TFT_BL 27

#define MAX_IMAGES 100
#define AUTOPLAY_INTERVAL 3000UL
#define SERIAL_BAUD 115200

TFT_eSPI tft = TFT_eSPI();

String imageNames[MAX_IMAGES];
int imageCount = 0;
int currentImageIndex = 0;
bool autoplay = false;
unsigned long lastAutoplayTime = 0;

void showError(const char* title, const char* line1, const char* line2 = nullptr) {
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 110);
  tft.println(title);

  tft.setTextSize(1);
  tft.setCursor(20, 180);
  tft.println(line1);

  if (line2 != nullptr) {
    tft.setCursor(20, 200);
    tft.println(line2);
  }
}

void renderJPEGToScreen() {
  uint16_t* pImg = JpegDec.pImage;

  const int mcuW = JpegDec.MCUWidth;
  const int mcuH = JpegDec.MCUHeight;

  const int imageW = JpegDec.width;
  const int imageH = JpegDec.height;

  const int screenW = tft.width();
  const int screenH = tft.height();

  int offsetX = 0;
  int offsetY = 0;

  if (imageW < screenW) {
    offsetX = (screenW - imageW) / 2;
  }

  if (imageH < screenH) {
    offsetY = (screenH - imageH) / 2;
  }

  uint16_t clippedBuffer[16 * 16];

  while (JpegDec.read()) {
    int x = JpegDec.MCUx * mcuW;
    int y = JpegDec.MCUy * mcuH;

    int drawX = x + offsetX;
    int drawY = y + offsetY;

    int validW = min(mcuW, imageW - x);
    int validH = min(mcuH, imageH - y);

    if (validW <= 0 || validH <= 0) {
      continue;
    }

    int srcX = 0;
    int srcY = 0;

    if (drawX < 0) {
      srcX = -drawX;
      validW -= srcX;
      drawX = 0;
    }

    if (drawY < 0) {
      srcY = -drawY;
      validH -= srcY;
      drawY = 0;
    }

    if (drawX >= screenW ||
        drawY >= screenH ||
        validW <= 0 ||
        validH <= 0) {
      continue;
    }

    if (drawX + validW > screenW) {
      validW = screenW - drawX;
    }

    if (drawY + validH > screenH) {
      validH = screenH - drawY;
    }

    if (validW <= 0 || validH <= 0) {
      continue;
    }

    if (srcX == 0 &&
        srcY == 0 &&
        validW == mcuW &&
        validH == mcuH) {

      tft.pushImage(
        drawX,
        drawY,
        validW,
        validH,
        pImg
      );

      continue;
    }

    if (validW > 16 || validH > 16) {
      continue;
    }

    for (int row = 0; row < validH; row++) {
      for (int col = 0; col < validW; col++) {
        clippedBuffer[row * validW + col] =
          pImg[(srcY + row) * mcuW + (srcX + col)];
      }
    }

    tft.pushImage(
      drawX,
      drawY,
      validW,
      validH,
      clippedBuffer
    );
  }
}

bool renderJPEG(const char* filename) {
  Serial.print("Decodificando: ");
  Serial.println(filename);

  File file = SD.open(filename, FILE_READ);

  if (!file) {
    Serial.println("ERROR: no se pudo abrir el archivo JPG");

    showError(
      "ERROR JPG",
      "No se pudo abrir:",
      filename
    );

    return false;
  }

  size_t fileSize = file.size();

  file.close();

  if (fileSize == 0) {
    Serial.println("ERROR: archivo JPG vacio");

    showError(
      "ERROR JPG",
      "Archivo vacio:",
      filename
    );

    return false;
  }

  tft.fillScreen(TFT_BLACK);

  int decoderResult = JpegDec.decodeSdFile(filename);

  if (decoderResult < 0 ||
      JpegDec.width <= 0 ||
      JpegDec.height <= 0) {

    Serial.print("ERROR: JPEGDecoder no pudo decodificar. Codigo: ");
    Serial.println(decoderResult);

    showError(
      "ERROR JPG",
      "Formato no compatible",
      "Use JPG RGB 24-bit no progresivo"
    );

    return false;
  }

  Serial.print("Resolucion: ");
  Serial.print(JpegDec.width);
  Serial.print("x");
  Serial.println(JpegDec.height);

  renderJPEGToScreen();

  return true;
}

void findImages(const String& path) {
  File dir = SD.open(path);

  if (!dir || !dir.isDirectory()) {
    Serial.print("No se puede abrir: ");
    Serial.println(path);
    return;
  }

  File entry = dir.openNextFile();

  while (entry && imageCount < MAX_IMAGES) {

    if (!entry.isDirectory()) {

      String originalName = String(entry.name());
      String lowerName = originalName;

      lowerName.toLowerCase();

      if (lowerName.endsWith(".jpg") ||
          lowerName.endsWith(".jpeg")) {

        String fullPath;

        if (path == "/") {

          if (originalName.startsWith("/")) {
            fullPath = originalName;
          } else {
            fullPath = "/" + originalName;
          }

        } else {

          if (originalName.startsWith("/")) {
            fullPath = originalName;
          } else {
            fullPath = path + "/" + originalName;
          }
        }

        imageNames[imageCount] = fullPath;

        Serial.print("[");
        Serial.print(imageCount);
        Serial.print("] ");
        Serial.println(imageNames[imageCount]);

        imageCount++;
      }
    }

    entry = dir.openNextFile();
  }

  entry.close();
  dir.close();
}

void listAllImages() {
  Serial.println();
  Serial.println("===== IMAGENES EN SD =====");

  for (int i = 0; i < imageCount; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(imageNames[i]);
  }

  Serial.println("==========================");
  Serial.println();
}

void showInfo() {
  Serial.println();
  Serial.println("===== INFORMACION =====");

  Serial.print("Imagenes: ");
  Serial.println(imageCount);

  Serial.print("Actual: ");
  Serial.print(currentImageIndex);
  Serial.print(" -> ");

  if (imageCount > 0) {
    Serial.println(imageNames[currentImageIndex]);
  } else {
    Serial.println("ninguna");
  }

  Serial.print("Pantalla: ");
  Serial.print(tft.width());
  Serial.print("x");
  Serial.println(tft.height());

  Serial.println("Hardware: ESP32-3248S035");
  Serial.println("TFT configurada mediante TFT_eSPI");
  Serial.println("SD CS: GPIO 5");
  Serial.println("SD SPI: SCK 18 / MISO 19 / MOSI 23");

  Serial.println();
  Serial.println("Comandos:");
  Serial.println("n = siguiente");
  Serial.println("p = anterior");
  Serial.println("l = listar imagenes");
  Serial.println("i = informacion");
  Serial.println("sN = ir al indice N");
  Serial.println("a = autoplay ON");
  Serial.println("t = autoplay OFF");

  Serial.println("=======================");
  Serial.println();
}

void showImage(int index) {
  if (imageCount == 0) {
    return;
  }

  if (index < 0) {
    index = 0;
  }

  if (index >= imageCount) {
    index = imageCount - 1;
  }

  currentImageIndex = index;

  Serial.println();
  Serial.print("Mostrando [");
  Serial.print(currentImageIndex);
  Serial.print("/");
  Serial.print(imageCount - 1);
  Serial.print("] ");

  Serial.println(imageNames[currentImageIndex]);

  if (!renderJPEG(imageNames[currentImageIndex].c_str())) {
    Serial.println("No se pudo mostrar la imagen.");
  }
}

bool initSD() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.begin(
    SD_SCK,
    SD_MISO,
    SD_MOSI,
    SD_CS
  );

  if (!SD.begin(
        SD_CS,
        SPI,
        20000000
      )) {

    return false;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    return false;
  }

  Serial.print("Tipo SD: ");

  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  }
  else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSizeMB =
    SD.cardSize() / (1024ULL * 1024ULL);

  Serial.print("Capacidad: ");
  Serial.print((unsigned long long)cardSizeMB);
  Serial.println(" MB");

  return true;
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  delay(500);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();

  tft.setRotation(0);

  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(
    TFT_CYAN,
    TFT_BLACK
  );

  tft.setTextSize(2);

  tft.setCursor(35, 180);

  tft.println("AgroMetric");

  tft.setTextSize(1);

  tft.setCursor(35, 220);

  tft.println("Inicializando SD...");

  Serial.println();
  Serial.println("========================================");
  Serial.println(" ESP32-3248S035 JPG VIEWER CORREGIDO");
  Serial.println("========================================");

  Serial.println("Inicializando SD...");

  if (!initSD()) {

    Serial.println(
      "ERROR: no se pudo inicializar la microSD."
    );

    showError(
      "ERROR SD",
      "Revisa la tarjeta",
      "GPIO CS=5  SPI=18/19/23"
    );

    while (true) {
      delay(1000);
    }
  }

  Serial.println(
    "SD inicializada correctamente."
  );

  imageCount = 0;

  Serial.println(
    "Buscando JPG en /images ..."
  );

  findImages("/images");

  if (imageCount == 0) {

    Serial.println(
      "No hay JPG en /images. Buscando en raiz..."
    );

    findImages("/");
  }

  Serial.print(
    "Total JPG encontrados: "
  );

  Serial.println(imageCount);

  if (imageCount == 0) {

    showError(
      "NO HAY JPG",
      "Crea /images/ en la SD",
      "y copia archivos .jpg"
    );

    while (true) {
      delay(1000);
    }
  }

  listAllImages();

  showImage(0);

  Serial.println(
    "Sistema listo."
  );

  Serial.println(
    "Escribe 'i' para ver los comandos."
  );
}

void loop() {

  if (autoplay && imageCount > 0) {

    if (
      millis() - lastAutoplayTime >=
      AUTOPLAY_INTERVAL
    ) {

      int nextIndex =
        currentImageIndex + 1;

      if (nextIndex >= imageCount) {
        nextIndex = 0;
      }

      showImage(nextIndex);

      lastAutoplayTime = millis();
    }
  }

  if (Serial.available()) {

    String input =
      Serial.readStringUntil('\n');

    input.trim();
    input.toLowerCase();

    if (input.length() == 0) {
      return;
    }

    char command =
      input.charAt(0);

    switch (command) {

      case 'n':

        if (imageCount > 0) {

          int nextIndex =
            currentImageIndex + 1;

          if (nextIndex >= imageCount) {
            nextIndex = 0;
          }

          showImage(nextIndex);
        }

        break;

      case 'p':

        if (imageCount > 0) {

          int prevIndex =
            currentImageIndex - 1;

          if (prevIndex < 0) {
            prevIndex = imageCount - 1;
          }

          showImage(prevIndex);
        }

        break;

      case 'l':

        listAllImages();

        break;

      case 'i':

        showInfo();

        break;

      case 's':

        if (
          input.length() > 1 &&
          imageCount > 0
        ) {

          int index =
            input.substring(1).toInt();

          if (
            index >= 0 &&
            index < imageCount
          ) {

            showImage(index);

          } else {

            Serial.print(
              "Indice invalido. Rango: 0-"
            );

            Serial.println(
              imageCount - 1
            );
          }
        }

        break;

      case 'a':

        autoplay = true;

        lastAutoplayTime =
          millis();

        Serial.println(
          "Autoplay ON"
        );

        break;

      case 't':

        autoplay = false;

        Serial.println(
          "Autoplay OFF"
        );

        break;

      default:

        Serial.println(
          "Comando desconocido. Usa 'i'."
        );

        break;
    }
  }

  delay(20);
}