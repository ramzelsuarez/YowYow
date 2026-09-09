# 05 — Plan de cierre (v1)

Plan para **un solo prompt de agente**. Cierre = demo en `L_StylizedCity`. Todo el código se hace en el working tree actual. Visión: [`01-vision-del-juego.md`](01-vision-del-juego.md).

El agente **no abre UE**. Al **terminar todo el código**, escribe [`06-para-fran.md`](06-para-fran.md): guía paso a paso, exhaustiva, de lo que Franco tiene que hacer en el editor para que el juego compile y se pueda jugar. Ese md es deliverable obligatorio, no un afterthought.

---

## Reglas de implementación (el agente las sigue todas)

1. **No git.** Ni commit, ni push, ni ramas, ni PRs, ni mencionar github.
2. **No compilar. No PIE. No UnrealEditor. No UBT.** El código lo escribe el agente; Franco compila y cablea en el editor después.
3. **No romper el layout de clases nativas.**
   - No renombrar `UPROPERTY` / `UFUNCTION` existentes.
   - No borrar `UPROPERTY` que el BP ya asigna (deprecar: dejar la property, dejar de usarla).
   - **No shadear miembros.** Nombres nuevos, únicos en la jerarquía (`ACharacterBase` / `APaperZDCharacter` / `ACharacter` ya tienen `Sprite`, `Mesh`, `CharacterMovement`, etc.).
   - Nuevos `UPROPERTY` **al final** del `protected`/`private` de la clase, nunca intercalados arriba de properties que el BP serializa.
   - Nuevos valores de `UENUM` **al final** del enum, para no recorrelear valores serializados.
   - No cambiar firmas de `UFUNCTION` existentes; agregar overload / función nueva.
4. **No crear ni editar `.uasset` / `.umap`.** Solo `.h` / `.cpp` / `.ini` / `Docs/*.md`.
5. **No tocar items.** `AItemBase`, `AHealthItem`, `ATrickGaugeItem`, `AJihanki` se quedan como están. No bob, no billboard, no rewrite.
6. **No des-bindear el menú de pausa** (`PauseAction` / `IA_Pause`). El pause in-Combat se queda.
7. **No meter `IDNAInteractable` nuevo.** Se borra (ver paso 8).
8. Comentarios cortos, en inglés como el resto del Source. Nada de narrar el plan en el código.
9. **Verificación estática obligatoria con `rg` antes de terminar.** `rg` significa *ripgrep*: es una herramienta de búsqueda rápida que inspecciona texto sin modificar archivos. Como el agente no compila ni abre Unreal Editor, debe usarlo para revisar referencias, includes y restos de funcionalidad deprecada.
   - Listar los archivos relevantes antes y después de implementar:
     ```powershell
     rg --files Source Config Docs
     ```
   - Confirmar que no quedan referencias a `IDNAInteractable`:
     ```powershell
     rg "IDNAInteractable" Source Config Content Docs
     ```
     Esta búsqueda debe devolver cero resultados, salvo que se esté documentando explícitamente su eliminación.
   - Confirmar que `Puzzle` solo queda como valor de enum/comentario deprecado y no se sigue asignando:
     ```powershell
     rg "SetDemoPhase\s*\(\s*EDemoPhase::Puzzle|EDemoPhase::Puzzle" Source
     ```
     Revisar manualmente cada resultado; no debe existir ningún `SetDemoPhase(EDemoPhase::Puzzle)`.
   - Confirmar que no se siguen bindeando las acciones deprecadas, pero que `IA_Pause` permanece:
     ```powershell
     rg "HomingAction|TrickAuraDebugAction|PauseAction|IA_Homing|IA_TrickVFXDebug|IA_Pause" Source Config Docs
     ```
   - Confirmar que el cook contiene el mapa correcto y no el mapa viejo:
     ```powershell
     rg "MapsToCook|Title_test|L_StylizedCity|/Game/Maps/Test" Config/DefaultGame.ini
     ```
   - Buscar símbolos nuevos para comprobar que tienen declaraciones, definiciones y referencias coherentes:
     ```powershell
     rg "ETrickDirection|AEnemyProjectile|HomingTargetMarker|GameWon|GameOver|FinishTutorial|DNAPhase" Source Docs
     ```
   - Después de cada búsqueda, corregir includes, nombres, referencias obsoletas o documentación desactualizada que aparezcan. `rg` es una verificación auxiliar: no reemplaza la compilación ni la prueba en PIE.

Si un paso pide un widget o un mesh, el C++ expone `TSubclassOf` / `TObjectPtr` y se queda en null. Franco lo asigna.

---

## Spec cerrado (no reinventar)

### Trick Mode

- Entrar **solo** con gauge **llena** (`Current >= Max`).
- Fill: **cada hit a `IComboable`** (mismo momento que el stylish combo, snapshot antes del damage). Monto tunable, default `20` (5 hits → 100).
- Input: hold de `TrickModeAction` (Started = intentar entrar, Completed = soltar).
- Al entrar: `PlayerController::EnterTrickMode()` (ya swappea `IMC_TrickMode`), ActionState = Trick, aura Niagara ON, generar **secuencia de 4 cardinales**, broadcast al HUD.
- Tick: drain gauge (`DrainPerSecond`, default `40` → ~2.5s de ventana).
- IMC de trick: **no movimiento, no ataque, no look de combate.** Solo el QTE (direcciones).
- QTE: 4 direcciones. El input actual es `TrickInputAction` (Vector2D). Cuantizar a cardinal; si matchea el siguiente, avanzar índice; una dirección incorrecta falla inmediatamente. El vector neutro / dentro de deadzone no cuenta como error.
- Éxito (índice == 4): cerrar el QTE, gastar gauge a 0 y ejecutar `DoAttack(EAttackType::DNA)`. Mantener bloqueado el input de gameplay durante **las dos fases completas**. No llamar `ExitTrickMode()` ni restaurar el IMC gameplay al acertar el cuarto input.
- Durante DNA: no drenar gauge ni procesar direcciones QTE; soltar el hold no cancela el ataque. Al terminar fase 2 y restaurar los yoyos de idle: `ExitTrickMode()`, aura OFF, IMC gameplay.
- Fail (dirección incorrecta, gauge 0 **o** se suelta el hold antes de completar): no atacar, gauge a 0, `ExitTrickMode()`, aura OFF, IMC gameplay inmediato.
- No se puede re-entrar hasta volver a llenar.

DNA es un ataque de **dos fases**. No es Area. No es un seno hacia adelante.

**Fase 1 — rodea a Eri**

- Se ejecuta la **primera animación** (PaperZD sequence, Franco la asigna).
- Los **2 yoyos de siempre** orbitan **alrededor de Eri** (senoidal / círculo centrado en ella, no las medialunas de Area).
- Hitbox sigue esos yoyos: daño a quien esté cerca mientras giran.

**Fase 2 — asterisco**

- Se ejecuta la **segunda animación**.
- Salen **6 yoyos disparados** desde Eri, 60° entre sí, plano XY (vista asterisco `*`).
- Cada uno lleva hitbox. Al terminar el vuelo se destruyen o vuelven y se apagan; Eri otra vez con 2 yoyos en las manos.

Eri en idle tiene 2 yoyos. Este ataque es la excepción. El agente elige la implementación más limpia y la documenta en `06-para-fran.md`. Opciones válidas:

- A) 2 meshes de siempre + **spawn 6 actores/meshes transitorios** en fase 2 (recomendado: el rest pose no cambia).
- B) 4 meshes extra nativos en `AEriCharacter` (`YoYoBurst0..3` o similar, nombres nuevos, hidden en idle) + los 2 de siempre = 6 en el burst. Franco los ve en el BP y les pone mesh/Niagara.

No shadear `YoYoRight` / `YoYoLeft`. Presentation mode nuevo (`DNASurround`, `DNABurst`) al final del enum interno de Eri.

Tipos: `EAttackType::DNA` al final. No hace falta `EAttackMotion::DNAWave` genérico si las fases viven en Eri + hitboxes spawn; si el hitbox necesita motion, agregar al **final** algo tipo `OrbitOwner` (fase 1, círculo alrededor del source actor) y `RadialBurst` (fase 2, vuelo recto en una dirección). `FAttackData DNA` al final de `CharacterAttackData`, más campos nuevos al final de esa struct o en la data class: `DNAPhase1Duration`, `DNABurstCount` (default 6), `DNABurstRange`, `DNABurstSpeed`. Two `TObjectPtr<UPaperZDAnimSequence>` en Eri: `DNAPhaseOneSequence`, `DNAPhaseTwoSequence` (null ok, Franco asigna).

### Homing

- Light en aire + target found = homing. **Dejar `TryAttack` como está.**
- `HomingAction` / `IA_Homing`: **deprecado**. Dejar el `UPROPERTY` para no romper BP. **No bindear.**
- Crosshair: `USceneComponent` nativo en `AEnemyCharacter`, visible **solo** cuando `SetHomingTargeted(true)`. Offset arriba del sprite. Visual (widget/mesh/sprite) lo pone Franco en el BP; C++ crea el root y un `UWidgetComponent` opcional (`HomingMarkerWidgetClass`, puede ser null → el root igual se muestra/oculta).

### Flow (`ASpinningRiot`)

Reemplazar Puzzle. Enum `EDemoPhase` (agregar al final, **no reordenar Combat**):

```
Combat,          // playing — ya existe, valor 0
Puzzle,          // DEPRECADO. Dejar el valor para no recorrelear. Nunca setearlo. HandleEncounterCompleted ya no lo usa.
Tutorial,        // nuevo
GameWon,         // nuevo
GameOver,        // nuevo
```

Default de BeginPlay: `Tutorial`. Si no hay `TutorialWidgetClass`, skip a `Combat`.

| Fase | UI | Gameplay |
| --- | --- | --- |
| Tutorial | widget 2 slides | input bloqueado salvo UI (avanzar slide) |
| Combat | HUD | juego |
| GameWon | you-win + rank de combo | nada. Freeze AI. Snapshot del tier |
| GameOver | you-lose (distinta a win y a pause) | nada. Freeze AI. Reemplaza el pause-on-death |

Slides del tutorial: **las trackea el widget**, no el GameMode. El widget llama `FinishTutorial()` (C++) al salir de la slide 2. GameMode solo sabe Tutorial vs Combat.

Muerte de Eri: **no** `OpenPauseMenuOnDeath`. GameMode → `GameOver`. Pause sigue existiendo solo in-Combat.

GameWon: al `OnEncounterCompleted`. Widget recibe rank (`EComboTier`) + hit count.

### Sacar (sí, hacerlo)

- Fase Puzzle: no setearla más. Comment deprecado en el enum.
- `IDNAInteractable` .h/.cpp — borrar. Nadie más lo incluye salvo él mismo.
- Bind de `HomingAction`.
- Bind de `TrickAuraDebugAction` (dejar UPROPERTY).
- Comment de grapple en `IHomingable`.
- `bDebugDrawCombo` default **false**.

No tocar: `PauseAction` (menú pausa), items, `IA_PauseAI` (debug freeze de AI, se puede dejar).

### Stylish combo en Game Won

Sí. Snapshot al entrar a GameWon: points, hits, tier. El widget lo lee de funciones del GameMode.

---

## Orden de implementación (un prompt, en este orden)

Cada paso tiene que dejar headers compilables por sí mismos (includes, generated.h, UCLASS). No dejar un .h a medias para “seguir en el siguiente”.

### Paso 1 — Tipos DNA + Trick QTE

**Nuevo** `Source/YowYow/Public/Types/TrickTypes.h` (enum only, UENUM BlueprintType):

- `ETrickDirection` : None, Up, Down, Left, Right

**`AttackTypes.h`** — al **final** de `EAttackType`: `DNA`. Si el hitbox necesita motions nuevas, al **final** de `EAttackMotion`: `OrbitOwner`, `RadialBurst`. No reordenar los valores viejos.

**`CharacterAttackData.h`** — al final: `FAttackData DNA` + tunables de las dos fases (`DNAPhase1Duration`, `DNABurstCount = 6`, `DNABurstRange`, `DNABurstSpeed`). Ctor: no pisar defaults de Area.

### Paso 2 — Trick gauge: fill por hit + drain

**`TrickGaugeComponent`**

Agregar (nombres nuevos, no shadow):

- `bool IsFull() const` — `CurrentGauge >= MaxGauge`
- `bool IsEmpty() const`
- `float GetFillPercent() const`
- `void FillFromHit()` — suma `GaugePerHit` (nuevo `UPROPERTY`, default 20)
- `void Drain(float DeltaTime)` — resta `DrainPerSecond * DeltaTime` (nuevo, default 40), clampa 0
- `void EmptyGauge()` — a 0 + broadcast
- Tick **off** por default. El caller drena en su tick; no hace falta prender tick del component si Eri ya tickea.

No renombrar `CurrentGauge` / `MaxGauge` / `AddTrickGauge` / `SpendTrickGauge`.

**`ComboComponent::NotifyHit`** — después de `RegisterHit`, si el instigator tiene `UTrickGaugeComponent` y **no** está en Trick, `FillFromHit()`. No fill durante Trick Mode (ActionState == Trick).

### Paso 3 — DNA dos fases

**`AttackComponent`**

- `TryAttack(DNA)` como Area: no usa el index de combo light. Mantiene `bPresentationBlocking` hasta que Eri avisa que **las dos fases** terminaron (`NotifyPresentationComplete`).
- `GetAttackData` case DNA → `&AttackData->DNA`.

**`AttackHitbox`**

- Fase 1: hitbox(es) que orbitan al owner (`OrbitOwner`: ángulo alrededor de `SourceActor`, radio tunable).
- Fase 2: 6 hitboxes `RadialBurst` — cada una vuela en una dirección (0, 60, 120, 180, 240, 300)° en XY, distancia `DNABurstRange`, speed `DNABurstSpeed`.
- No tocar ArcSweep / FollowSource / OrbitCircle.

**`AEriCharacter`**

- `HandleAttackStarted` si DNA: arrancar fase 1 (anim 1 + 2 yoyos orbitando a Eri). No reusar el path de Area (las medialunas son otra cosa).
- Al terminar fase 1: anim 2 + disparar 6 yoyos, sin recuperar el input. Al terminar fase 2: yoyos de idle de vuelta a sockets, `NotifyPresentationComplete` y `ExitTrickModeInternal()` para devolver el input normal.
- Properties nuevas al final: `DNAPhaseOneSequence`, `DNAPhaseTwoSequence`.
- Extra yoyos: ver spec. Nombres nuevos. Documentar la elección en `06-para-fran.md`.

### Paso 4 — QTE + enter/exit Trick Mode

Hoy `AEriCharacter::EnterTrickMode` solo setea ActionState. Hay que:

Started `TrickModeAction`:

- muerto / ya en Trick / gauge no full → return
- `CharacterStateComponent` → Trick
- `Cast<ASpinningRiotPlayerController>(GetController())->EnterTrickMode()`
- aura ON (`TrickAuraVFX->Activate`, no el debug toggle)
- generar 4 direcciones random cardinales, guardar índice 0
- multicast `OnTrickQTEStarted(TArray<ETrickDirection>)` para el HUD

Tick, solo si `bTrickQTEActive` (no durante DNA):

- `TrickGauge->Drain(DeltaTime)`
- si `IsEmpty()` → fail path (abajo)

Completed `TrickModeAction` (release):

- si `bTrickQTEActive` y QTE incompleto → fail path. Si DNA ya empezó, ignorar el release.

`TryTrickInput`:

- solo si `bTrickQTEActive`; durante DNA ignorar inputs QTE
- cuantizar Vector2D a cardinal (deadzone ~0.5; eje dominante)
- edge-trigger: no spamear el mismo frame; tratar `Triggered` como “dirección sostenida”, avanzar **una vez** por cambio de cardinal (guardar `LastAcceptedDirection`)
- cardinal neutro / dentro de deadzone → no avanzar ni fallar; resetear el cardinal anterior para permitir repetir una dirección tras soltarla
- dirección incorrecta → fail path inmediato
- match → `TrickQTEIndex++`, broadcast `OnTrickQTEIndexChanged(int32)`
- index == 4 → success: `bTrickQTEActive = false`, `EmptyGauge()`, broadcast `OnTrickQTEEnded(true)` y `DoAttack(DNA)`. No llamar todavía `ExitTrickModeInternal()`. Bloquear movimiento, ataques y look de combate hasta terminar DNA, independientemente del ActionState que use el ataque internamente.

Fail path (dirección incorrecta, tiempo agotado o release antes del éxito):

- `bTrickQTEActive = false`, `EmptyGauge()`, broadcast `OnTrickQTEEnded(false)`
- llamar `ExitTrickModeInternal()` inmediatamente, sin ejecutar DNA

`ExitTrickModeInternal()` — se llama al fallar el QTE o al terminar **todo DNA**, nunca entre sus fases:

- ActionState Default
- `PlayerController::ExitTrickMode()`
- aura OFF
- clear secuencia
- limpiar `bTrickQTEActive` y estado de cardinal; no volver a emitir `OnTrickQTEEnded` (se emitió al resolver el QTE)

**No** llamar `ToggleTrickAuraVFX` desde gameplay. Dejar la función y el `UPROPERTY TrickAuraDebugAction`; **no bindear** el debug action.

Delegates: en Eri o en un component nuevo `UTrickQTEComponent`. Preferir **meter estado en Eri** (ya tiene Enter/Exit/TryTrickInput) para no sumar un component que el BP no espera. Miembros **nombres nuevos**: `TrickQTESequence`, `TrickQTEIndex`, `bTrickQTEActive`, `LastQTECardinal`. No llamar nada `Direction` (choca con `SpriteDirectionComponent::Direction` en el owner graph, no en la clase, pero evitá el nombre igual).

`SetupPlayerInputComponent`: **sacar el BindAction de `HomingAction`** y de `TrickAuraDebugAction`. Dejar los UPROPERTY.

### Paso 5 — GameMode flow

**`SpinningRiot.h`**

- Enum: dejar `Combat`, dejar `Puzzle` marcado deprecado, **agregar al final** `Tutorial`, `GameWon`, `GameOver`.
- `BeginPlay`: fase `Tutorial` (o `Combat` si `TutorialWidgetClass` es null).
- `HandleEncounterCompleted` → `GameWon` (nunca Puzzle).
- `HandlePlayerDied()` público, lo llama CharacterBase en vez del pause-on-death.
- `FinishTutorial()` BlueprintCallable.
- `GetWonComboTier()`, `GetWonComboHits()`, `GetWonComboPoints()` — snapshot.
- `UPROPERTY` nuevas al final: `TutorialWidgetClass`, `GameWonWidgetClass`, `GameOverWidgetClass` (`TSubclassOf<UUserWidget>`).
- Crear/destruir widgets al entrar/salir de fase. Viewport z-order > HUD.

**`CharacterBase::HandleHealthDepleted`** (player): freeze AI **sí**. **No** `OpenPauseMenuOnDeath`. Avisar al GameMode `HandlePlayerDied()`.

**`ASpinningRiotPlayerController`**: dejar pause para Combat. `OpenPauseMenuOnDeath` puede quedar pero **nadie lo llama**. No borrar el UFUNCTION.

En GameWon / GameOver / Tutorial: `UEnemyAIComponent::SetGlobalAIFrozen(true)`. Al entrar a Combat desde Tutorial: unfreeze.

Input: Tutorial/Won/Over → `SetInputMode UIOnly` + cursor. Combat → `GameOnly` (como hoy el PC ya fuerza GameOnly en BeginPlay; el GameMode tiene que volver a UI cuando corresponda).

### Paso 6 — Homing crosshair en enemigos

**`AEnemyCharacter`**

- Agregar **constructor** (`AEnemyCharacter()`).
- `CreateDefaultSubobject<USceneComponent>(TEXT("HomingTargetMarker"))` attach al root. Nombre **`HomingTargetMarker`** — no `Sprite`, no `Mesh`, no `Root`.
- Opcional: `UWidgetComponent* HomingMarkerWidget` child del marker, `SetDrawAtDesiredSize`, space Screen. `TSubclassOf<UUserWidget> HomingMarkerWidgetClass`.
- Offset default `(0,0,80)`.
- `SetHomingTargeted_Implementation`: `HomingTargetMarker->SetHiddenInGame(!bTargeted)` + `SetVisibility(bTargeted)`. Empieza oculto en BeginPlay.
- No shadear `bIsHomingTargeted`.

**`IHomingable.h`**: borrar la frase del grapple. Enemies only.

### Paso 7 — HUD C++ surface (widgets los hace Franco)

Exponer BlueprintPure / delegates, no armar UMG:

- Trick gauge: ya hay `OnTrickGaugeChanged`
- QTE: delegates del paso 4
- Combo: ya hay `OnComboChanged` / `OnComboTierChanged` / `OnComboBroken`
- GameMode: fase + snapshot win
- `bDebugDrawCombo` default **false**

### Paso 8 — Borrar DNAInteractable

- Delete `Public/Interfaces/DNAInteractable.h`
- Delete `Private/Interfaces/DNAInteractable.cpp`
- Grep: no pueden quedar includes.

### Paso 9 — I-frames del player

En `UHealthComponent` o `ACharacterBase`, nombres nuevos (`InvulnDuration` default 0.8, `bIsInvulnerable`). Si el owner es player y recibe damage, prender invuln. Early-out si ya está invulnerable. No shadear `bIsDead`. Enemigos sin i-frames.

### Paso 10 — Homing: cancel al aterrizar

En `AEriCharacter` / landed: si `HomingAttackComponent->IsHomingInFlight()` → `CancelHomingAttack()`. El search ya pide airborne; esto cubre dash a medio camino contra el piso.

### Paso 11 — TV ranged (hoy casi no existe)

Hecho actual: `ExecuteRangedAttack` spawnea `AttackData.Ranged.Projectile` si hay class. **La AI nunca llama Ranged** — siempre `DoAttack(Normal)` a `AttackRange` (180). No hay actor proyectil.

Hacer:

1. **Nuevo** `AEnemyProjectile` (`EnemyProjectile.h/.cpp`): root sphere, speed (default 800), damage (default 1), lifetime (default 3), overlap ApplyDamage al player, ignore owner, Destroy. Nombres únicos.
2. **`UEnemyAIComponent`** — UPROPERTY nuevas al final:
   - `bUseRangedAttack` default false
   - `RangedAttackRange` default 600 (distancia a la que **quiere** estar para disparar)
   - `RangedStopDistance` default 450 (si está más cerca que esto y es ranged, no sigue caminando encima)
3. Lógica, si `bUseRangedAttack`:
   - Fuera de `DetectionRange`: idle (como hoy)
   - Distancia > `RangedAttackRange`: **acercarse** (mismo walk de ahora)
   - Distancia ≤ `RangedAttackRange`: parar, face player, `DoAttack(Ranged)` con token + cooldown
   - No melee. Si `DoAttack(Ranged)` falla (no projectile), log una vez.
4. Si `bUseRangedAttack` es false: melee actual, no tocar.
5. No editar BP_TV. Franco prende `bUseRangedAttack` y asigna el projectile class en el data asset / BP.

### Paso 12 — Cook maps (`DefaultGame.ini`)

Esto **no es gameplay**. Es la lista de mapas que Unreal **incluye cuando empaquetás** el juego. Hoy cookea `Title_test` y `Test`. El mapa de ship es `L_StylizedCity`, así que un pack con `Test` sale el nivel viejo.

En `Config/DefaultGame.ini`, sección `MapsToCook`:

- Dejar `/Game/Maps/Title_test`
- Sacar `/Game/Maps/Test`
- Agregar `/Game/SylizedCity/Scene/L_StylizedCityMap` (el path real del umap; la carpeta se llama `SylizedCity` con una sola `t`)

No cambiar `GameDefaultMap` (sigue siendo el title). El OpenLevel del menú lo cablea Franco.

### Paso 13 — HUD C++ listo

Ya cubierto en paso 7. Confirmar delegates de QTE + `bDebugDrawCombo = false`.

### Paso 14 — `Docs/06-para-fran.md` (obligatorio, último)

Escribir un markdown **extenso**, paso a paso, para que Franco deje el juego corriendo **sin adivinar**. Tiene que reflejar **lo que el agente realmente implementó** (nombres de properties, paths, enums), no copiar este 05 en abstracto.

Secciones mínimas (si falta una, el md está incompleto):

1. **Compilar** — abrir el `.uproject` con UE 5.7, qué módulo, qué hacer si sale shadow/duplicate component. No pegar comandos de build; Franco compila desde el editor.
2. **Input** — `IMC_TrickMode`: qué actions tiene que tener y cuáles **no**. `IA_Homing` y `IA_TrickVFXDebug` ya no se bindean (pueden quedar asignados en el BP). **No tocar `IA_Pause`.**
3. **BP_EriCharacter** — cada property nueva (DNA anims, extra yoyos si se crearon, AttackData). Dónde se ven en el details panel. Qué mesh/Niagara ponerle a yoyos extra si existen. Aura: ya no es debug toggle.
4. **EriCharacterAttackData** — llenar DNA (durations, burst 6, range, speed, damage).
5. **Enemigos** — `HomingTargetMarker`: cómo se ve en viewport, asignar widget/mesh de crosshair, offset. **BP_TV** (o el AI component del TV): `bUseRangedAttack = true`, `RangedAttackRange`, projectile class. LongNeck/Tengu: dejar `bUseRangedAttack` false.
6. **Proyectil del TV** — crear BP hijo de `AEnemyProjectile` si el agente hizo la clase C++, asignar mesh/sphere, meterlo en `RangedEnemyAttackData.Projectile` o el AttackData del TV.
7. **GameMode `GM_SpinningRiot`** — asignar `TutorialWidgetClass`, `GameWonWidgetClass`, `GameOverWidgetClass`. Qué hacer si el BP todavía referencia `Puzzle`.
8. **Widgets a crear/editar** (uno por uno: para qué fase, qué bind C++, qué botones, qué art ya existe):
   - Tutorial 2 slides (`tutorial_1`, `tutorial_2`) → `FinishTutorial`
   - QTE 4 flechas (`Arrow_up/down/left/right`) → delegates de Eri
   - Trick gauge → `OnTrickGaugeChanged` (hold solo si llena)
   - Combo HUD → `ComboComponent`
   - Game Won → rank snapshot del GameMode
   - Game Over → **nuevo**, distinto a pause y a win. Pause **no** se usa en death.
9. **Title** — `WBP_MainMenu` OpenLevel a `L_StylizedCity` (`/Game/SylizedCity/Scene/L_StylizedCityMap`).
10. **Cook** — confirmar que `DefaultGame.ini` ya lo cambió el agente; si Franco empaqueta, esos dos mapas.
11. **PIE checklist** — lista tildable: tutorial 2 slides → jugar; pegar enemigos llena gauge; hold trick → 4 flechas → DNA fase1 órbita + fase2 6 yoyos, con input bloqueado hasta terminar todo el ataque; soltar el hold durante DNA no lo cancela; fail por dirección incorrecta, tiempo o release previo al éxito devuelve el input inmediatamente y vacía la gauge sin atacar; vector neutro no falla; homing light-en-aire + crosshair en el elegido; TV se acerca y pega shot; muerte → UI game over (no pause); oleadas done → UI win + rank; pause (Esc) sigue funcionando in-combat.
12. **Lo que el agente eligió** — DNA 6 yoyos: ¿spawn transitorio o components extra? Nombres exactos. Cualquier default raro.
13. **No hacer** — no mergear ramas, no reactivar Puzzle, no asignar Dog Walk/Boing, no usar pause como game over.

Tono: imperativo, numerado, un paso = una acción en el editor. Si hay que crear un widget, decir de qué clase C++ hereda (UserWidget), qué funciones llamar, y qué assets de `Content/Assets/HUD_Assets` usar.

---

## Orden compacto para el agente

```
1.  TrickTypes + DNA enum/data (dos fases, burst 6)
2.  TrickGauge fill/drain + NotifyHit fill
3.  DNA fase1 órbita-en-Eri + fase2 6 yoyos asterisco
4.  Eri QTE + IMC enter/exit + unbind Homing y TrickVFXDebug
    (NO unbind del pause)
5.  GameMode Tutorial / Combat / GameWon / GameOver
6.  Death → GameOver (no pause)
7.  HomingTargetMarker en EnemyCharacter
8.  Borrar DNAInteractable
9.  I-frames player
10. Homing cancel al aterrizar
11. TV ranged: AEnemyProjectile + AI se acerca a RangedAttackRange y dispara
12. DefaultGame.ini MapsToCook → Title_test + L_StylizedCity
13. Delegates HUD; bDebugDrawCombo false
14. Escribir Docs/06-para-fran.md (guía editor exhaustiva)
```

No paralelizar pasos que tocan el mismo .h (Eri, GameMode, AttackTypes). **No tocar items.**

El detalle de qué hace Franco en el editor **no va acá**: va en `06-para-fran.md`, escrito al final con los nombres reales.

---

## Fuera de v1 (no implementar)

- Dog Walk, Boing, elegir trick
- Grapple, puertas DNA
- Boss, más mapas, story, save de progreso
- Pathfinding, 8 dir de enemigos
- Cambios a pickups / jihanki / bob de items
