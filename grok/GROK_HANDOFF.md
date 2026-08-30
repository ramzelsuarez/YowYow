# YowYow — Handoff para retomar con Grok

**Proyecto:** `YowYow` (UE 5.7, C++ + PaperZD + SpinningRiot)  
**Repo:** `https://github.com/ramzelsuarez/YowYow.git`  
**Última sesión:** polish de combate (ataque, dual yoyo, impact, health, homing cam)  
**Preferencia del dev:** **no compilar desde el agente** — el usuario compila en el editor.

---

## Qué es el juego (1 línea)

Action 2.5D en UE5: jugador **Eri** (yoyó, homing, trick gauge), enemigos con AI simple + wave tokens, sprites PaperZD.

---

## Plan original (3 checkpoints, 1 PR)

| CP | Objetivo | Código |
|----|----------|--------|
| **1** | Impact + types + health + hit damage | Hecho |
| **2** | AttackComponent limpio, dual yoyo, locks, area | Hecho (+ fixes ida/vuelta/buffer) |
| **3** | Homing camera lock + polish | Hecho |

**Fuera de scope (sigue fuera):** trick mode real, ranged real, rewrite EnemyAI.

---

## Qué quedó implementado en C++

### Combat feel
- `Source/YowYow/Public|Private/Combat/CombatImpactLibrary.*`
- `Source/YowYow/Public|Private/Combat/CombatImpactSubsystem.*` (`UWorldSubsystem` — UE lo monta solo)
- Knockback (`LaunchCharacter`) + hitstop (`CustomTimeDilation` + timer restore por actor)

### Attack data
- `Source/YowYow/Public/Types/AttackTypes.h`
- `EAttackMotion`: `ArcSweep` | `FollowSource` | `OrbitCircle`
- `EYoYoHand`: `Right` | `Left` | `Both`
- `FAttackData`: Damage, HitboxRadius, Motion, arcs, HitStop*, RecoveryTime, YoYoHand, FollowDamageWindow

### Health
- Fixes: daño real, death `<= 0`, heal clamp, `OnHealthDamageTaken` broadcast
- `int32` (no `int8` — Blueprint no lo soporta)
- Delegate param: `DamageInstigator` (no `Instigator` — shadowing de `AActor`)
- `ACharacterBase`: dead → LifeState; player hurt → camera shake (`DamageCameraShake`)

### Hitbox
- `AAttackHitbox`: motions data-driven; `HandleHit` → ApplyDamage + impact lib
- `bDrawDebug` off por default

### AttackComponent
- **Sin** Cast a `AEriCharacter`
- Multi-hitbox; `OnAttackStarted` / `OnAttackFinished` (comentarios de consumer)
- `SetHandSources`, `SetRequiresPresentationComplete`, `NotifyPresentationComplete`
- Buffer de ataque mientras yoyo no volvió
- Facing lock: `bOrientRotationToMovement = false` durante ataque
- Components de gameplay: **solo BP** (CharacterBase no hace `NewObject` de Attack/Health/State)

### Eri yoyos
- C++ crea `YoYoRight` / `YoYoLeft` (`UStaticMeshComponent`) — como la cámara
- Mesh: slot `YoYoMeshAsset` / `YoYoLeftMeshAsset` o Static Mesh en el componente del BP
- **No pisar** rest pose del viewport en BeginPlay (solo se cachea)
- Motion: **ida → vuelta**; hit window cierra → return; buffer del próximo ataque hasta home
- Area: OrbitCircle + ambos yoyos (visual)

### Homing cam (CP3)
- Lock yaw detrás de Eri en dash (dirección a target / velocity)
- Unlock en bounce o cancel (`CancelHomingAttack`)
- `Look` bloquea yaw si locked; pitch opcional `bHomingCameraLockPitch`
- Tunables: `HomingCameraYawInterpSpeed`, `bHomingCameraLockPitch`

---

## Decisiones de diseño importantes

1. **Un AttackComponent** compartido; delivery por **data** (no EriAttack vs EnemyAttack).
2. **No** auto-montar components de combate en C++ (duplicados con BP).
3. **Yoyos:** subobjects C++ + asset mesh del proyecto (pickers de “component instance” en BP listaban clases de engine — no sirvieron).
4. **Hitstop:** subsystem con map actor→timer (no timers en el hitbox, se destruye antes).
5. **Shake de cámara:** solo al **recibir** daño (3 HP = golpe raro).
6. **Homing cam:** solo durante launch→hit; unlock al bounce (sprite falling libre).

---

## Archivos clave tocados / nuevos

```
Source/YowYow/
  Public|Private/Combat/CombatImpactLibrary.*
  Public|Private/Combat/CombatImpactSubsystem.*
  Public/Types/AttackTypes.h
  Public|Private/Attacks/AttackHitbox.*
  Public|Private/ActorComponents/AttackComponent.*
  Public|Private/ActorComponents/HealthComponent.*
  Public|Private/ActorComponents/HomingAttackComponent.*
  Public|Private/Characters/CharacterBase.*
  Public|Private/Characters/EriCharacter.*
```

---

## Trabajo manual en editor (pendiente del usuario)

El agente **no** edita `.uasset` de forma fiable.

### BP Eri
- [ ] Un solo `AttackComponent`, `HealthComponent`, `CharacterStateComponent`, `HomingAttackComponent`
- [ ] `YoYoRight` / `YoYoLeft` visibles; asignar **Static Mesh** (asset) y posición rest en viewport
- [ ] Opcional: `YoYo Mesh Asset` en Class Defaults
- [ ] `DamageCameraShake` + scale
- [ ] Input: Attack + Area + Homing flow (aire + target)

### Attack data (`EriCharacterAttackData` o similar)
| Entry | Motion | YoYoHand | Notas |
|-------|--------|----------|--------|
| Normal[0] | FollowSource | Right | Damage/Range/Speed/Knockback/HitStop |
| Normal[1] | FollowSource | Left | |
| Normal[2] | FollowSource | Both | |
| Area | OrbitCircle | (visual Both) | Damage bajo, **RecoveryTime alto** |
| Enemy normal | ArcSweep | n/a | default |

### Otros
- [ ] Enemigos: Health + AttackComponent en BP
- [ ] PaperZD: pose **espalda** en ActionState Homing
- [ ] Smoke PIE completo (ver abajo)

---

## Cómo probar en PIE (resumen)

### Combate base
- Golpes bajan HP; a 0 → dead / LifeState
- Hitstop + knockback al conectar
- Player recibe daño → shake (si hay class)

### Yoyo
- Combo R → L → Both según data
- Ida y **vuelta** visible; no teleporta al rest al cerrar hitbox
- Spam attack: el siguiente sale **cuando el yoyo vuelve**
- Rest pose = viewport (no salta a otra posición al PIE)
- Area: círculo + recovery anti-spam

### Homing
- Dash: cámara locked (espalda); no orbitar mid-dash
- Bounce: look libre
- Aterrizar / perder target mid-dash: no queda locked

---

## Cómo retomar con Grok (prompt sugerido)

```
Repo YowYow UE5.7. Leé GROK_HANDOFF.md.

Contexto: CP1–CP3 del polish de combate ya están en C++.
Preferencia: no compilar vos; yo compilo en el editor.

Siguiente trabajo posible:
- [ ] bugs de PIE que te describo
- [ ] tuneos / polish
- [ ] trick mode
- [ ] content/data helpers
- [ ] otra feature

No reimplementes health/attack/yoyo/homing cam a menos que esté roto.
```

---

## Bugs / límites conocidos

- `USceneComponent*` + component picker en Class Defaults lista **clases de engine**, no instancias del BP → por eso yoyos son subobjects C++.
- Delegates dinámicos: no usar nombres de param que shadown members de `AActor` (`Instigator`) ni `AttackData` en subclasses de CharacterBase.
- EnemyAI tiene logs ruidosos (no era parte del polish).
- Checklists de sesión vieja pueden estar solo en `~/.grok/sessions/...` de la PC anterior; **este archivo en el repo es la fuente portable**.

---

## Estructura útil del repo

```
YowYow.uproject          # EngineAssociation 5.7, PaperZD
Source/YowYow/           # módulo runtime
Content/                 # BPs, maps (MAP_Playground), PaperAssets
Config/DefaultEngine.ini # GM_SpinningRiot, mapa playground
```

---

## Comandos / flujo del dev

- Compilar: Unreal Editor / Live Coding (no el agente).
- Rama: una sola branch/PR por deadline (acordado en plan).
- Si abrís otra PC: `git pull`, abrir `.uproject`, compilar, leer este `GROK_HANDOFF.md`.
