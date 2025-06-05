# Formatos de archivo utilizados por Triangle

Triangle es un programa desarrollado por Jonathan Richard Shewchuk para generar triangulaciones de Delaunay, mallas de Voronoi y refinamientos de malla. A continuación, se explican los formatos de archivos de entrada y salida más comunes que utiliza.

---

## `.poly` — Definición de dominio poligonal

Este archivo se usa para describir dominios con huecos, contornos y segmentos definidos manualmente.

**Estructura**:

1. **Primera línea**: `<# de puntos> <dimensión> <# de atributos> <marcadores de frontera>`
2. **Lista de puntos**:

   ```
   <índice> <x> <y> [atributos] [marcador]
   ```

3. **Segmentos**:

   - Línea: `<# de segmentos> <marcadores>`
   - Luego: `<índice> <punto1> <punto2> [marcador]`

4. **Hoyos (opcional)**:

   - Línea: `<# de hoyos>`
   - Luego: `<índice> <x> <y>` (un punto dentro del hoyo)

5. **Regiones (opcional)**:

   - Línea: `<# de regiones>`
   - Luego: `<índice> <x> <y> <número de región> <atributo máximo>`

---

## `.node` — Lista de nodos (puntos)

Archivo básico de entrada que define los puntos usados en la triangulación.

**Estructura**:

1. **Primera línea**: `<# de nodos> <dimensión> <# de atributos> <marcadores de frontera>`
2. **Cada nodo**:

   ```
   <índice> <x> <y> [atributos] [marcador]
   ```

---

## `.ele` — Elementos (triángulos)

Archivo de salida que contiene la información de los triángulos generados.

**Estructura**:

1. **Primera línea**: `<# de triángulos> <nodos por triángulo> <# de atributos>`
2. **Cada triángulo**:

   ```
   <índice> <nodo1> <nodo2> <nodo3> [atributos]
   ```

---

## `.neigh` — Vecinos de triángulos

Archivo de salida que muestra la conectividad entre triángulos (vecindad).

**Estructura**:

1. **Primera línea**: `<# de triángulos> <número de vecinos por triángulo>`
2. **Cada línea**:

   ```
   <índice> <vecino1> <vecino2> <vecino3>
   ```

   - Un valor `-1` indica que no hay vecino en ese lado (frontera).

---

## Recursos

- [Documentación oficial de Triangle](https://www.cs.cmu.edu/~quake/triangle.html)
- [Ejemplos de archivos](https://www.cs.cmu.edu/~quake/triangle.poly.html)

---

Este resumen proporciona la estructura básica de los archivos que Triangle puede leer o generar. Son útiles para definir dominios complejos, controlar mallas y analizar la topología resultante.
