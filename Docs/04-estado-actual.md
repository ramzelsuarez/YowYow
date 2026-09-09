# 04 — Estado actual

Qué está **jugable** hoy vs. lo que el [05](05-plan-de-cierre.md) va a meter.

## Jugable

- Move, jump, cámara orbit, sprites 4 dir
- Light combo yoyo, area, homing (light en aire + bounce)
- Hitstop + knockback
- 3 HP, cura pickup / jihanki
- Trick gauge sube con **pickup** (todavía no con hits)
- Trick Mode: ActionState vacío, IMC no se swappea
- Oleadas + 3 enemigos chase/melee
- Title, pause, HUD vida
- Muerte: freeze AI + **pause** (tiene que pasar a GameOver)
- Encounter complete → fase **Puzzle** (tiene que pasar a GameWon)
- Stylish combo: puntos + tiers + drain + drop al hit. Overlay debug, sin HUD
- Ciudad `L_StylizedCity`

## Gaps que el 05 cubre

| Hoy | Ship |
| --- | --- |
| Gauge por pickup | Fill por hit a enemigo; enter solo si full |
| Trick = ActionState | Hold + IMC QTE 4 dirs + drain; dirección incorrecta, tiempo o release = fallo y recuperar input; éxito = DNA completo antes de recuperar input |
| `IA_Homing` bindeado | Deprecado, no bind. Light-en-aire se queda |
| Sin marker de homing | `HomingTargetMarker` visible solo en el target |
| Fase Puzzle | Tutorial → Combat → GameWon / GameOver |
| Death abre pause | UI de Game Over distinta |
| Tutorial art suelto | 2 slides, después Combat |
| Combo overlay | HUD + rank en Game Won |
| `IDNAInteractable` vacío | Borrar |
| Cook `Test` | Cook `L_StylizedCity` |
| Sin i-frames | Invuln al recibir hit (player) |
| TV = melee (AI nunca llama Ranged; no hay proyectil) | TV se acerca a `RangedAttackRange` y dispara |
| Items (HP / gauge / jihanki) | **No tocar** |

## Debug a des-bindear (properties se quedan)

- `IA_TrickVFXDebug`
- `IA_Homing`
- `bDebugDrawCombo` default false

**No des-bindear el pause del juego** (`IA_Pause`). `IA_PauseAI` (congelar enemigos) se puede dejar.
