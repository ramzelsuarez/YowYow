# 02 — Sistemas

Cómo está armado el juego por bloques. Casi toda la lógica vive en C++ (`Source/YowYow`); Blueprints son wrappers (BP_Eri, enemigos, UI, GameModes).

## Personaje y estados

`ACharacterBase` (PaperZDCharacter) es la base de Eri y enemigos.

Componentes que crea en C++:

- `UAttackComponent`
- `UHealthComponent` (default 3 HP, enteros tipo “corazones”)
- `UCharacterStateComponent`

Estados groseros (`CharacterStates.h`):

| Eje | Valores | Uso |
| --- | --- | --- |
| Action | Default / Attacking / Trick / Homing | Qué “modo” estás haciendo |
| Locomotion | Grounded / Airborne | Homing solo en aire |
| Life | Alive / Dead | Bloquea input, spawnea pause en player |
| Attack | None / Attacking / Recovery | `CanMove()` es false si no es None |

Comentario en código: estos estados son groseros a propósito. El timing fino del hit (start / active / combo / recovery) vive en `AttackComponent`.

## Sprites y cámara

- `ASpinningRiotCameraManager` brodcasta la rotación de cámara.
- `USpriteDirectionComponent` cuantiza a 4 direcciones relativas a cámara (X = L/R, Y = front/back). El AnimBP de PaperZD lee eso.
- El flipbook se rota para siempre mirar a cámara (`Yaw + 90`).
- **Hecho:** hay un comentario de que Ana confirmó enemigos de 1 sola dirección (siempre de frente a cámara). El component igual está en la base, no solo en Eri.

Homing lockea yaw detrás de Eri (interp 540 deg/s). Pitch se puede dejar libre.

## Eri (player)

`AEriCharacter` agrega:

- SpringArm + Camera
- Dos `UStaticMeshComponent` de yoyo (`YoYoRight` / `YoYoLeft`) con sockets `Hand_R` / `Hand_L`
- Niagara en cada yoyo (ataque normal vs area)
- Aura de Trick (hoy debug)
- `UHomingAttackComponent`
- `UTrickGaugeComponent`
- `UComboComponent` (stylish rank)

Presentación del yoyo (`EYoYoPresentationMode`):

- **Thrust** — sale y vuelve (light)
- **Orbit** — medialunas (area)
- **Homing** — charge hacia el target, después return

Mientras el yoyo está afuera o volviendo, el próximo ataque se **bufferea**. Eri setea `SetRequiresPresentationComplete(true)`; los enemigos no.

Input (Enhanced Input, `IMC_SpinningRiot`):

| Acción | Qué hace |
| --- | --- |
| Move / Look / Jump | estándar |
| Attack | light, o homing si airborne + target found |
| AreaAttack | heavy, tierra y aire |
| Homing | opcional, dedicado |
| TrickMode | hold → ActionState = Trick (no más) |
| TrickInput | stick/vector, se descarta |
| TrickVFXDebug | toggle del aura |
| Pause | menú |
| PauseAI | freeze global de AI (debug) |

## Combate

Data: `UCharacterAttackData` (assets `EriCharacterAttackData`, `EnemyAttackData`, `RangedEnemyAttackData`).

Tipos: `Normal` (array = combo), `Area` (un solo shot), `Ranged` (spawnea un actor proyectil).

Motions del hitbox (`AAttackHitbox`, no es Blueprintable):

- **ArcSweep** — semicírculo adelante. Enemigos.
- **FollowSource** — el volumen sigue un scene component (el yoyo). Damage outbound-only o full path.
- **OrbitCircle** — dos medialunas back→front.

Feel compartido (`UCombatImpactLibrary` + `UCombatImpactSubsystem`):

- Knockback horizontal (`LaunchCharacter`)
- Hitstop local (time dilation por actor, sobrevive si el hitbox se destruye)

Ranged: `ExecuteRangedAttack` spawnea `FRangedAttackData.Projectile`. No hay clase C++ de proyectil. Hay data `RangedEnemyAttackData` — **inferencia:** el TV dispara.

## Homing

Estados: Idle → Searching → TargetFound → Charging → Launching → Hit → Recovery.

- Search solo airborne, sphere overlap, actors que implementan `IHomingable` y `CanBeHomed`.
- Charge: gravedad 0, yoyos vuelan al target, después `BeginLaunch`.
- Hit: damage + **bounce up** + cooldown 2s.
- Input de ship: **light en el aire** si hay target (`TryAttack` → homing). `HomingAction` / `IA_Homing` deprecado, no bindear.
- Marker: `HomingTargetMarker` en el enemigo, visible solo si es el target seleccionado.

## Combo (stylish rank)

Notas de pickup: `COMBO_PICKUP.md`.

`UComboComponent` vive en Eri. Hits a actors `IComboable` (enemigos vivos; jihanki no) suman puntos.

- Tiers: `None → D → C → B → A → S → SS → SSS` (umbrales 1 / 40 / 90 / 150 / 230 / 320 / 420)
- +10 pts por hit (Normal, Area y Homing; el killing blow cuenta)
- 1.5s sin pegar → drena 35 pts/s
- Eri recibe daño → baja 1 tier (en D se corta)
- Overlay debug `bDebugDrawCombo = true`. HUD real no está (`OnComboChanged` / `OnComboTierChanged` / `OnComboBroken` listos para bind)

El combo de ataques Normal 1-2-3 (`UAttackComponent`) es otro sistema; no se tocó.

## Trick gauge y Trick Mode

Modo de ataque con QTE. v1 = solo DNA.

- Gauge 0–100. **Fill por hit a enemigo** (`FillFromHit`). Pickups de gauge pueden seguir sumando.
- Enter: hold Trick Mode **solo si IsFull**. IMC_TrickMode: sin move/attack, solo direcciones.
- 4 cardinales en HUD. Drain en tick solo mientras el QTE está activo. Dirección incorrecta, release o gauge 0 sin completar → fallo, gauge a 0, sin ataque y recuperación inmediata del input normal. 4/4 → `DoAttack(DNA)`; mantener el input de gameplay bloqueado hasta terminar ambas fases y restaurar los yoyos de idle. Durante DNA no se aceptan inputs QTE ni se cancela por soltar el hold.
- DNA: dos fases. (1) órbita/senoidal alrededor de Eri + anim 1. (2) anim 2 + 6 yoyos en asterisco.
- `IDNAInteractable` se borra.

SFX que existen: `Trick１DNA` (v1), `Trick2DogWalk` / `Trick3boing` (no v1), `yoyotrick_voice`.

## Enemigos y encounters

`AEnemyCharacter` : CharacterBase + Homingable + Comboable. Al morir: registra derrota, chance de dropear HP (25% default), cadáver 1.2s. Marker de homing nativo (plan 05).

Tres BPs:

- `BP_LongNeck` / `BP_LongNeckSalaryWoman` — mujer de cuello largo (office lady)
- `BP_Tengu`
- `BP_TV`

AI (`UEnemyAIComponent`): hoy **todos** son melee. Chase si < 700, `DoAttack(Normal)` si < 180. No hay rango de disparo, no hay “acercarse a distancia de shot”. `ExecuteRangedAttack` existe (spawnea `FRangedAttackData.Projectile`) pero **nadie lo llama** y no hay clase de proyectil. El TV tiene que ser ranged-only: acercarse hasta un threshold y disparar. LongNeck/Tengu siguen melee.

`AWaveEnemyManager`:

- Auto-start con delay
- Waves configurables (default 2, 3, 4)
- Spawn points (`AEnemySpawnPoint`, “path mouth”)
- Al completar → `OnEncounterCompleted` → GameMode pasa a **GameWon** (Puzzle deprecado)

No hay diferenciación de comportamiento por tipo de enemigo en C++ (todos chase + Normal attack). El TV tendría que usar Ranged desde data/BP.

## Items y mundo

Solo dos pickups pensados (`ItemBase` comment):

- `AHealthItem` — cura 1 si no estás full. Pickup on overlap.
- `ATrickGaugeItem` — +25 gauge.

`AJihanki` (máquina expendedora): al recibir damage dropea HP con cooldown 1s.

`IPickupable` existe pero `SetOverlappingItem` está vacío. Los items concretos curan/suman directo en overlap, no pasan por el player.

`AItemBase` tiene helpers de seno para bob que el Tick no usa. **v1: no tocar items.** El pickup ya funciona por overlap.

Prototipos de nivel (no cableados a tricks en C++): `BP_DoorFrame`, `BP_JumpPad`, `BP_WobbleTarget`.

## UI y persistencia

- HUD: `WBP_HUD` / `WBP_HUD_DEMO`. `AMyHUD` C++ está vacío. Health events se bindean desde BP (`bind health change events to hud`).
- Pause / death: `WBP_Pause`, `WBP_Option_Pause`.
- Title: `WBP_MainMenu`, `WBP_Option_Title`, `GM_MainMenu` (preloadea `BP_EriCharacter` para que el OpenLevel no hitch-ee Niagara/meshes).
- Clear: `WBP_GameClear`.
- Save: `GI_Main` + `Save_Audio`. **Inferencia:** solo volumen BGM/SFX.

Art de HUD: LIFE, trickgauge, tutorial, controls, start/end, GameClear.

## Plugins / módulos que están pero no se usan en gameplay C++

- `GameplayStateTree` enabled en el uproject — no hay StateTrees en Source.
- `AIModule`, `NavigationSystem`, `GeometryCollectionEngine` en el Build.cs — AI actual es chase en línea recta, no NavMesh.
- `ModelingToolsEditorMode` — editor only.
