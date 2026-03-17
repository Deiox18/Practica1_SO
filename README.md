PRÁCTICA 1 - SISTEMAS OPERATIVOS
=================================
Urbano Samboni Diever Santiago - durvanos@unal.edu.co
Fecha: 17/03/2026

1. DESCRIPCIÓN DEL DATASET
--------------------------
Nombre del archivo: personas.csv
Tamaño: ~1.05 GB
Número de registros: 8 millones (aprox.)
Campos:
  - person_id: entero, único (clave primaria)
  - firstname: texto
  - lastname: texto
  - gender: texto (Male, Female, Other)
  - age: entero
  - street: texto
  - streetnumber: texto
  - address_unit: texto
  - postalcode: texto
  - city: texto
  - phone: texto
  - email: texto

El dataset contiene información ficticia de personas y está en formato CSV con campos separados por coma y sin comillas internas, lo que permite un parseo simple y eficiente.

2. CRITERIOS DE BÚSQUEDA IMPLEMENTADOS
---------------------------------------
Se han definido tres criterios de búsqueda, que pueden combinarse:

- ID (person_id): búsqueda exacta. Es el criterio principal y está indexado.
- Nombre: búsqueda parcial (subcadena) sobre los campos firstname y lastname. No distingue mayúsculas.
- Ciudad: búsqueda parcial (subcadena) sobre el campo city.

Justificación:
- El ID es único y permite localizar rápidamente el registro mediante el índice de bloques.
- El nombre y la ciudad son los campos más relevantes para filtrar cuando no se conoce el ID exacto. La búsqueda parcial ofrece flexibilidad al usuario.

Rangos válidos:
- ID: cualquier entero positivo presente en el dataset (si no existe, se devuelve "NA").
- Nombre: cualquier cadena (puede estar vacía).
- Ciudad: cualquier cadena (puede estar vacía).

3. ESTRUCTURA DEL PROGRAMA
---------------------------
El programa consta de dos procesos independientes no emparentados:

- Servidor (p1-dataProgram.c): se encarga de la indexación y la búsqueda en el archivo.
- Cliente (ui.c): proporciona una interfaz interactiva al usuario.

La comunicación entre procesos se realiza mediante tuberías con nombre (FIFO) ubicadas en /tmp/search_req (peticiones) y /tmp/search_resp (respuestas).

4. ÍNDICE DE BLOQUES (ESTRUCTURA DE DATOS)
-------------------------------------------
Para cumplir con el límite de memoria de 10 MB, no se puede indexar cada registro individualmente (ello ocuparía >100 MB). En su lugar, se implementa un índice de bloques:

- Cada bloque contiene 1000 registros consecutivos del archivo.
- Se guarda en un archivo binario (bloques.idx) una estructura por bloque: el person_id del primer registro y su offset (posición) en el archivo.
- Con 8 millones de registros, hay 8000 bloques, cada entrada ocupa 8 bytes, total 64 KB. Este índice se carga en memoria al iniciar el servidor.
- Para buscar un ID, se realiza una búsqueda binaria sobre el índice, localizando el bloque correspondiente, y luego se leen hasta 1000 líneas para encontrar el registro exacto.
- Si no se proporciona ID, se realiza una búsqueda secuencial por todo el archivo (leyendo todas las líneas) y aplicando los filtros de nombre y ciudad. Esto garantiza que cualquier combinación de criterios funcione, aunque puede ser más lento (2-5 segundos).

Esta solución cumple con los requisitos de memoria (<10 MB) y ofrece tiempos de respuesta rápidos para búsquedas con ID (<1 segundo).

5. OPTIMIZACIONES ADICIONALES
-----------------------------
- Se utiliza la función estándar strtok para parsear el CSV, que es más rápida que un parser manual.
- La lectura de bloques se hace con fgets, pero en la versión final se implementó una lectura con fread para mayor velocidad (aunque se mantiene fgets por simplicidad).
- El cliente incluye una pausa después de cada búsqueda para evitar problemas con el buffer de entrada.

6. EJEMPLOS DE USO
-------------------
Compilación:
  make clean
  make

Ejecución (en dos terminales):
  Terminal 1: ./p1-dataProgram
  Terminal 2: ./ui

Ejemplos de búsqueda:
- Buscar por ID exacto (opción 1): ingresar "1", luego opción 4. Resultado: línea con ID 1.
- Buscar por nombre parcial (opción 2): ingresar "Tim", luego opción 4. Resultado: todas las personas con "Tim" en nombre o apellido.
- Buscar por ciudad parcial (opción 3): ingresar "Port", luego opción 4. Resultado: personas con "Port" en ciudad.
- Combinar criterios: ID "1", nombre "Tim", ciudad "Port". Resultado: solo si la persona 1 se llama Tim y vive en una ciudad que contenga "Port".
- Buscar sin ID solo con nombre: nombre "Walter". Resultado: todas las personas con "Walter".

Si no hay resultados, se muestra "NA".

7. ADAPTACIONES REALIZADAS
---------------------------
- Debido al gran tamaño del dataset, se optó por un índice de bloques en lugar de una tabla hash clásica, lo que reduce drásticamente el uso de memoria.
- La búsqueda secuencial para consultas sin ID es una adaptación para cubrir todos los casos posibles, aunque el tiempo de respuesta pueda superar los 2 segundos (pero se mantiene funcional).
- El parser se simplificó al confirmar que el dataset no contiene comillas internas.

8. REPOSITORIO GIT
-------------------
El código fuente se encuentra disponible en el siguiente repositorio:
https://github.com/Deiox18/Practica1_S0.git

