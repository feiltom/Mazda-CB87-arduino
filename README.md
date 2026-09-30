# Jauge de compteur Mazda CB87 — Arduino Uno & ESPHome

Pilotage d'une aiguille de compteur de voiture directement par un Arduino Uno.

Le moteur d'aiguille **n'est pas un moteur pas à pas** : c'est une jauge **air-core**
(bobines croisées), bornes `C+ C- S+ S-` (Cosinus / Sinus), environ 205 Ω par bobine.
L'aiguille s'aligne sur le champ des deux bobines : pour un angle θ, on envoie
un courant ∝ cos θ dans la bobine C et ∝ sin θ dans la bobine S.

## Démo

https://github.com/user-attachments/assets/f71c3ea1-1e5e-48d8-9546-62c2b9e05537

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

## ESPHome : composant `aircore_gauge`

Le dossier `components/aircore_gauge` est un composant externe ESPHome qui pilote
les jauges air-core à partir de sorties PWM (`ledc` sur ESP32, en direct ou via un DRV8833).

- jauges **4 fils** (`C+ C- S+ S-`) ou **3 fils** (`SIN COS COM`, commun tenu à mi-tension)
- conversion valeur → angle (ex. 0–240 km/h → 0–270°), vitesse d'aiguille réglable
- entité `number` pour Home Assistant, actions `aircore_gauge.set_value` / `set_angle`,
  ou suivi automatique d'un capteur (`sensor:`)

```yaml
external_components:
  - source: github://feiltom/Mazda-CB87-arduino@main
    components: [aircore_gauge]

output:
  - { platform: ledc, id: cos_pos, pin: GPIO25, frequency: 25000Hz }
  - { platform: ledc, id: cos_neg, pin: GPIO26, frequency: 25000Hz }
  - { platform: ledc, id: sin_pos, pin: GPIO27, frequency: 25000Hz }
  - { platform: ledc, id: sin_neg, pin: GPIO14, frequency: 25000Hz }

aircore_gauge:
  - id: speedo
    cos_pos: cos_pos
    cos_neg: cos_neg
    sin_pos: sin_pos
    sin_neg: sin_neg
    zero_offset: -38
    min_value: 0
    max_value: 240
    max_angle: 270

number:
  - platform: aircore_gauge
    gauge_id: speedo
    name: "Compteur"
```

Exemple complet : [`esphome/mazda-gauge.yaml`](esphome/mazda-gauge.yaml).

| Option | Défaut | Rôle |
|--------|--------|------|
| `cos_pos` `cos_neg` `sin_pos` `sin_neg` | — | sorties d'une jauge 4 fils |
| `cos` `sin` `common` | — | sorties d'une jauge 3 fils (à la place des 4 ci-dessus) |
| `zero_offset` | `0` | décalage en degrés pour caler le 0 du cadran |
| `max_power` | `80%` | PWM max, limite le courant dans les bobines |
| `speed` | `180` | vitesse de l'aiguille en °/s (`0` = instantané) |
| `min_value` / `max_value` | `0` / `100` | échelle de la valeur affichée |
| `min_angle` / `max_angle` | `0` / `270` | angles correspondants sur le cadran |
| `sensor` | — | capteur suivi automatiquement par l'aiguille |

En direct sur un ESP32 (3,3 V), une bobine de 205 Ω consomme ~16 mA : pas besoin de driver,
mais l'aiguille a un peu moins de couple qu'en 5 V. Pour du 5 V, intercaler un **DRV8833**
(une bobine par pont en H, nSLEEP à l'état haut).
