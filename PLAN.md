# NebulaOS - Project Plan

## Overview
A custom Linux distribution based on Arch Linux with a fully custom Wayland compositor providing a liquid glass, Apple-like desktop experience. Violet/purple color scheme throughout.

---

## Architecture

### Stack (bottom to top)
```
┌─────────────────────────────────────┐
│         NebulaOS Applications       │  ← File Manager, Terminal, Editor, Settings, etc.
├─────────────────────────────────────┤
│       Desktop Components            │  ← Dock, Top Bar, Launcher, Notifications
├─────────────────────────────────────┤
│    nebula-compositor (wlroots)      │  ← Window management, liquid glass rendering
├─────────────────────────────────────┤
│         Wayland Protocol            │  ← Display protocol
├─────────────────────────────────────┤
│          Linux Kernel               │  ← Arch Linux kernel
├─────────────────────────────────────┤
│            Hardware                 │  ← x86_64 PCs
└─────────────────────────────────────┘
```

### Technology Choices
| Component | Technology | Language |
|-----------|-----------|----------|
| Base OS | Arch Linux | - |
| Compositor | wlroots | C |
| Glass Effects | OpenGL ES 3.0 + custom shaders | GLSL |
| UI Toolkit (apps) | GTK4 + custom CSS | C / Python |
| IPC | D-Bus + custom protocol | C |
| Build System | Meson | - |
| ISO Builder | archiso | Shell |
| Settings Backend | dconf + custom | C |
| Animations | Custom easing library | C |

---

## Project Structure

```
nebulaos/
├── compositor/              # nebula-compositor (wlroots-based Wayland compositor)
│   ├── src/
│   │   ├── main.c          # Entry point
│   │   ├── server.c        # Wayland server setup
│   │   ├── output.c        # Output/rendering
│   │   ├── input.c         # Keyboard/mouse/touch handling
│   │   ├── window.c        # Window management (floating)
│   │   ├── glass.c         # Liquid glass shader effects
│   │   ├── blur.c          # Gaussian blur backend
│   │   ├── animation.c     # Animation/easing engine
│   │   ├── decoration.c    # Client-side decorations
│   │   └── config.c        # Configuration parsing
│   ├── include/
│   │   ├── nebula.h        # Main header
│   │   ├── glass.h         # Glass effects API
│   │   └── animation.h     # Animation API
│   └── meson.build
├── shell/                   # Desktop shell (panels, dock, launcher)
│   ├── dock/               # macOS-style dock
│   ├── topbar/             # Top panel bar
│   ├── launcher/           # Spotlight-style launcher
│   ├── notifications/      # Toast notification system
│   └── session/            # Login screen / session manager
├── apps/                    # Custom applications
│   ├── nebula-files/       # File manager
│   ├── nebula-terminal/    # Terminal emulator
│   ├── nebula-editor/      # Text editor
│   ├── nebula-settings/    # System settings
│   ├── nebula-monitor/     # System monitor
│   └── nebula-browser/     # Web browser (or integrate existing)
├── theme/                   # Global theme
│   ├── gtk/                # GTK4 CSS themes
│   ├── icons/              # Icon set
│   ├── fonts/              # Custom fonts
│   ├── cursors/            # Cursor theme
│   ├── wallpapers/         # Default wallpapers
│   └── glass-shaders/      # GLSL shader files
├── iso/                     # ISO build configuration
│   ├── packages/           # Package lists
│   ├── airootfs/           # Root filesystem overlay
│   └── grub/               # Bootloader config
├── installer/               # Installation wizard
├── docs/                    # Documentation
└── scripts/                 # Build/utility scripts
```

---

## Phased Development Plan

### Phase 1: Foundation (Months 1-2)
**Goal**: Bootable Arch-based system with custom compositor showing a glass window

- [ ] Set up Arch Linux build environment
- [ ] Create archiso configuration for NebulaOS live ISO
- [ ] Implement basic wlroots compositor (window display, input)
- [ ] Add floating window manager
- [ ] Implement basic glass shader (transparency + blur)
- [ ] Create basic top bar (clock, window title)
- [ ] Create simple dock (launch terminal)
- [ ] Boot into compositor from ISO

### Phase 2: Core Desktop (Months 3-4)
**Goal**: Functional desktop with dock, topbar, file manager, terminal

- [ ] Complete dock with app icons, animations, indicators
- [ ] Top bar with system tray, clock, menu
- [ ] Window decorations (title bars with glass effect)
- [ ] File manager (nebula-files)
- [ ] Terminal emulator (nebula-terminal)
- [ ] Window snapping/positioning
- [ ] Virtual desktops (workspaces)
- [ ] Animated gradient wallpaper

### Phase 3: Polish & Apps (Months 5-7)
**Goal**: Settings, system monitor, launcher, notifications

- [ ] Spotlight-style app launcher
- [ ] Toast notification system
- [ ] Settings application (display, wallpaper, dock, theme)
- [ ] System monitor
- [ ] Text editor
- [ ] Display manager (login screen)
- [ ] File drag and drop
- [ ] Copy/paste between apps

### Phase 4: Installer & ISO (Months 8-10)
**Goal**: Installable ISO with simple GUI installer

- [ ] Custom GUI installer
- [ ] Disk partitioning
- [ ] User creation
- [ ] Post-install configuration
- [ ] Bootloader setup (GRUB/systemd-boot)
- [ ] Release ISO images

### Phase 5: Refinement (Months 11-12)
**Goal**: Bug fixes, performance, documentation

- [ ] Performance optimization
- [ ] Accessibility features
- [ ] Internationalization
- [ ] Documentation
- [ ] First release (v1.0)

---

## Liquid Glass Design System

### Color Palette
```css
--nebula-primary: #8B5CF6;        /* Violet-500 */
--nebula-primary-light: #A78BFA;  /* Violet-400 */
--nebula-primary-dark: #7C3AED;   /* Violet-600 */
--nebula-accent: #C084FC;         /* Purple-400 */
--nebula-glass-bg: rgba(139, 92, 246, 0.15);  /* Glass background */
--nebula-glass-border: rgba(255, 255, 255, 0.18);
--nebula-glass-highlight: rgba(255, 255, 255, 0.25);
--nebula-surface: rgba(15, 10, 30, 0.75);     /* Dark surface */
--nebula-text: #F5F3FF;           /* Violet-50 */
--nebula-text-secondary: #C4B5FD; /* Violet-300 */
```

### Glass Effect Properties
- **Background**: Semi-transparent with backdrop blur (30-40px)
- **Border**: 1px white border at 15-20% opacity
- **Shadow**: Large soft shadow (0 8px 32px rgba(0,0,0,0.4))
- **Highlight**: Subtle inner glow at top edge
- **Radius**: 16px for windows, 24px for dock items
- **Noise texture**: Subtle grain overlay for realism

### Animation Timing
- **Window open**: 300ms spring (scale 0.95→1, opacity 0→1)
- **Window close**: 200ms ease-out (scale 1→0.95, opacity 1→0)
- **Dock hover**: 200ms spring (scale 1.0→1.15)
- **Dock item launch**: 400ms spring (scale 1.0→0.9→1.1→1.0)
- **Menu open**: 250ms ease-out (opacity, translateY)
- **Workspace switch**: 350ms ease-in-out (slide)

---

## Key Technical Decisions

1. **Why wlroots**: Battle-tested Wayland compositor library, used by Sway, Wayfire, etc. Handles the hard parts (input, outputs, DRM/KMS, DRM buffers) while letting us focus on the glass effects and desktop shell.

2. **Why GTK4 for apps**: Mature, good Wayland support, CSS theming for glass effects, Python bindings for rapid development.

3. **Why Arch Linux**: Rolling release means latest drivers/kernel, archiso makes ISO building trivial, AUR gives access to everything, minimal base to build on.

4. **Why pure floating**: Apple-like experience requires floating windows. Can add optional tiling later.

---

## Build Commands (Once Implemented)

```bash
# Build the compositor
cd compositor && meson setup build && ninja -C build

# Build all shell components
cd shell && make

# Build all apps
cd apps && make all

# Build the ISO
cd iso && sudo mkarchiso -w work -o out .

# Run in QEMU for testing
qemu-system-x86_64 -cdrom out/nebulaos.iso -m 4G -enable-kvm
```
