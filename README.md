# ESP32-3248S035 (CYD 3.5") - Configuración Personal

> **Estado:** ✅ Configuración verificada y funcional.
>
> **Fecha:** ___________
>
> **Notas:** Esta configuración fue validada en mi placa. En mi caso el controlador de la pantalla **NO** es ST7796 sino **ILI9488**.

---

# Información General

| Característica | Valor |
|---------------|-------|
| Placa | ESP32-3248S035 |
| Tipo | Cheap Yellow Display (CYD) |
| Tamaño de pantalla | 3.5" TFT |
| Resolución | 320 × 480 píxeles |
| Controlador TFT | ILI9488 |
| Comunicación | SPI |
| Librería utilizada | TFT_eSPI |

---

# Configuración del Arduino IDE

| Opción | Valor |
|---------|-------|
| Board | ESP32 Dev Module |
| CPU Frequency | 240 MHz |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | Según la memoria de la placa (4 MB / 16 MB) |
| Partition Scheme | Huge APP |
| Upload Speed | 921600 |

---

# Driver de la Pantalla

Activar únicamente este driver:

```cpp
#define ILI9488_DRIVER
```

Desactivar cualquier otro:

```cpp
// #define ST7796_DRIVER
// #define ILI9341_DRIVER
// #define ST7735_DRIVER
```

---

# Resolución

```cpp
#define TFT_WIDTH  320
#define TFT_HEIGHT 480
```

---

# Pines SPI

```cpp
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
```

---

# Pines de Control

```cpp
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
```

> Si en algún proyecto el reset presenta problemas, probar:

```cpp
#define TFT_RST 27
```

---

# Backlight

GPIO utilizado:

```cpp
#define TFT_BL 27
```

Inicialización:

```cpp
pinMode(TFT_BL, OUTPUT);
digitalWrite(TFT_BL, HIGH);
```

---

# Frecuencia SPI

Configuración estable:

```cpp
#define SPI_FREQUENCY 40000000
```

Opcional (si la pantalla funciona correctamente):

```cpp
#define SPI_FREQUENCY 80000000
```

---

# Rotación

Vertical

```cpp
tft.setRotation(0);
```

Horizontal

```cpp
tft.setRotation(1);
```

Invertida

```cpp
tft.setRotation(2);
```

Horizontal invertida

```cpp
tft.setRotation(3);
```

---

# Código de Prueba

```cpp
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

#define TFT_BL 27

void setup()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();

    tft.setRotation(1);

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);

    tft.drawString("ESP32 OK", 60, 100);
}

void loop()
{
}
```

---

# Problemas Encontrados

## Síntoma

La pantalla mostraba:

- Imagen con ruido.
- Píxeles aleatorios.
- Pantalla parcialmente negra.
- Texto visible únicamente en una parte.

## Causa

Se estaba utilizando el driver incorrecto.

```cpp
#define ST7796_DRIVER
```

## Solución

Cambiar a:

```cpp
#define ILI9488_DRIVER
```

Después del cambio la pantalla funcionó correctamente.

---

# Librerías

Instalar desde el Administrador de Librerías:

- TFT_eSPI

---

# Configuración Pendiente

Aún falta identificar:

- [ ] Controlador Touch (XPT2046 o GT911)
- [ ] Pines del Touch
- [ ] Pines de la MicroSD
- [ ] LED RGB
- [ ] Sensor de luz
- [ ] Buzzer (si existe)
- [ ] PSRAM (si la placa dispone de ella)

---

# Resumen Rápido

| Parámetro | Valor |
|-----------|-------|
| Modelo | ESP32-3248S035 |
| Driver TFT | ILI9488 |
| Resolución | 320x480 |
| MOSI | GPIO13 |
| MISO | GPIO12 |
| SCK | GPIO14 |
| CS | GPIO15 |
| DC | GPIO2 |
| RST | -1 |
| Backlight | GPIO27 |
| SPI | 40 MHz |
| Librería | TFT_eSPI |

---

# Observaciones

- No asumir que todas las ESP32-3248S035 utilizan el mismo controlador TFT.
- Verificar siempre el driver antes de iniciar un proyecto.
- Guardar esta configuración como plantilla para futuros desarrollos.
- Si aparece ruido o artefactos en pantalla, revisar primero el driver seleccionado y la frecuencia SPI antes de modificar el código de la aplicación.

---
**Versión del documento:** 1.0
**Estado:** ✅ Configuración validada para mi ESP32-3248S035 con controlador ILI9488.
