# Checkpoint 1 — Checklist PIE (pendiente de probar completo)

Guardada para retomar cuando quieras. CP2 se implementó sin esperar esta smoke completa.

## Setup mínimo en editor

1. **BP Eri / Character**
   - `HealthComponent` presente (MaxHealth = 3).
   - Opcional: asignar **Damage Camera Shake**.
   - **Un solo** `AttackComponent` (ya no se auto-monta desde C++).

2. **Attack data**
   - `Damage` ≥ 1  
   - `Knockback` > 0 para notar launch  
   - `HitStop Duration` ~ 0.08  

3. Enemigo en el mapa con health + collision Pawn.

## Checklist PIE (CP1)

### Daño
- [ ] Eri pega a enemigo → baja vida.
- [ ] Enemigo pega a Eri → baja vida.
- [ ] A 0 HP → dead / LifeState Dead.

### Feel de hit
- [ ] Hitstop al conectar.
- [ ] Knockback si data > 0.

### Eri yoyo (bridge CP1; CP2 cambia dual yoyo)
- [ ] Ataque normal mueve yoyo / hit conecta.
- [ ] Enemigo: ArcSweep medialuna.

### Camera shake
- [ ] Sin shake class → no crash.
- [ ] Con shake + te pegan → cámara tiembla.

### No esperado en CP1 (son CP2+)
- Dual yoyo R→L→Both  
- Lock rotación  
- Area orbit pro  
- Homing camera lock  

## Tips debug
- No baja vida → hit no toca Pawn o Damage = 0.
- No hitstop → HitStopDuration = 0.
- Falta AttackComponent en BP → warning en log, no ataca.
