# PLANE page — Home Assistant setup

The **PLANE** page (page 7 on the vertical 64x64 display) shows the aircraft
currently closest to the pool: **origin → destination airport (IATA), airline,
and ground speed**. When no aircraft is nearby it shows **"No plane"**.

The ESP32 does **not** call any flight API itself. All the flight logic lives in
Home Assistant, exactly like every other value on the display. This document lists
**everything you must create in Home Assistant** to make the page work. No YAML
files are required — it's all done through the HA UI.

## What you create (overview)

| # | Type | Name | Resulting entity | Read by ESPHome as |
|---|------|------|------------------|--------------------|
| 1 | HACS integration | FlightRadar24 | `sensor.flightradar24_current_in_area` | (source only) |
| 2 | Helper → Toggle | `Pool LED Display Plane` | `input_boolean.pool_led_display_plane` | `display_plane_enabled` |
| 3 | Helper → Template → Sensor | `Plane Overhead Route` | `sensor.plane_overhead_route` | `plane_route` |
| 4 | Helper → Template → Sensor | `Plane Overhead Airline` | `sensor.plane_overhead_airline` | `plane_airline` |
| 5 | Helper → Template → Sensor | `Plane Overhead Callsign` | `sensor.plane_overhead_callsign` | `plane_callsign` |
| 6 | Helper → Template → Sensor | `Plane Overhead Speed Kmh` | `sensor.plane_overhead_speed_kmh` | `plane_speed` |
| 7 | Helper → Template → Sensor | `Plane Overhead Category` | `sensor.plane_overhead_category` | `plane_category` |

> The entity IDs must match the right-hand column exactly, because the ESPHome
> config (`pool-led-display.vertical-64x64.yaml`) references them by those IDs.
> HA derives the entity ID from the **Name** you type, so use the names above.

---

## Step 1 — Install the FlightRadar24 integration (HACS)

Source: <https://github.com/AlexandrErohin/home-assistant-flightradar24>
(Free, no API key. Uses FlightRadar24's unofficial API — personal/educational use.)

1. In HACS, add/search **Flightradar24** and install it, then restart HA.
2. **Settings → Devices & Services → + Add Integration → Flightradar24.**
3. Configure:
   - **Latitude / Longitude** → the pool's coordinates.
   - **Radius** → start with **15–25 km** (how far out to look for "overhead" planes).
   - **Min / Max altitude** (optional) → e.g. min a few hundred metres to skip
     ground traffic; leave max open.
   - **Scan interval** → **~30 s**.

This creates `sensor.flightradar24_current_in_area`. Its state is the number of
flights in the area; its **`flights`** attribute is a list of flight objects, each
with `callsign`, `airline`, `airport_origin_code_iata`,
`airport_destination_code_iata`, `ground_speed` (knots), `distance` (km), etc.

Verify in **Developer Tools → States**: find `sensor.flightradar24_current_in_area`
and confirm the `flights` attribute fills with nearby aircraft (cross-check against
the flightradar24.com map for your area).

---

## Step 2 — Create the toggle helper

**Settings → Devices & Services → Helpers → + Create Helper → Toggle**

- **Name:** `Pool LED Display Plane` → creates `input_boolean.pool_led_display_plane`.

Turning this on jumps to the PLANE page; while on, the page is included in the
30-second rotation.

---

## Step 3 — Create the five template sensor helpers

For each one: **+ Create Helper → Template → Template a sensor**, set the **Name**,
paste the **State template**, and (for speed) set the **Unit of measurement**.

Each template picks the **closest** flight within **12 km** (smallest `distance`,
filtered with `selectattr('distance', 'le', 12)`) from the FR24 sensor's `flights`
list and returns one field. They render to an empty value when nothing qualifies,
which the display shows as "No plane".

> **Why the 12 km cutoff?** The FlightRadar24 integration's "radius" setting
> actually defines a *square* bounding box, not a circle — so a flight near a
> corner of the box can be up to `radius × √2` away in a straight line (e.g. a
> 25 km radius lets flights as far as ~35 km register as "in area"). Without a
> distance cutoff in these templates, the display would confidently show a plane
> that's actually a speck on the horizon (or invisible entirely at cruise
> altitude). All five templates apply the *same* 12 km cutoff so they always
> agree on which flight — if only one template were filtered, you could end up
> with e.g. a nearby callsign paired with a distant flight's route. Tune the `12`
> to taste; smaller shows fewer but more plausibly-visible aircraft.

### 3a. `Plane Overhead Route`  → `sensor.plane_overhead_route`
State template:
```jinja
{% set flights = state_attr('sensor.flightradar24_current_in_area', 'flights')
   | default([], true) | rejectattr('distance', 'none')
   | selectattr('distance', 'le', 12) | list %}
{% if flights | count > 0 %}
  {% set f = flights | sort(attribute='distance') | first %}
  {% set o = f.airport_origin_code_iata or '' %}
  {% set d = f.airport_destination_code_iata or '' %}
  {{ (o ~ '>' ~ d) if (o or d) else '' }}
{% endif %}
```

### 3b. `Plane Overhead Airline`  → `sensor.plane_overhead_airline`
State template:
```jinja
{% set flights = state_attr('sensor.flightradar24_current_in_area', 'flights')
   | default([], true) | rejectattr('distance', 'none')
   | selectattr('distance', 'le', 12) | list %}
{% if flights | count > 0 %}
  {% set f = flights | sort(attribute='distance') | first %}
  {{ f.airline or f.airline_short or '' }}
{% endif %}
```

### 3c. `Plane Overhead Callsign`  → `sensor.plane_overhead_callsign`
State template:
```jinja
{% set flights = state_attr('sensor.flightradar24_current_in_area', 'flights')
   | default([], true) | rejectattr('distance', 'none')
   | selectattr('distance', 'le', 12) | list %}
{% if flights | count > 0 %}
  {% set f = flights | sort(attribute='distance') | first %}
  {{ (f.callsign or '') | trim }}
{% endif %}
```

### 3d. `Plane Overhead Speed Kmh`  → `sensor.plane_overhead_speed_kmh`
**Unit of measurement:** `km/h`
State template (FR24 reports knots; this converts to km/h):
```jinja
{% set flights = state_attr('sensor.flightradar24_current_in_area', 'flights')
   | default([], true) | rejectattr('distance', 'none')
   | selectattr('distance', 'le', 12) | list %}
{% if flights | count > 0 %}
  {% set f = flights | sort(attribute='distance') | first %}
  {{ ((f.ground_speed | float(0)) * 1.852) | round(0) | int }}
{% endif %}
```

### 3e. `Plane Overhead Category`  → `sensor.plane_overhead_category`
State template (drives the helicopter vs. airplane icon on the display):
```jinja
{% set flights = state_attr('sensor.flightradar24_current_in_area', 'flights')
   | default([], true) | rejectattr('distance', 'none')
   | selectattr('distance', 'le', 12) | list %}
{% if flights | count > 0 %}
  {% set f = flights | sort(attribute='distance') | first %}
  {{ f.aircraft_category or '' }}
{% endif %}
```
FR24 reports this as `Airplane`, `Helicopter`, etc. — the display only special-cases
the exact string `Helicopter` (see `pool-led-display.vertical-64x64.yaml`), anything
else just draws the regular plane icon.

---

## Verify

1. **Developer Tools → States** → the five `sensor.plane_overhead_*` entities exist
   and resolve to the closest flight when one is nearby, and go blank when it's clear.
2. If an entity ID came out different from the table above (e.g. a trailing `_2`),
   open the helper and rename its **entity ID** to match.
3. On the device, flip `input_boolean.pool_led_display_plane` and confirm the
   PLANE page appears with route / airline / speed.

## Troubleshooting

- **Sensors always blank:** confirm `sensor.flightradar24_current_in_area` exists and
  its `flights` attribute has data. If your HA language isn't English the FR24 entity
  may be localized (e.g. `sensor.flightradar24_bereich_...`) — update the entity name
  in all five templates to match.
- **Page never shows:** make sure the toggle `input_boolean.pool_led_display_plane` is
  on and that its entity ID matches.
- **Route shows only `>`:** FR24 had no origin/destination for that flight; the display
  falls back to the callsign. This is expected for some flights.
- **Route/airline look stale or wrong:** FR24's data is crowd-sourced and best-effort;
  occasional gaps or wrong routes are normal.
- **Nothing shows even though a plane is nearby:** it's probably farther than the
  12 km cutoff in the templates (see the bounding-box note above) — check the
  `distance` field on the closest entry in `sensor.flightradar24_current_in_area`'s
  `flights` attribute to confirm, and loosen the `12` if you want a wider net.
