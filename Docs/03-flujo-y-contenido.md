# 03 — Flujo y contenido

## Arranque

| Setting | Valor |
| --- | --- |
| Engine | UE 5.7 |
| GameInstance | `GI_Main` |
| GameMode default | `GM_SpinningRiot` |
| GameDefaultMap | `/Game/Maps/Title_test` |
| EditorStartupMap | `/Game/Maps/MAP_Playground` |
| Maps cooked (ini, desactualizado) | `Title_test`, `Test` |
| Mapa de ship (confirmado) | `SylizedCity/Scene/L_StylizedCityMap` (`L_StylizedCity`) |

Flujo de ship:

```
Title_test → L_StylizedCity
  Tutorial (2 slides UI) → Combat
  → GameWon (UI + combo rank) | GameOver (UI distinta)
```

Puzzle no existe más. El cook tiene que ser `Title_test` + `L_StylizedCity` (hoy todavía lista `Test`). El title preloadea Eri.

## Maps que existen

| Mapa | Rol |
| --- | --- |
| `Maps/Title_test` | Main menu. Se cookea. |
| `SylizedCity/Scene/L_StylizedCityMap` | **Nivel de ship.** Pack de ciudad + wave manager puesto en un commit. |
| `Maps/Test` | Se cookea hoy, ya no es el mapa de ship. |
| `Maps/MAP_Playground` | Sandbox de editor / combat. |
| `ThirdPerson/Lvl_ThirdPerson` | template |
| `Free_Magic`, `SplineEffect2`, Niagara demo | packs de VFX, no gameplay |

## Personajes y animación

### Eri

Sprites 4 direcciones: Idle, Run, Fall/Jump, Attack (right / left / both), Area (front/back), Death, Homing (charge/launch). PaperZD `AS_Eri` + `ABP_Eri`.

Hay un set `PaperAssets/TEST` de sprites viejos. **Inferencia:** no deberían usarse en ship.

### Enemigos

| Enemigo | Anims que vi | Notas |
| --- | --- | --- |
| Long Neck Salary Woman | Idle, Walk, Attack, Damage, Die | SFX + Niagara de hit propios |
| Tengu | Move, Attack, Damage, Death | |
| TV | Movement (idle+move?), Attack, Death | Data de ranged existe. ¿Dispara de verdad? |

Todos heredan AI chase+melee. No hay boss.

## Ciudad / props

Contenido propio en `Content/Assets/Buildings`: edificios, casa, cruce, montaña, árboles, **vending machine** (blue/red drink).

Pack `SylizedCity`: bloques, techos, veredas, carteles (karaoke, noodles, sushi, tech), autos, bici, policía, van, tranvía, faroles, grafitis, plantas.

Level prototyping: door, jump pad, wobble target.

SFX de mundo que sugieren traversal: `door_open`, `ElevatorStart`, `ElevatorEnd`, `EnteringCombat`.

La demo se juega **entera en `L_StylizedCity`**. No hay tramo puzzle.

## Audio (lo que hay)

BGM: GameStartMenu, Exploration, Battle_Room, AmbientWomenSounds.

Player: 3 yoyo attacks + voices, area, jump kick, jump yoyo throw, hurt, death, jump, land, walk, 3 tricks.

Enemigos: alert / walk / attack / hurt / die por tipo.

UI: menu open/close/click/back.

Mundo: vending hit, heal pickup, item get, game clear, entering combat, door, elevator.

## UI screens

- Title: start / options
- Tutorial: 2 slides (`tutorial_1` / `tutorial_2`). Índice en el widget; GameMode solo `FinishTutorial`
- In-game HUD: LIFE + trick gauge + QTE de 4 flechas + stylish combo
- Pause (solo en Combat)
- Game Won (`WBP_GameClear` + rank de combo)
- Game Over: widget **distinto** a win y a pause

Retry / Title en esas UIs: Franco en el widget (OpenLevel / load title).

## Ramas / gente (contexto, no docs de diseño)

Por git: Fran (sistemas C++), Zel (enemigos, VFX, SFX, waves, city), Ana (level design), metha (rama mergeada). Repo `ramzelsuarez/YowYow`.

Combo stylish ya está en este working tree (lógica + overlay debug). HUD y rank en Game Won faltan.
