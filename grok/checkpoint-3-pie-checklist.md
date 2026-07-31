# Checkpoint 3 — Homing camera lock (PIE)

## Qué se implementó (código)

- Cámara **locked detrás de Eri** durante dash homing (yaw sigue dirección al target / velocity).
- **Unlock** al bounce (`FinishHomingAttack` → `OnHomingAttackFinished(true)`).
- **Unlock** al cancel/fail (target perdido, aterrizaje mid-flight → `CancelHomingAttack` → finished false).
- `Look`: yaw bloqueado en lock; pitch opcional (`bHomingCameraLockPitch`).
- Tunables en Eri: `HomingCameraYawInterpSpeed`, `bHomingCameraLockPitch`.

## PIE checklist

- [ ] En aire con target: attack → dash; no podés orbitar la cámara alrededor a mitad del vuelo.
- [ ] Vista se siente “espalda” (sprite back) durante el dash.
- [ ] Al rebotar: look libre de nuevo + falling.
- [ ] Aterrizar / perder target mid-dash: cámara no queda trabada.
- [ ] Regression: yoyo R/L/Both ida-vuelta, area, daño, hitstop, enemigo arc.

## Manual editor

- [ ] PaperZD: pose espalda en ActionState Homing (si hace falta en ABP).
- [ ] Tunear `Homing Camera Yaw Interp Speed` si el giro se siente lento/brusco.
- [ ] Tunear HitStop/Knockback/Recovery en attack data tras feel global.

## Scope completo del PR (recordatorio)

CP1 + CP2 + CP3 en código. Content/data assets y smoke PIE siguen siendo tuyos.
