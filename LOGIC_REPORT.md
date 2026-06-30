# Smart Traffic Junction — How the Logic Works

> A plain-English walkthrough of what this code actually does, with worked
> examples. You don't need to know C++ to follow this — if you understand
> "cars arrive at a junction and the lights decide who goes," you're good.

---

## 1. The Big Picture (read this first)

Imagine a normal **4-way crossroads**: traffic comes from **North, South, East,
and West**. Our board (an ESP32 microcontroller) is the "brain" that decides
which direction gets a **green light**, and for **how long**.

The clever part: instead of dumb fixed timers ("green for 30s no matter what"),
this system **watches how many cars are actually waiting** and gives more green
time to the busier road — while still being **fair** so no road waits forever.

### Two "lanes", not four

The four roads are grouped into **two lanes**, because opposite roads always
move together (just like real traffic lights):

| Lane | Roads it controls | Light group |
|------|-------------------|-------------|
| **Lane A** | North + South | "NS" lights |
| **Lane B** | East + West | "EW" lights |

So at any moment, **either Lane A is green or Lane B is green — never both.**
When one is green, the other is red. Simple.

---

## 2. How does it know how many cars are waiting?

Every road has **two sensors**: one at the **entry (IN)** and one at the
**exit (OUT)**. Think of them like the rubber tube counters at petrol stations,
or magnetic loops buried in the road.

The rule is dead simple:

- A car **enters** the queue → the IN sensor fires → **count goes UP (+1)**
- A car **leaves** the junction → the OUT sensor fires → **count goes DOWN (−1)**
- The count can never go below 0 (you can't have −1 cars).

So the "count" for a road is simply: **how many cars are currently sitting there
waiting.**

```
northCount = (cars that entered North)  −  (cars that left North)
```

Then we add opposite roads together to get each lane's total demand:

```
Lane A demand (countA) = northCount + southCount
Lane B demand (countB) = eastCount  + westCount
```

> **Worked example:** 3 cars arrive from North, 2 from South, and 1 of them
> drives away. `northCount = 3`, `southCount = 2 − 1 = 1`, so
> `countA = 3 + 1 = 4`. Lane A currently has **4 cars waiting**.

### A small but important detail: "debounce"

Real sensors are noisy — a single car can make the sensor flicker on/off many
times in a few milliseconds. If we counted every flicker we'd think one car was
fifty cars. So the code **ignores any change that happens within 50 milliseconds
of the previous one** and only counts a clean "road clear → road blocked"
transition. This is called **debouncing**, and it's why the counts stay
accurate.

*(In the code this lives in `sensors.cpp`. You don't need to touch it.)*

---

## 3. The traffic-light cycle (the "state machine")

The system is always in exactly **one** of these five situations (called
"states"). It walks through them in a loop:

```
   ┌──────────────┐
   │ LANE A GREEN │  ← North/South can go
   └──────┬───────┘
          │ (green time runs out)
   ┌──────▼────────┐
   │ LANE A YELLOW │  ← 2 seconds warning
   └──────┬────────┘
          │
   ┌──────▼───────┐
   │ DECIDE PHASE │  ← brief moment, ALL RED, brain picks next lane
   └──────┬───────┘
          │ (it might pick A again, or switch to B)
   ┌──────▼────────┐
   │ LANE B GREEN  │  ← East/West can go
   └──────┬────────┘
          │
   ┌──────▼────────┐
   │ LANE B YELLOW │  ← 2 seconds warning
   └──────┬────────┘
          │
   ┌──────▼───────┐
   │ DECIDE PHASE │  ← again, pick next lane...
   └──────────────┘
        (loops forever)
```

Key points:

- **Green** = go. Its length is *calculated* (see Section 4).
- **Yellow** = fixed **2 seconds** warning before stopping.
- **DECIDE PHASE** = a tiny "everyone stop" moment where the brain runs its
  decision (see Section 5). Both directions are red during this instant for
  safety.
- After deciding, it's totally allowed to give the **same lane green again** if
  that lane is still the busiest. It does **not** have to alternate.

---

## 4. How long does the green light last?

The green duration is **not** fixed. The busier the lane, the longer the green —
up to a safety cap. The formula:

```
green time = BASE TIME + (number of cars × bonus per car)
           = 10 seconds + (count × 0.5 seconds)
```

…but it is **never allowed to exceed 20 seconds** (the cap), no matter how busy.

> **Worked examples:**
> - 4 cars waiting → `10 + (4 × 0.5) = 12 seconds` of green.
> - 0 cars waiting → `10 + 0 = 10 seconds` (the minimum).
> - 50 cars waiting → `10 + 25 = 35`, but that's over the cap, so it becomes
>   **20 seconds**.

This means a busy road gets more time, but one massively busy road can't hog the
junction forever.

*(These numbers — 10s base, 0.5s per car, 20s cap — are all set in `config.h`
and can be tuned without rewriting any logic.)*

---

## 5. The heart of it: which lane goes next? (the "score")

This is the most important part. When the system reaches **DECIDE PHASE**, it
has to choose: give green to Lane A or Lane B?

It does this in **two layers**.

### Layer 1 — The "score" (normal case)

Each lane gets a **score**. Higher score wins. The score balances **two things
we care about**:

1. **Demand** — how many cars are waiting (we want to clear busy roads).
2. **Fairness** — how long this lane has already been waiting (we don't want to
   ignore a quiet road forever).

The formula:

```
score = number of cars + (0.5 × seconds this lane has been waiting)
```

The `0.5` is a "fairness weight". A bigger number would make waiting time matter
more; a smaller number would make car-count matter more. Right now they're
balanced so that **every 2 seconds of waiting counts as much as 1 extra car.**

> **Worked example:**
> - Lane A: 6 cars, waited 4 seconds → `score = 6 + (0.5 × 4) = 8`
> - Lane B: 3 cars, waited 20 seconds → `score = 3 + (0.5 × 20) = 13`
>
> Even though Lane A has **more cars**, Lane B wins because it has been **waiting
> much longer** — that's the fairness part kicking in. Lane B goes green next.

### Layer 2 — The "anti-starvation" override (emergency fairness)

There's a worst-case worry: what if one lane is always a *little* busier, so the
score keeps picking it, and the other lane **never** gets a turn? People would
sit at a red light forever. That's called **starvation**.

To prevent it, there's a hard rule that is checked **before** the score:

> **If any lane has been waiting `45 seconds or more`, it is FORCED green,
> regardless of how few cars it has.**

- If **only one** lane is starved → that lane is forced green.
- If **both** are starved (≥45s) → the one that has waited **longer** goes first.

> **Worked example:** Lane B has 0 cars but has been stuck on red for 46 seconds.
> Normally its score would be tiny and it'd keep losing. But the override kicks
> in: **Lane B is forced green** so those waiting drivers finally move.

### Tie-breaker

If the two scores come out **exactly equal**, the lane that has **waited longer**
wins. If even that is equal, **Lane A** wins by default. (This just guarantees
the system never freezes on an impossible "can't decide" situation.)

### The full decision order (cheat sheet)

```
1. Is either lane starved (waited ≥ 45s)?  → force that lane (longer wait if both)
2. Otherwise, higher SCORE wins
3. If scores tie → longer WAIT wins
4. If still tied → Lane A wins
```

---

## 6. The two number displays

There are two little 4-digit screens (one for NS, one for EW). They show a
**countdown of how many seconds are left** for whichever lane is currently
active (green or yellow). The lane that is stopped just shows `0`. This is purely
for humans to see — it has no effect on the decisions.

---

## 7. The serial "debug" printout

Once per second the board prints a status line to the computer, e.g.:

```
[STATUS] N:3 S:1 E:0 W:2 | countA:4 countB:2 | waitA:5s waitB:18s | State:0 | GreenDur:12000 | Last:Lane A won by score
```

This is just a live report so we can watch what the brain is thinking:
the per-road counts, the lane totals, how long each lane has waited, the current
state, the green duration (in milliseconds), and the reason for the last
decision. Very handy for the demo and for debugging.

---

## 8. One-paragraph summary (if you only read one thing)

> The board groups the 4 roads into 2 lanes (North+South, East+West) and lets
> only one lane go green at a time. Sensors count how many cars are waiting on
> each road. When it's time to choose who goes next, each lane gets a **score =
> cars waiting + a fairness bonus for how long it's been stuck**. The higher
> score wins, the green light then lasts longer for busier lanes (10–20s), and a
> safety rule guarantees that **no lane ever waits more than 45 seconds** before
> it's forced green. That's the whole idea: **react to real traffic, but stay
> fair.**

---

## Appendix: where each piece lives in the code

| What it does | File |
|--------------|------|
| Boots the board, runs the main loop | `src/main.cpp` |
| Counts cars from the sensors (with debounce) | `src/sensors.cpp` |
| The state machine + scoring + decision + lights | `src/traffic.cpp` |
| Shared variables (counts, current state, timers) | `src/state.cpp` |
| The countdown number displays | `src/display.cpp` |
| The 1-second status printout | `src/debug.cpp` |
| All the tunable numbers (times, weights, pins) | `include/config.h` |
