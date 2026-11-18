# Design System Specification: Crowd Pressure Radar

## 1. Product Type Classification
- **Domain:** Public Safety / Incident Management & Real-Time IoT Operations
- **Source Classification:** `ui-ux-pro-max/data/products.csv` (Row 166: Status Page / Incident Management + Row 158: Emergency SOS & Safety + Row 24: Real-Time IoT Operations).
- **Core User Archetype:** Festival ground marshals, temple queue supervisors, and security gate operators working in time-pressured, high-distraction environments.
- **Cognitive Requirement:** The **Two-Second Rule** — An operator must identify the operational safety state of every queue zone within 2 seconds without cognitive decoding of decorative elements.

---

## 2. Style Selection: Swiss Minimalist & Flat Data Density
- **Selected Style:** `Minimalism & Swiss Style` (Style ID: 1) merged with `Data-Dense Dashboard` (Style ID: 28) and `Accessible & Ethical` (Style ID: 8).
- **Aesthetic Principles:**
  - **Flat and Calm:** Neutral matte surfaces, 1px crisp separation borders, zero heavy drop-shadows.
  - **Zero Decorative Slop:** No gradient fills, no gradient text, no glowing drop-shadows, no glassmorphic blurs (`backdrop-filter: none`), no background particle grids, no bouncing/pulsing animations without status meaning.
  - **Status-Restricted Color:** Color is strictly functional and reserved for operational status (Clear, Building up, Danger, Focus/Active). Neutral surfaces are completely monochromatic.
  - **Information Density with Breathing Room:** High information throughput using structured grids, tabular figures, and plain-language annotations.

---

## 3. Color Palette & WCAG AA Contrast Ratios

### Surface & Neutral Architecture
| Token Name | Light Theme Hex | Dark Theme Hex | Contrast on Respective Surface | Role |
| :--- | :--- | :--- | :--- | :--- |
| `surface-base` | `#F8FAFC` | `#0B0F17` | Baseline | Canvas background |
| `surface-raised` | `#FFFFFF` | `#111827` | 1.1:1 vs base | Cards, panels, dialogs |
| `surface-sunken` | `#F1F5F9` | `#070A10` | 1.1:1 vs base | Inset inputs, code blocks, radar scope |
| `border-subtle` | `#E2E8F0` | `#1F2937` | 1.4:1 vs base | 1px clean separation lines |
| `border-strong` | `#94A3B8` | `#374151` | 3.2:1 vs base | Interactive boundaries, inputs |
| `text-primary` | `#0F172A` | `#F9FAFB` | **15.2:1** (Light) / **17.8:1** (Dark) | Primary labels, numbers, titles |
| `text-secondary` | `#475569` | `#94A3B8` | **6.1:1** (Light) / **6.8:1** (Dark) | Explanations, units, timestamps |
| `text-muted` | `#64748B` | `#6B7280` | **4.6:1** (Light) / **4.7:1** (Dark) | Metadata, helper text |
| `focus-ring` | `#2563EB` | `#3B82F6` | **6.4:1** vs base | Keyboard accessibility focus indicator |

### Quad-Coded Status Tokens (Color + Icon + Word + Shape)
In compliance with WCAG 2.2 AA (Criterion 1.4.1 Non-text Contrast & Use of Color), status is never communicated by color alone. Every status incorporates:
1. **Color** ($\ge 4.5:1$ contrast against surface)
2. **Icon** (Check, Triangle Alert, Octagon Alert)
3. **Word** ("Clear", "Building up", "Danger, barrier closed")
4. **Shape** (Pill, Square, Octagon container)

| Status | State Label | Text Hex (Light) | Text Hex (Dark) | Background (Light) | Background (Dark) | Border | Contrast Ratio |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **GREEN** | `Clear` | `#15803D` | `#4ADE80` | `#F0FDF4` | `#052E16` | `#86EFAC` / `#166534` | **5.4:1** (Light) / **7.8:1** (Dark) |
| **AMBER** | `Building up` | `#B45309` | `#FBBF24` | `#FFFBEB` | `#451A03` | `#FDE68A` / `#92400E` | **4.8:1** (Light) / **8.1:1** (Dark) |
| **RED** | `Danger, barrier closed` | `#B91C1C` | `#F87171` | `#FEF2F2` | `#450A0A` | `#FECACA` / `#991B1B` | **5.6:1** (Light) / **7.5:1** (Dark) |
| **OFFLINE** | `Offline / Stale` | `#475569` | `#9CA3AF` | `#F1F5F9` | `#1F2937` | `#CBD5E1` / `#374151` | **6.1:1** (Light) / **5.8:1** (Dark) |

---

## 4. Typography Pairing & Justification
- **Source Reasoning:** `ui-ux-pro-max/data/typography.csv` (Category: Dashboards, Analytics, Data Visualization, Admin Panels).
- **Body & Headings:** **Fira Sans** (`300`, `400`, `500`, `600`, `700`)
  - *Justification:* Designed by Erik Spiekermann for screen legibility. As a humanist grotesque, it possesses wide apertures, distinguishable letterforms (e.g. `1`, `l`, `I`), and balanced optical weight. It avoids the generic, overused aesthetic of Inter or Roboto while providing superior scanning speed for gate officers.
- **Numbers, Data & Code:** **Fira Code** (`400`, `500`, `600`)
  - *Justification:* Monospaced font with tabular figures. When telemetry streams at 1 Hz, monospaced digits guarantee that column widths and metric cards never jitter or reflow.

---

## 5. Viewport-Based Fluid Scaling Scale (Frontend-Slides Rule)
Layout and type utilize fluid scaling via CSS `clamp()` and container queries:

```css
/* Fluid Typography Scale */
--text-xs: clamp(0.6875rem, 0.65rem + 0.2vw, 0.75rem);   /* 11px - 12px */
--text-sm: clamp(0.75rem, 0.70rem + 0.25vw, 0.875rem);   /* 12px - 14px */
--text-base: clamp(0.875rem, 0.82rem + 0.3vw, 1.0rem);   /* 14px - 16px */
--text-lg: clamp(1.0rem, 0.92rem + 0.4vw, 1.1875rem);    /* 16px - 19px */
--text-xl: clamp(1.25rem, 1.1rem + 0.6vw, 1.5rem);       /* 20px - 24px */
--text-2xl: clamp(1.5rem, 1.3rem + 1.0vw, 2.0rem);       /* 24px - 32px */

/* Fluid Spacing Scale */
--space-1: clamp(0.25rem, 0.2rem + 0.2vw, 0.375rem);     /* 4px - 6px */
--space-2: clamp(0.5rem, 0.4rem + 0.4vw, 0.75rem);       /* 8px - 12px */
--space-3: clamp(0.75rem, 0.6rem + 0.6vw, 1.0rem);       /* 12px - 16px */
--space-4: clamp(1.0rem, 0.8rem + 0.8vw, 1.5rem);        /* 16px - 24px */
--space-6: clamp(1.5rem, 1.2rem + 1.2vw, 2.25rem);       /* 24px - 36px */
```

---

## 6. Chart Type Selection (ui-ux-pro-max Reasoning)
1. **Overview & Zone Detail Time Series (10-minute risk score):**
   - *Selection:* **Line Chart** with horizontal threshold indicator lines at 50 (Building up) and 75 (Danger).
   - *Reasoning (`charts.csv`):* When user needs to observe trends or rate of change over a continuous time axis, Line Chart offers minimal visual distortion. Area fills and gradients are removed to prevent false volumetric interpretation.
2. **Correlation / Micro-Dynamics (Density vs. Movement Speed):**
   - *Selection:* **Dual-Axis Line Chart** with distinct line patterns and explicit units on left (`p/m²`) and right (`m/s`).
   - *Reasoning (`charts.csv`):* Directly answers the operational question: *"When crowd density rises while movement slows, risk rises"*. Scatter plots require mental coordinate translation; aligned time-series lines immediately expose the inverse crossover where density spikes as speed collapses.
3. **Radar Spatial View:**
   - *Selection:* **Flat Polar Coordinate Grid** with labeled concentric range rings (2m, 4m, 6m), corridor boundary guidelines, and discrete circular target dots.
   - *Reasoning:* Eliminates blurred phosphor sweeps and glow blobs. Provides immediate metric distances and speeds in unboxed, clean SVG text.

---

## 7. Anti-Patterns Explicitly Avoided
1. ❌ **No Blurred Glassmorphism:** Replaced `backdrop-filter: blur(12px)` and semi-transparent alpha overlays with solid, opaque surfaces (`#111827` dark / `#FFFFFF` light).
2. ❌ **No Glow Shadows & Gradient Fills:** Replaced all `box-shadow: 0 0 25px rgba(...)` and `linear-gradient` backgrounds with flat 1px solid borders (`border-subtle`).
3. ❌ **No Decorative Background Patterns:** Removed all SVG background radial dots and simulated grid meshes.
4. ❌ **No Decorative Animations:** Removed spinning radar sweep cones, pulsing buttons, and bouncing icons. Retained only intentional 2Hz blinking on the critical RED state pill.
5. ❌ **No Inter/Roboto Default Typography:** Switched to Fira Sans and Fira Code for technical clarity.
6. ❌ **No Jargon:** Replaced military/academic terminology with plain English labels.

