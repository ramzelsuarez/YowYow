# Checkpoint 2 — Checklist PIE + trabajo manual en editor

## Trabajo manual en editor (obligatorio)

### BP Eri
- [ ] Agregar **2** componentes mesh yoyo (Static Mesh o Scene) Right / Left, rest poses en viewport.
- [ ] Asignar en defaults C++: **YoYo Right**, **YoYo Left**.
- [ ] Quitar el viejo yoyo C++ si el BP aún lo tenía (ya no se crea en C++).
- [ ] **Un solo** `AttackComponent` (BP).
- [ ] `HealthComponent`, `CharacterStateComponent`, `HomingAttackComponent` en BP.
- [ ] `AreaAttackAction` + binding IMC.
- [ ] Opcional: `DamageCameraShake`.

### Data asset de ataque Eri (`EriCharacterAttackData`)

| Entry | Motion | YoYoHand | Otros sugeridos |
|-------|--------|----------|-----------------|
| Normal[0] | FollowSource | Right | Damage/Range/Speed/Knockback/HitStop |
| Normal[1] | FollowSource | Left | idem |
| Normal[2] | FollowSource | Both | idem |
| Area | OrbitCircle (o dejar ArcSweep → C++ lo sube a Orbit) | Both (visual) | **Damage bajo**, **RecoveryTime alto** (anti-spam) |

### Enemigos
- [ ] Attack data: Motion **ArcSweep** (default).
- [ ] Un AttackComponent en BP.

---

## Checklist PIE (CP2)

- [ ] Normal combo: yoyo **derecho → izquierdo → ambos** (según data).
- [ ] Yoyo va y vuelve **recto** (body no gira a mitad del golpe).
- [ ] No move durante Attacking; tampoco en Recovery de area.
- [ ] Area: círculo, recovery larga, menos daño, resetea combo.
- [ ] Enemigo: medialuna ArcSweep sin yoyos.
- [ ] Spam click no apila hitboxes rotos.
- [ ] Sin Cast a Eri en AttackComponent (si falla FollowSource: warning de sources).

## Aún no (CP3)
- Homing camera lock detrás de Eri.

## También pendiente
- [ ] Checklist completa de CP1: `checkpoint-1-pie-checklist.md` (mismo directorio).
