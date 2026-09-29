# Jauge de compteur Mazda CB87 — Arduino Uno

Pilotage d'une aiguille de compteur voi directement par un Arduino Uno.

Le moteur d'aiguille **n'est pas un moteur pas à pas** : c'est une jauge **air-core**
(bobines croisées), bornes `C+ C- S+ S-` (Cosinus / Sinus), environ 205 Ω par bobine.
L'aiguille s'aligne sur le champ des deux bobines : pour un angle θ, on envoie
un courant ∝ cos θ dans la bobine C et ∝ sin θ dans la bobine S.

## Démo

[▶ Voir la vidéo de l'aiguille en fonctionnement](compteur.mp4)

## Câblage

| Jauge | Uno |
|-------|-----|
| C+    | D3  |
| C-    | D9  |
| S+    | D11 |
| S-    | D10 |

Chaque paire de broches forme un pont en H : l'une reçoit le PWM, l'autre est à 0 V,
selon le signe du cosinus / sinus. Courant limité à ~19 mA par bobine (`pwmMax = 200`).
Le PWM tourne à ~31 kHz (timers 1 et 2) pour supprimer le sifflement des bobines.

## Utilisation

```sh
pio run -t upload       # compiler et téléverser
pio device monitor      # moniteur série (115200)
```

Commandes dans le moniteur :

| Commande | Effet |
|----------|-------|
| `90`     | aller à 90° (0–360), en douceur |
| `s`      | balayage 0 → 270° → 0 |
| `o -38`  | décalage du zéro (valeur calibrée dans le code : -38) |
| `m 150`  | PWM max 0–255 (réduire si la jauge chauffe) |

Fermer le moniteur (`Ctrl+C`) avant chaque téléversement.

## Suite possible

Passage sur ESP32-C3 SuperMini avec un module **DRV8833** (double pont en H,
entrées compatibles 3,3 V) : AIN1/AIN2 → GPIO 4/5 (bobine C), BIN1/BIN2 → GPIO 6/7 (bobine S),
VM sur 5 V, nSLEEP à l'état haut.
