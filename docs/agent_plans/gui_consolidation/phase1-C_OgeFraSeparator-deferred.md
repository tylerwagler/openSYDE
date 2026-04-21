# Deferred: C_OgeFraSeparator migration

## Why deferred

This class is a stylesheet-only QFrame subclass, but its usage footprint is
**much larger** than any previous migration:

- **168 widget instances across 96 .ui files**
- **96 customwidget blocks** (one per .ui file)
- **4 programmatic .cpp/.hpp references** including a factory function
  `C_SebToolboxUtil::h_AddNewHeading` that returns `C_OgeFraSeparator *`
- **~15 .qss selectors** across 3 apps (Color.qss in each, plus attribute
  variants for HasColor*Background)

Target replacement: `QFrame + styleRole="fra-separator"`.

## Scope estimate

- Reading widget context + editing each: ~168 × 2 tool calls = ~336 for the
  widget insertions alone
- Customwidget renames: ~96 (though replace_all may condense some)
- .cpp changes: the factory function return type change propagates to callers
- Multiple commits likely required

## Recommended approach for the new session

1. Survey once more to confirm counts and check for substring collisions
   (no other class starts with `C_OgeFraSeparator`, so `replace_all` on the
   widget class line is safe)
2. Consider doing one batch of .ui files per commit to keep diffs reviewable
3. Handle the factory function in `C_SebToolboxUtil::h_AddNewHeading` — it
   currently returns `C_OgeFraSeparator *`; replace return type with
   `QFrame *` and set styleRole in the factory body so all callers
   automatically get the new behavior
4. For the 3 .cpp files with direct instantiation (`new C_OgeFraSeparator`),
   refactor to `new QFrame` + `setProperty("styleRole", "fra-separator")`

## Files to touch

See the existing class files plus the grep results captured in the
conversation. The 96 .ui files span both shared (`libraries/opensyde_gui/`)
and app-specific directories (all 3 apps use it).
