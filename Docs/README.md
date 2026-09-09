# Spinning Riot — documentación

Nombre público: **Spinning Riot**. Repo / módulo: **YowYow** (solo interno).

Esta carpeta describe el juego según código, assets y correcciones de Franco. No es un playthrough.

**Visión:** [`01-vision-del-juego.md`](01-vision-del-juego.md). **Plan de cierre (un prompt):** [`05-plan-de-cierre.md`](05-plan-de-cierre.md).

## Cómo está organizada

| Archivo | Qué hay |
| --- | --- |
| [01-vision-del-juego.md](01-vision-del-juego.md) | De qué va el juego, fantasía, loop, tono. Lo más importante para corregir. |
| [02-sistemas.md](02-sistemas.md) | Cómo está armado por sistemas (player, combate, enemigos, items, UI). |
| [03-flujo-y-contenido.md](03-flujo-y-contenido.md) | Maps, menús, personajes, assets que existen. |
| [04-estado-actual.md](04-estado-actual.md) | Qué está jugable vs. qué es stub / debug / asset suelto. |
| [05-plan-de-cierre.md](05-plan-de-cierre.md) | Plan de cierre ejecutable (un prompt). |
| [06-para-fran.md](06-para-fran.md) | Lo escribe el agente **al terminar** el 05: guía editor paso a paso. Todavía no existe. |

## Convenciones

En los docs marco tres tipos de afirmación:

- **Hecho** — está en código, config o asset con nombre claro.
- **Inferencia** — lo deduzco de comentarios, SFX, widgets o ramas. Puede estar mal.
- **Pregunta** — no lo sé y necesito que lo confirmes.

Hechos de producto:

- Título: **Spinning Riot** (YowYow = interno)
- Personaje: **Eri**
- Engine: **Unreal 5.7**, PaperZD + Enhanced Input + Niagara + UMG
- Cierre: **esta demo**, mapa de ship **`L_StylizedCity`**
- Tricks: DNA / Dog Walk / Boing — **v1 solo DNA**
- Homing: light en el aire, dash + bounce. `IA_Homing` deprecado. Crosshair en el target. No grapple
- Trick Mode: hold con gauge llena → QTE 4 direcciones → DNA o nada
- Flow: Tutorial (2 slides) → Combat → GameWon / GameOver. Puzzle afuera
- Combo stylish: in-combat + rank en Game Won
