# 01 — Visión del juego

Correcciones de Franco. Esta es la visión de la demo v1.

## Nombres

- Título público: **Spinning Riot**
- Repo / módulo UE: **YowYow** (solo interno)

## Pitch

**Spinning Riot** es un action 3D con sprites 2D (PaperZD). Jugás a **Eri**, dos yoyos, en **`L_StylizedCity`**.

Combate: combo light, area, homing aéreo (dash + bounce), y **Trick Mode** — un modo de ataque con QTE de direcciones. v1: el único trick es **DNA**. Dog Walk y Boing no entran.

Cierre = **esta demo**. No hay más niveles, boss, story ni grapple.

## Fantasía del player

Tercera persona, cámara orbit. Sprite billboard 4 direcciones.

### Ataques normales

1. **Light** — combo de 3. El yoyo vuela y vuelve. Hitbox sigue al yoyo.
2. **Area / heavy** — dos medialunas back→front. Tierra y aire.
3. **Homing** — en el aire, si hay target, el **light** lanza el dash. Al llegar, bounce arriba. Cámara lockeada detrás de Eri. **`IA_Homing` está deprecado**; el hijack del light en aire es el diseño correcto. Marker de crosshair sobre el enemigo seleccionado cuando hay varios.

### Trick Mode (DNA, v1)

No es traversal. Es un **modo de ataque** con QTE.

1. La **trick gauge** se llena **con cada golpe a enemigos**. Solo se puede entrar cuando está **llena**.
2. **Hold** del botón Trick Mode (el que hoy dispara `EnterTrickMode`) con la barra llena → entra a Trick Mode.
3. Al entrar: se cambia a un **IMC distinto**. No te podés mover ni atacar. Solo inputtear lo que aparece en pantalla.
4. En HUD aparecen **4 inputs direccionales**. Hay que apretarlos (en orden) **antes de que la barra se acabe**.
5. Mientras el botón se mantiene, la barra **drena en tick**.
6. Si se ingresa una dirección incorrecta, la barra llega a 0 o se suelta el hold **sin** completar los 4 inputs → falla el QTE: no hay ataque, gauge a 0 y vuelta inmediata al IMC normal.
7. Si se completan los 4 inputs → se dispara **DNA** (dos fases, después IMC normal):
   1. Los yoyos **rodean a Eri** en un senoidal/órbita alrededor de ella. Primera animación.
   2. Segunda animación: salen **6 yoyos** disparados en asterisco (6 direcciones). Eri en idle tiene 2; este ataque es la excepción.
8. Durante **ambas fases de DNA** el input de gameplay sigue bloqueado y ya no se aceptan direcciones del QTE. Soltar el hold después del éxito no cancela DNA. El IMC normal se recupera **solo cuando termina todo el ataque** y los yoyos vuelven al estado de idle.

Dog Walk / Boing: no existen en v1. El QTE no elige entre tricks; el único resultado de éxito es DNA.

### Gauge vs stylish combo

Son dos medidores:

- **Trick gauge** — recurso para entrar a Trick Mode. Fill por hit a enemigo. Drain solo durante Trick Mode.
- **Stylish combo** (D / C / B / A / S / SS / SSS) — rank de pelea. Idle drain, bajar un tier si te pegan. Se muestra in-combat y **en la UI de Game Won**.

## Loop de la demo

```
Title_test
  → OpenLevel L_StylizedCity
  → Tutorial (2 slides de UI)
  → Playing (ciudad + oleadas)
  → Game Won (UI + rank de combo)     si se terminan las oleadas
  → Game Over (UI distinta)           si Eri muere
```

No hay fase Puzzle. Está **deprecada**; hay que sacarla.

A nivel código el flow es un **modo/fase** en el GameMode. Cada modo muestra una UI distinta y nada más (Game Won / Game Over no tienen gameplay). Tutorial: dos slides, al terminar se pasa a Playing. El índice de slides puede vivir en el widget; el GameMode solo necesita saber si todavía estás en Tutorial o ya en Playing.

Mapa de ship: `Content/SylizedCity/Scene/L_StylizedCityMap`. Cook actual (`Title_test` + `Test`) hay que alinearlo.

## Qué no es v1

- Grapple
- Puzzle / puertas DNA / `IDNAInteractable`
- Dog Walk, Boing
- Boss, más niveles, diálogos
- Save de progreso (solo audio)
- Botón dedicado de homing (`IA_Homing`)
