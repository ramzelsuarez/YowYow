# Combo system — pickup

Stylish rank tipo DMC. **No compilado** acá (esta máquina tiene UE 5.6, el proyecto es 5.7). Abrir en la PC con 5.7, compilar, PIE.

## Qué hace

- Cada golpe a un **IComboable** suma 10 pts y el hit count.
- Umbrales suben el rank: `None → D → C → B → A → S → SS → SSS`.
- Si no pegás 1.5s, drena 35 pts/s y el rank baja de a uno. A 0 se corta.
- Eri recibe daño (`OnHealthChanged` con delta < 0) → baja **1 tier**. En D se corta.
- Killing blow cuenta (se chequea `CanGrantCombo` **antes** de `ApplyDamage`).
- Jihanki / props no implementan `IComboable` → no suman.
- El combo de ataques Normal 1-2-3 (`UAttackComponent`) no se tocó.

## Archivos nuevos

- `Source/YowYow/Public/Interfaces/Comboable.h` (+ `.cpp`)
- `Source/YowYow/Public/Types/ComboTypes.h`
- `Source/YowYow/Public/ActorComponents/ComboComponent.h` (+ `.cpp`)

## Archivos tocados

- `EnemyCharacter` — implementa `IComboable` (`CanGrantCombo` = no muerto)
- `EriCharacter` — `CreateDefaultSubobject<UComboComponent>`
- `AttackHitbox::HandleHit` — snapshot comboable → damage → `NotifyHit`
- `HomingAttackComponent::ApplyHomingHitDamage` — igual

## Tunables (Eri → ComboComponent)

| Prop | Default |
|------|---------|
| `HitPoints` | 10 |
| `DecayDelay` | 1.5 |
| `DecayPerSecond` | 35 |
| `Tiers[].MinPoints` | D1 / C40 / B90 / A150 / S230 / SS320 / SSS420 |
| `bDebugDrawCombo` | **true** (overlay amarillo en PIE) |

## Lo que falta (editor)

1. Compilar con UE 5.7.
2. PIE: pegar enemigo → overlay `COMBO D  1 HITS  10 PTS`. Dejar de pegar → drain. Recibir hit → baja un rank.
3. HUD de verdad: en `WBP_HUD` bind al `ComboComponent` de Eri:
   - `OnComboChanged(Points, HitCount, Tier)` — texto rank / hits / barra
   - `GetProgressToNextTier()` — 0–1 al siguiente umbral
   - `GetTierDisplayName()` / `GetTierColor()`
   - `OnComboBroken` — hide
4. Cuando el HUD esté, desactivar `bDebugDrawCombo`.

## Test rápido

- Normal / Area / Homing suman (mismo `HitPoints`).
- Enemigo muerto / no-comboable no suma.
- String Normal 1-2-3 sigue igual.
